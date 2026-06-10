#include "WebServer.hpp"

// main, launch the web server

int main() {
    WebServer server(8080);     //create server on port 8080
    server.start();             // initialize server
    server.run();               // accept responses loop
    return 0;
}