//
// Created by jinxin on 2021-06-10.
//

#ifndef WEASEL_SERVER_H
#define WEASEL_SERVER_H


class Server
{
public:
    explicit Server(int port);
    bool run();
private:
    bool epollup();
private:
    int m_port;
    int m_sock;
};


#endif //WEASEL_SERVER_H
