#include "ConnectionPool.h"

ConnectionPool::ConnectionPool(std::size_t pool_size, const std::string& ip, unsigned int port)
    : ip(ip), port(port) {
    for (std::size_t i = 0; i < pool_size; ++i) {
        int socket = create_socket();
        if (socket != -1) {
            pool.push(socket);
        }
    }
}

int ConnectionPool::create_socket() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket failed");
        return -1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
    addr.sin_port = htons(port);

    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect failed");
        close(sock);
        return -1;
    }

    return sock;
}

int ConnectionPool::borrow() {
    std::unique_lock<std::mutex> lock(pool_mutex);
    condition.wait(lock, [this] { return !pool.empty(); });
    int socket = pool.front();
    pool.pop();
    return socket;
}

void ConnectionPool::give_back(int socket) {
    std::lock_guard<std::mutex> lock(pool_mutex);
    pool.push(socket);
    condition.notify_one();
}