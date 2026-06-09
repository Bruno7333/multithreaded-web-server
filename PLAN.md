# C++ Multithreaded Web Server - Architectural & Development Plan

This document outlines the architectural design, class structures, multi-threading model, and implementation phases for the C++ Multithreaded Web Server.

---

## 1. Architectural Overview

The server is built using the **Thread Pool (Producer-Consumer)** pattern. This decouples connection acceptance from request processing, allowing high throughput and efficient CPU utilization.

### Data Flow Diagram
```
                     +----------------------+
                     |     Client/Browser   |
                     +----------+-----------+
                                | (TCP Connection)
                                v
                     +----------+-----------+
                     |      WebServer       | (Main thread / socket listener)
                     +----------+-----------+
                                |
                                | (Accept & wrap socket descriptor)
                                v
                     +----------+-----------+
                     |  SafeQueue<SOCKET>   | (Thread-safe FIFO Task Queue)
                     +----------+-----------+
                                |
          +---------------------+---------------------+
          | (Worker thread pop) | (Worker thread pop) | (Worker thread pop)
          v                     v                     v
   +------+-----+        +------+-----+        +------+-----+
   | Thread #1  |        | Thread #2  |        | Thread #N  | (ThreadPool)
   +------+-----+        +------+-----+        +------+-----+
          |                     |                     |
          v                     v                     v
+---------+---------+ +---------+---------+ +---------+---------+
| RequestHandler    | | RequestHandler    | | RequestHandler    |
| & ConnectionMgr   | | & ConnectionMgr   | | & ConnectionMgr   |
+---------+---------+ +---------+---------+ +---------+---------+
          |                     |                     |
          v                     v                     v
   (HTTP Response)       (HTTP Response)       (HTTP Response)
```

---

## 2. Multi-Platform Socket Layer (Windows Focus)

Because the development environment is **Windows**, socket programming requires **Winsock2** (`winsock2.h`, linking against `ws2_32.lib`). We will write a lightweight wrapper or compatibility layer to ensure socket handling works smoothly.

### Winsock Initialization Checklist:
1. Initialize Winsock DLL: `WSAStartup(...)` at server launch.
2. Check for errors and handle cleanup: `WSACleanup()` on exit.
3. Use `SOCKET` type rather than standard `int` file descriptors (on Linux, sockets are `int`; on Windows, they are `SOCKET`/`UINT_PTR`).
4. Close sockets using `closesocket()` rather than `close()`.

---

## 3. Component Details & Class Designs

### A. `SafeQueue.hpp` (Thread-Safe Task Queue)
A generic queue template that holds pending client sockets.
- **Data Members:**
  - `std::queue<T> m_queue` - The underlying standard queue.
  - `std::mutex m_mutex` - Controls access to the queue.
  - `std::condition_variable m_cv` - Notifies worker threads when a task is available.
- **Key Methods:**
  - `void push(T value)`: Locks, pushes to queue, notifies one waiting thread.
  - `T pop()`: Locks, waits on `m_cv` while queue is empty, pops, returns value.
  - `bool try_pop(T& value)`: Non-blocking pop; returns false immediately if empty.
  - `bool empty()`: Returns whether queue is empty.

### B. `WebServer` (`WebServer.hpp` / `WebServer.cpp`)
Main server controller. Spawns threads, listens for client requests, and inserts connections into the `SafeQueue`.
- **Data Members:**
  - `SOCKET m_listenerSocket` - Listening server socket.
  - `int m_port` - Binding port.
  - `std::vector<std::thread> m_threadPool` - Worker threads pool.
  - `SafeQueue<SOCKET> m_taskQueue` - Active connections queue.
  - `std::atomic<bool> m_running` - Controls server run-state for graceful stop.
- **Key Methods:**
  - `void start()`: Winsock initialization, socket bind, socket listen, spawns threads.
  - `void run()`: Loop that accepts incoming connections and pushes them to `m_taskQueue`.
  - `void stop()`: Triggers shutdown, notifies all threads, joins worker threads, cleans up sockets.

### C. `ConnectionManager` (`ConnectionManager.hpp` / `ConnectionManager.cpp`)
Handles reading from and writing to raw network sockets safely.
- **Key Methods:**
  - `std::string readAll(SOCKET clientSocket)`: Reads incoming data from the socket until the double CRLF (`\r\n\r\n`) representing the end of HTTP headers, handling partial reads.
  - `void sendAll(SOCKET clientSocket, const std::string& response)`: Safely writes bytes back to client, looping until all data is transmitted.

### D. `RequestHandler` (`RequestHandler.hpp` / `RequestHandler.cpp`)
Processes HTTP packets and maps requests to static resources.
- **Key Methods:**
  - `void handleConnection(SOCKET clientSocket)`: Orchestrates the read, parse, and response loop for a given connection.
  - `std::string parseRequest(const std::string& rawRequest)`: Extracts Method, Path, and headers.
  - `std::string buildResponse(const std::string& path)`: Determines MIME-type, loads resource from `public/` directory (or returns 404), and formats the standard HTTP/1.1 response.

### E. `ConsoleThread` (`ConsoleThread.hpp` / `ConsoleThread.cpp`)
Ensures clean termination.
- Runs a dedicated input thread watching standard input (`std::cin`).
- Upon typing `"stop"` or `"exit"`, invokes `WebServer::stop()` to cleanly close open ports and terminate threads without hard-killing the executable.

---

## 4. Resource & Thread Safety Checklist

- [ ] **No Race Conditions on Sockets:** Socket descriptors are popped exclusively by a single thread from the task queue.
- [ ] **Mutex Protection:** Output streams (like logging to `std::cout`) must be guarded by a mutex if multiple threads print simultaneously, to prevent garbled text.
- [ ] **Deadlock Prevention:** Ensure lock order is consistent across all multi-threaded operations.
- [ ] **Thread Termination:** Worker threads must wake up and exit when the server is stopped, even if they are currently blocked waiting on an empty queue.

---

## 5. Development Roadmap (Milestones)

### Milestone 1: Environment Setup & Hello World Socket
- Set up a `CMakeLists.txt` configured for Windows linking `ws2_32`.
- Write `main.cpp` and `WebServer.cpp` with a blocking single-threaded accept-loop that returns `HTTP 200 OK "Hello World"`.

### Milestone 2: Thread Pool & Queue Synchronization
- Implement template-based `SafeQueue.hpp`.
- Initialize `m_threadPool` in `WebServer`.
- Update `WebServer` to push accepted sockets to `SafeQueue`. Worker threads pop sockets and print the request to `std::cout`.

### Milestone 3: HTTP Protocol Parser
- Complete `RequestHandler` to parse verbs (`GET`), target paths (e.g., `/index.html`), and build valid responses.
- Set up a `public/` directory containing an `index.html` file and serve it.

### Milestone 4: Graceful Shutdown & Polish
- Write `ConsoleThread` to join everything on command.
- Ensure all sockets are closed and thread handles joined.
- Handle error conditions (e.g., ports already in use).
