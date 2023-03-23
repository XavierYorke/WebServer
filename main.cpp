#include <iostream>
#include <string.h>
#include <cstdio>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <signal.h>
#include <assert.h>
#include "locker.h"
#include "threadpool.h"
#include "http_conn.h"

#define MAX_FD 65535 // 最大文件描述符个数
#define MAX_EVENT_NUMBER 10000 // 监听的最大事件数量

// 添加文件描述符到epoll中
extern void addfd(int epollfd, int fd, bool one_shot);

// 从epoll中删除文件描述符
extern void removefd(int epollfd, int fd);

// 添加信号捕捉
void addsig(int sig, void(handler)(int)) {
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = handler;
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, NULL) != -1);
}

//需要在命令行指定端口号，通过参数传递
int main(int argc, char* argv[]) {
    if (argc <= 1) {
        printf("请按照如下格式运行：./%s port_number\n", basename(argv[0]));
        exit(-1);
    }

    // 获取端口号
    int port = atoi(argv[1]);

    // 对SIGPIPE信号处理
    addsig(SIGPIPE, SIG_IGN);

    // 创建线程池并初始化
    threadpool<http_conn>* pool = NULL;
    try {
        pool = new threadpool<http_conn>;
    } catch(...) {
        exit(-1);
    }

    // 创建数组保存客户端信息
    http_conn* users = new http_conn[MAX_FD];
    
    // 创建监听套接字
    int listenfd = socket(PF_INET, SOCK_STREAM, 0);

    // 设置端口复用，在绑定前
    int reuse = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    // 绑定
    struct sockaddr_in address;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    bind(listenfd, (struct sockaddr*)& address, sizeof(address));

    // 监听
    listen(listenfd, 5);

    // 创建epoll对象，事件数组，添加
    epoll_event events[MAX_EVENT_NUMBER];
    int epollfd = epoll_create(5);

    // 将监听的文件描述符添加到epoll对象中
    addfd(epollfd, listenfd, false);
    http_conn::m_epollfd = epollfd;

    while (true) {
        // 检测到的事件数
        int num = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
        if ((num < 0) && (errno != EINTR)) {
            printf("epoll failure\n");
            break;
        }

        // 循环遍历事件数组
        for (int i = 0; i < num; ++i) {
            
            int sockfd = events[i].data.fd;

            // 有客户端连接进来
            if (sockfd == listenfd) {
                struct sockaddr_in client_address;
                socklen_t client_addrlen = sizeof(client_address);
                int connfd = accept(listenfd, (struct sockaddr*)& client_address, &client_addrlen);
                if (connfd < 0) {
                    printf("errno is: %d\n", errno);
                    continue;
                }
                
                // 目前连接数满了
                if (http_conn::m_user_count >= MAX_FD) {
                    // 给客户端写一个信息：服务器内部正忙
                    close(connfd);
                    continue;
                }

                // 将新的客户的数据初始化，并放入数组中
                users[connfd].init(connfd, client_address);
            }

            // 对方异常断开或错误等事件
            else if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                users[sockfd].close_conn();
            }

            // 读事件
            else if (events[i].events & EPOLLIN) {
                if (users[sockfd].read()) {
                    // 一次把所有数据读完
                    pool->append(users + sockfd);
                }                
                else {
                    users[sockfd].close_conn();
                }
            }

            // 写事件
            else if (events[i].events & EPOLLOUT) {
                if (!users[sockfd].write()) {
                    // 一次写完所有数据
                    users[sockfd].close_conn();
                }
            }
        }
    }

    close(epollfd);
    close(listenfd);
    delete [] users;
    delete pool;

    return 0;
}
