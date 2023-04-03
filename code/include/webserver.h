#ifndef WEBSERVER_H
#define WEBSERVER_H

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

class WebServer {

public:
    WebServer(int port);
    ~WebServer();
    void start();

private:
    void addsig(int sig, void(handler)(int));
    void call_addfd(int epollfd, int fd, bool one_shot);
    void call_removefd(int epollfd, int fd);
    void threadpool_init();
    void socket_init(int port);
    void epoll_init();

    http_conn* users;
    int max_fd = 65535;                 // 最大文件描述符个数
    int max_event_number = 10000;       // 监听的最大事件数量
    epoll_event* events;
    int epollfd;
    int listenfd;
    threadpool<http_conn>* pool;

};

#endif