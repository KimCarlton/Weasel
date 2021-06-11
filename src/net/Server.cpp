//
// Created by jinxin on 2021-06-10.
//

#include "Server.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string>
#include <fcntl.h> 
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MAX_EVENTS 10000 
Server::Server(int port)
{
    m_port = port;
}

bool Server::run()
{
    m_sock = socket(AF_INET, SOCK_STREAM, 0);
    //设置非阻塞模式
    fcntl(m_sock, F_SETFL, O_NONBLOCK);
    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("0.0.0.0");
    server.sin_port = htons(m_port);

    int nRet = bind(m_sock, (struct sockaddr*)&server, sizeof(server));
    if (nRet < 0)
    {
        perror("bind\n");
        return false;
    }

    nRet = listen(m_sock, 4);
    if (nRet < 0)
    {
        perror("listen\n");
        return false;
    }


    for (int i = 0;i<5;i++)
    {
        pid_t pid = fork();
        if (pid == 0)
        {
            epollup();
            return 0;
        }

    }

    while (true)
    {
        sleep(10);
    }


    close(m_sock);

    return true;
}

bool Server::epollup()
{
    int epoll_fd = epoll_create(MAX_EVENTS);
    if (epoll_fd < 0)
    {
        perror("epoll_create\n");
        return false;
    }

    int timeout_num = 0;
    int done = 0;
    int timeout = 5000;
    int i = 0;
    int ret_num = -1;

    struct epoll_event ev;
    struct epoll_event event[MAX_EVENTS];
    ev.data.fd = m_sock;
    ev.events = EPOLLIN | EPOLLET;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, m_sock, &ev) < 0)
    {
        perror("epoll_ctl");
        return false;
    }


    while (!done)
    {
        ret_num = epoll_wait(epoll_fd, event, MAX_EVENTS, timeout);
        switch (ret_num)
        {
        case -1:
            if (errno == EINTR)
            {
                continue;
            }
            perror("epoll_wait");
            break;
        case  0:
            //if (timeout_num++ > 5)
            //    done = 1;
            //printf("time out...\n");
            break;
        default:
        {
            //事件处理
            for (i = 0; i < ret_num; ++i)
            {
                if (event[i].data.fd == m_sock && event[i].events & EPOLLIN)
                {
                    int new_sock = -1;
                    struct sockaddr_in client;
                    socklen_t len = sizeof(client);
                    new_sock = accept(m_sock, (struct sockaddr*)&client, &len);
                    if (new_sock < 0)
                    {
                        perror("accept");
                        printf("%s : %d \n", strerror(errno), new_sock);
                        continue;
                    }


                    // 设置非阻塞
                    if (fcntl(new_sock, F_SETFL, fcntl(new_sock, F_GETFD, 0) | O_NONBLOCK) == -1)
                    {
                        perror("fcntl");
                        continue;
                    }


                    ev.data.fd = new_sock;
                    ev.events = EPOLLIN | EPOLLET;
                    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_sock, &ev) < 0) 
                    {
                        perror("epoll_ctl");
                        return false;
                    }
                    //printf("get a connect [%d]...\n", new_sock);
                }
                else
                {
                    if (event[i].events & EPOLLIN) 
                    {
                        int fd = event[i].data.fd;
                        char sz[1024] = {0x0};
                        read(fd, sz, 1024);

                        //printf("%s\n", sz);

                        event[i].events = EPOLLOUT;
                    }

                    if (event[i].events & EPOLLOUT)
                    {
                        int fd = event[i].data.fd;


                        std::string strResp;
                        strResp.append("HTTP/1.1 200 OK\n");
                        strResp.append("Content-Length: 9\n");
                        strResp.append("Date: Fri, 11 Jun 2021 02:07:49 GMT\n");
                        strResp.append("Keep-Alive: timeout=58\n");
                        strResp.append("Server: Weasel v1.0\n");
                        strResp.append("\n");
                        strResp.append("12121231\n");

                        int nLen = write(fd, strResp.data(), strResp.length());

                        //移除fd
                        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
                        //断开连接
                        close(fd);
                    }

                }
            }
        }
        break;
        }

    }


    return true;
}
