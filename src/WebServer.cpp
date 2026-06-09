#include "WebServer.hpp"
#include <iostream>

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
    

    // Start listening
}