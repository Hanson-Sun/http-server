#include "HTTPServer.h"

int main() {
    HTTPServer* server = new HTTPServer(4321, "127.0.0.1", "./test_resources");

    server->start();

    delete server;
    std::cout << "done!" << std::endl;
    return 0;
}