#include "../HTTPServer.h"
#include <iostream>
/**
 * wrk -t16 -c400 -d10s http://127.0.0.1:4321/index.html
 */

int main() {
    HTTPServer* server = new HTTPServer(4321, "127.0.0.1", "../test_resources");

    server->start();

    delete server;
    std::cout << "done!" << std::endl;
    return 0;
}