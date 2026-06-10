#include "WebServer.hpp"
#include <iostream>
#include <cstring>

// m_port(port) sets m_port to the value of port, without wasting an extra initialization
WebServer::WebServer(int port) : m_port(port), m_listenerSocket(INVALID_SOCKET) {}

void WebServer::start(){
    // Initialize Winsock
    WSADATA wsaData; //Create a new WSADATA (struct that Winsock fills with metadata about itself)
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0){ 
        //MAKEWORKD(2,2) returns a two byte value winsock reads as version 2.2
                                  //&wsaData is the location where wsaData is stored
        //WSA is Windows Sockets API
        //WSAStartup -> loads networking DLL, makes functions like socket(), bind(), send() callable.
        //           -> gets the right version, sets up the state so we can track sockets
        //           -> returns a code, 0 if success, nonzero if failure
        std::cerr << "WSAStartup failed\n"; //cerr is standard error stream
        return;
    }

    // Create Socket
    m_listenerSocket = socket(AF_INET, SOCK_STREAM, 0); 
    //asks the os to create a new communication endpoint and returns a SOCKET
    //SOCKET is an unsigned integer, that returns the handle for the socket (stored in the OS)
    //      inf af: AF_INET -> tells you what kind of socket is being used | AF_INET -> we are using IPv4
    //      int type: SOCK_STREAM -> Socket Type | SOCK_STREAM -> we are using TCP connection, data arrives in order
    //      int protocol: 0 -> standard TCP protocal as expected
    if (m_listenerSocket == INVALID_SOCKET){
        std::cerr << "socket() failed: " << WSAGetLastError() << "\n";
        return;
    }

    // Bind to port
    sockaddr_in serverAddr{};
    // Predefined struct from winsock2.
    // Declaring socket address struct, (socketaddress_internet). has fields for address family, port, and IP address
    serverAddr.sin_family = AF_INET;
    // socket internet <-> sin
    // address family for the socket, same as as what was used above
    serverAddr.sin_port = htons(m_port);
    // htons <-> host to network short, convert CPU byte order to network byte order
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    // sin_addr.s_addr is the IP address to bind to
    // INADDR_ANY is a special value that means  listen on all network interfaces, server is reachable however a client tries to reach it.
    // to accept only local connections, bind would be 127.0.0.1
    if(bind(m_listenerSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR){
        //m_listenerSocket -> socket we want to bind to
        //(sockaddr*)&serverAddr a pointer to the address struct, expects sockaddr* intead of sockaddr_in* so we had to cast the type
        //sizeof(serverAddr) tells bind how big the struct is, so we know how much to read
        std::cerr << "bind() failed: " << WSAGetLastError() << "\n";
        return;
    }

    // Start listening
    if(listen(m_listenerSocket, SOMAXCONN) == SOCKET_ERROR){
    // listen turns the socket from idle, into one that is waiting for incoming connections.
    // SOMAXCONN is the backlong: maximum number of connections that can sit on the "wait to be accepted" queue at once
    // SOMAXCONN -> SOcket MAXimum CONNections. os picks a good maximum amount of connections
        std::cerr << "listen() failed: " << WSAGetLastError() << "\n";
        return;
    }

    std::cout << "Server listening on port " << m_port << "\n";
}

// accept and respond to requests
void WebServer::run(){
    while (true){
        SOCKET clientSocket = accept(m_listenerSocket, nullptr, nullptr);
        //Continually accept requests from the backlog queue from the listener socket
        // returns a different socket while m_listenerSocket keeps listening for future clients
        // accept freezes the program until someone connects, single thread bottleneck
        // might pass in sockaddr_in later rather than nullptr to log client IPs
        //no exit condition yet, ConsoleThread will implement add later
        if(clientSocket == INVALID_SOCKET) continue;
        // failure guard, wont crash if something goes wrong

        const char* response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 12\r\n"
            "\r\n"
            "Hello World!";
        // hardcoded placeholder response
        
        send(clientSocket, response, (int)strlen(response), 0);
        closesocket(clientSocket);
    }
}