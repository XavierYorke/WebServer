#include "webserver.h"

extern void addfd(int epollfd, int fd, bool one_shot);
extern void removefd(int epollfd, int fd);

WebServer::WebServer(int port) {
    addsig(SIGPIPE, SIG_IGN);
    threadpool_init();
    users = new http_conn[MAX_FD];
    socket_init(port);
    epoll_init();
}

WebServer::~WebServer() {
    close(epollfd);
    close(listenfd);
    delete [] users;
    delete pool;
}

void WebServer::start() {
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
}

void WebServer::addsig(int sig, void(handler)(int)) {
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = handler;
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, NULL) != -1);
}

void WebServer::call_addfd(int epollfd, int fd, bool one_shot) {
    addfd(epollfd, fd, one_shot);
}

void WebServer::call_removefd(int epollfd, int fd) {
    removefd(epollfd, fd);
}

void WebServer::threadpool_init() {
    pool = NULL;
    try {
        pool = new threadpool<http_conn>;
    } catch(...) {
        exit(-1);
    }
}

void WebServer::socket_init(int port) {
    // 创建监听套接字
    listenfd = socket(PF_INET, SOCK_STREAM, 0);

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
}

void WebServer::epoll_init() {
    // 创建epoll对象，事件数组，添加
    events = new epoll_event[MAX_EVENT_NUMBER];
    epollfd = epoll_create(5);

    // 将监听的文件描述符添加到epoll对象中
    addfd(epollfd, listenfd, false);
    http_conn::m_epollfd = epollfd;
}
