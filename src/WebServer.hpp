#pragma once
#include <winsock2.h>
#include <string>

class WebServer{
    public:
        WebServer(int port);
        void start();
        void run();

    private:
        SOCKET m_listenerSocket;
        int m_port;
    
};