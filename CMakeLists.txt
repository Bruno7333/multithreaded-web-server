cmake_minimum_required(VERSION 3.16)

project(mtWebServer CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(WebServer 
    src/main.cpp
    src/WebServer.cpp
    src/ConnectionManager.cpp
    src/RequestHandler.cpp
    src/ConsoleThread.cpp
)

find_package(Threads REQUIRED)
target_link_libraries(WebServer PRIVATE Threads::Threads)

if(WIN32)
    target_link_libraries(WebServer PRIVATE ws2_32)
endif()