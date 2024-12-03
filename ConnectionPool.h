#pragma once

#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

class ConnectionPool {
public:
    ConnectionPool(std::size_t pool_size, const std::string& ip, unsigned int port);
    int borrow();
    void give_back(int socket);

private:
    std::queue<int> pool;
    std::mutex pool_mutex;
    std::condition_variable condition;
    std::string ip;
    unsigned int port;

    int create_socket();
};