#pragma once

#include <iostream>
#include <string>
#include <map>
#include <unordered_map>
#include <vector>
#include <functional>
#include <variant>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>

#include "Cache.h"

#ifdef DEBUG
    #define DEBUG_PRINT(fmt, ...) \
        do { \
            fprintf(stderr, fmt, __VA_ARGS__); \
            fprintf(stderr, "\n"); \
        } while (0)
#else
    #define DEBUG_PRINT(fmt, ...) \
        do { } while (0)
#endif

struct Err {
    int code;
    std::string message;

    Err(int c = 0, const std::string& msg = ""): code(c), message(msg){}
    Err(const Err &err) = default;
    Err(Err &&err) = default;
    void print(std::ostream &s = std::cout) {
        s << "[Error " << code << "]: " << message << std::endl;
    }
};

struct HTTPRequest {
    std::string method;
    std::string path;
    std::string version;
    std::map<std::string, std::string> headers;
    std::string body; 
};

struct HTTPResponse {
    std::string version; 
    int status_code;
    std::string status_message;
    std::map<std::string, std::string> headers;
    std::string body;
};

struct Response {
    int status_code;
    std::string body;
};

class HTTPServer {
private:
    struct Route {
        std::string type;
        std::string path;

        bool operator==(const Route& other) const {
            return type == other.type && path == other.path;
        }
    };

    struct RouteHash {
        std::size_t operator()(const Route& route) const {
            auto hash1 = std::hash<std::string>{}(route.type);
            auto hash2 = std::hash<std::string>{}(route.path);
            return hash1 ^ (hash2 << 1);
        }
    };

    std::vector<std::string> routes;
    std::unordered_map<Route, std::function<HTTPResponse(HTTPRequest)>, RouteHash> route_map;
    struct sockaddr_in addr;
    unsigned int port;
    std::string ip;
    std::string static_dir;
    int tcp_fd;
    std::string version = "1.1";

    // todo: refactor this into separate threadpool class
    std::vector<std::thread> threads;
    int num_threads;
    std::queue<int> task_queue;
    std::mutex mutex;
    std::condition_variable task_available;
    bool running = true;

    int num_conn;

    int max_cache_size;
    Cache<std::string, std::string> cache;

    static const std::unordered_map<std::string, std::string> content_type_map;

public:
    HTTPServer(unsigned int port=4321, std::string ip="127.0.0.1", std::string static_dir = ".", int num_threads=16, int num_conn = 1000, int max_cache_size=80000);

    ~HTTPServer();

    int start();

    std::variant<std::monostate, Err> accept_connections();

    std::variant<std::monostate, Err> worker_thread();

    std::variant<std::monostate, Err> handle_request(int connection);

    int stop();

    void send_error(int connection, Err error);

    std::variant<Err, std::string> recv_request(int connection, std::vector<char> &buffer);

    std::variant<std::monostate, Err> send_response(int connection, const std::string &output_str);

    std::variant<HTTPRequest, Err> process_message(const std::vector<char>& buffer);

    std::variant<HTTPResponse, Err> get_response(HTTPRequest request);

    static std::string get_content_type(const std::string &path);

    std::variant<HTTPResponse, Err> get_static_resource(const std::string &path);

    std::string serialize_response(const HTTPResponse &response);

    HTTPServer* add(std::string URI, std::string type, std::function<HTTPResponse(HTTPRequest)> handler);

    HTTPServer* get(std::string URI, std::function<HTTPResponse(HTTPRequest)> handler);

    HTTPServer* post(std::string URI, std::function<HTTPResponse(HTTPRequest)> handler);

    HTTPServer* put(std::string URI, std::function<HTTPResponse(HTTPRequest)> handler);

    HTTPServer* del(std::string URI, std::function<HTTPResponse(HTTPRequest)> handler);

    HTTPServer* page(std::string URI, std::string path);

    void debug_print();
};