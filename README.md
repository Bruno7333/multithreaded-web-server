# multithreaded-web-server

Currently there is a working web server that displays "Hello World".

## Starting the server

```powershell
cmake -S . -B build
cmake --build build
.\build\Debug\WebServer.exe
```

Then open http://localhost:8080 in your browser.