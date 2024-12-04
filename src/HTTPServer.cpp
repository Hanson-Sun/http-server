#include "HTTPServer.h"

const std::unordered_map<std::string, std::string> HTTPServer::content_type_map = {
    {".html", "text/html"},
    {".htm", "text/html"},
    {".css", "text/css"},
    {".js", "application/javascript"},
    {".json", "application/json"},
    {".xml", "application/xml"},
    {".png", "image/png"},
    {".jpg", "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".gif", "image/gif"},
    {".bmp", "image/bmp"},
    {".ico", "image/x-icon"},
    {".svg", "image/svg+xml"},
    {".pdf", "application/pdf"},
    {".zip", "application/zip"},
    {".tar", "application/x-tar"},
    {".rar", "application/x-rar-compressed"},
    {".mp3", "audio/mpeg"},
    {".wav", "audio/wav"},
    {".mp4", "video/mp4"},
    {".avi", "video/x-msvideo"},
    {".mov", "video/quicktime"},
    {".wmv", "video/x-ms-wmv"},
    {".flv", "video/x-flv"},
    {".webm", "video/webm"},
    {".ogg", "application/ogg"},
    {".mkv", "video/x-matroska"},
    {".txt", "text/plain"},
    {".csv", "text/csv"},
    {".md", "text/markdown"},
    {".rtf", "application/rtf"},
    {".ttf", "font/ttf"},
    {".woff", "font/woff"},
    {".woff2", "font/woff2"},
    {".eot", "application/vnd.ms-fontobject"}  
};

HTTPServer::HTTPServer(unsigned int port, std::string ip, std::string static_dir, int num_threads, int num_conn, int max_cache_size)
    : port(port), ip(ip), static_dir(static_dir), num_threads(num_threads),  num_conn(num_conn), max_cache_size(max_cache_size), cache(max_cache_size) {}

HTTPServer::~HTTPServer() {
    stop();
    close(tcp_fd);
}

int HTTPServer::start() {
    std::cout << "Starting server @ http://" << ip << ":" << port << std::endl;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
    addr.sin_port = htons(port);

    if ((tcp_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
        return -1;

    int opt = 1;
    if (setsockopt(tcp_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
        return -1;

    if (bind(tcp_fd, (struct sockaddr*) &addr, sizeof(addr)) < 0)
        return -1;

    if (listen(tcp_fd, num_conn) < 0)
        return -1;

    for (int i = 0; i < num_threads; i++) {
        threads.push_back(std::thread([this] { worker_thread(); }));
    }

    auto result = accept_connections();

    if (std::holds_alternative<Err>(result))
        return std::get<Err>(result).code;

    return 1;
}

std::variant<std::monostate, Err> HTTPServer::accept_connections() {
    while (running) {
        struct sockaddr_in client_addr;
        socklen_t addrlen = sizeof(client_addr);
        int connection = accept(tcp_fd, (struct sockaddr*) &client_addr, &addrlen);
        if (connection < 0) {
            DEBUG_PRINT("Failed to establish connection");
            continue;
        }
        DEBUG_PRINT("Connection established.");
        {
            std::lock_guard<std::mutex> lock(mutex);
            task_queue.push(connection);
            task_available.notify_one();
        }
    }

    return std::monostate();
}

std::variant<std::monostate, Err> HTTPServer::worker_thread() {
    while (running) {
        int connection;
        {
            std::unique_lock<std::mutex> lock(mutex);
            task_available.wait(lock, [this] { return !task_queue.empty() || !running; });
            if (!running && task_queue.empty()) {
                return std::monostate();
            }

            connection = task_queue.front();
            task_queue.pop();
        }

        handle_request(connection);
    }

    return std::monostate();
}

std::variant<std::monostate, Err> HTTPServer::handle_request(int connection) {
    std::vector<char> buffer(2048);
    auto request = recv_request(connection, buffer);

    if (std::holds_alternative<Err>(request)) {
        send_error(connection, std::get<Err>(request));
        return std::get<Err>(request);
    }
    // std::cout << std::string(buffer.data()) << std::endl;

    std::variant<HTTPRequest, Err> result = process_message(buffer);

    if (std::holds_alternative<Err>(result)) {
        send_error(connection, std::get<Err>(result));
        return std::get<Err>(result);
    }

    std::variant<HTTPResponse, Err> response = get_response(std::get<HTTPRequest>(result));
    std::string output_str;

    if (std::holds_alternative<Err>(response)) {
        HTTPResponse res;
        res.status_code = std::get<Err>(response).code;
        res.status_message = std::get<Err>(response).message;
        output_str = serialize_response(res);
    } else {
        output_str = serialize_response(std::get<HTTPResponse>(response));
    }
    // std::cout << output_str << std::endl;

    auto sent = send_response(connection, output_str);
    if (std::holds_alternative<Err>(sent)) {
        return std::get<Err>(sent);
    }

    return std::monostate();
}

int HTTPServer::stop() {
    running = false;
    task_available.notify_all();
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    return 0;
}

void HTTPServer::send_error(int connection, Err error) {
    HTTPResponse res;
    res.status_code = 400;
    res.status_message = "Bad Request";
    res.body = error.message;
    std::string output_str = serialize_response(res);
    send_response(connection, output_str);
}

std::variant<Err, std::string> HTTPServer::recv_request(int connection, std::vector<char> &buffer) {
    size_t total_bytes_read = 0;

    while (true) {
        if (total_bytes_read == buffer.size()) {
            buffer.resize(buffer.size() * 2);
        }

        int bytes_read = recv(connection, buffer.data() + total_bytes_read, buffer.size() - total_bytes_read, 0);
        if (bytes_read < 0) {
            DEBUG_PRINT("Recv failed!!");
            close(connection);
            return Err(errno, "Connection failed");
        }

        if (bytes_read == 0) {
            break;
        }

        total_bytes_read += bytes_read;

        if (total_bytes_read >= 4 && std::string(buffer.data(), total_bytes_read).find("\r\n\r\n") != std::string::npos) {
            break;
        }
    }

    return std::string(buffer.data());
}

std::variant<std::monostate, Err> HTTPServer::send_response(int connection, const std::string &output_str) {
    std::lock_guard<std::mutex> lock(mutex);
    size_t total_sent = 0;
    size_t to_send = output_str.size();

    while (total_sent < to_send) {
        int send_res = send(connection, output_str.c_str() + total_sent, to_send - total_sent, 0);
        if (send_res < 0) {
            DEBUG_PRINT("Failed to send!!\n");
            close(connection);
            return Err(errno, "Response failed to send");
        }
        total_sent += send_res;
    }

    DEBUG_PRINT("Sent response.\n");

    close(connection);
    DEBUG_PRINT("Connection closed.\n");
    return Err(0, ""); // No error
}

std::variant<HTTPRequest, Err> HTTPServer::process_message(const std::vector<char>& buffer) {
    HTTPRequest request;
    std::string message(buffer.begin(), buffer.end());

    size_t header_end_pos = message.find("\r\n\r\n");
    if (header_end_pos == std::string::npos) {
        return Err(-1, "Malformed HTTP request: Missing header-body delimiter");
    }

    std::istringstream header_stream(message.substr(0, header_end_pos));
    std::string line;
    if (std::getline(header_stream, line)) {
        std::istringstream line_stream(line);
        line_stream >> request.method >> request.path >> request.version;
    }

    while (std::getline(header_stream, line) && !line.empty()) {
        size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
            std::string header_name = line.substr(0, colon_pos);
            std::string header_value = line.substr(colon_pos + 1);
            header_name.erase(header_name.find_last_not_of(" \t") + 1);
            header_value.erase(0, header_value.find_first_not_of(" \t"));
            request.headers[header_name] = header_value;
        }
    }

    request.body = message.substr(header_end_pos + 4);

    return request;
}

std::variant<HTTPResponse, Err> HTTPServer::get_response(HTTPRequest request) {
    Route route = {request.method, request.path};

    if (route_map.find(route) == route_map.end()) {
        return get_static_resource(request.path);
    } 

    try {
        return route_map[route](request);
    } catch (const std::exception& e) {
        return Err(-1, std::string(e.what()));
    }
}

std::string HTTPServer::get_content_type(const std::string &path) {
    std::string extension;
    size_t pos = path.find_last_of('.');
    if (pos != std::string::npos) {
        extension = path.substr(pos);
    }

    auto it = content_type_map.find(extension);
    if (it != content_type_map.end()) {
        return it->second;
    }

    return "application/octet-stream";
}

std::variant<HTTPResponse, Err> HTTPServer::get_static_resource(const std::string &path) {
    std::string static_path = static_dir + path;

    auto cached_result = cache.get(static_path);
    HTTPResponse response;
    std::string contents;

    if (std::holds_alternative<std::string>(cached_result)) {
        contents = std::get<std::string>(cached_result);
    } else if (std::filesystem::exists(static_path) && std::filesystem::is_regular_file(static_path)) {
        std::ifstream file(static_path, std::ios::binary);

        if (!file) {
            return Err(500, "Internal Server Error");
        }
        std::ostringstream ss;
        ss << file.rdbuf();
        contents = ss.str();

        cache.put(static_path, contents);
    } else {
        return Err(404, "Not Found"); 
    }

    response.status_code = 200;
    response.status_message = "OK";
    response.headers["Content-Type"] = get_content_type(path);
    response.body = contents;

    return response;
}

std::string HTTPServer::serialize_response(const HTTPResponse &response) {
    std::stringstream ss;
    ss << "HTTP/" << version << " " << response.status_code << " " << response.status_message << "\r\n";
    
    bool has_content_length = false;
    for (const auto &pair : response.headers) {
        ss << pair.first << ": " << pair.second << "\r\n";
        if (pair.first == "Content-Length") {
            has_content_length = true;
        }
    }

    if (!has_content_length && response.body.length() > 0) {
        ss << "Content-Length: " << response.body.size() << "\r\n";
    }

    ss << "\r\n";
    ss << response.body;

    return ss.str();
}

HTTPServer* HTTPServer::add(std::string URI, std::string type, std::function<HTTPResponse(HTTPRequest)> handler) {
    if (type != "GET" && type != "DELETE" && type != "POST" && type != "PUT")
        return nullptr;
    
    route_map[{URI, type}] = handler;
    return this;
}

HTTPServer* HTTPServer::get(std::string URI, std::function<HTTPResponse(HTTPRequest)> handler) {
    return add(URI, "GET", handler);
}

HTTPServer* HTTPServer::post(std::string URI, std::function<HTTPResponse(HTTPRequest)> handler) {
    return add(URI, "POST", handler);
}

HTTPServer* HTTPServer::put(std::string URI, std::function<HTTPResponse(HTTPRequest)> handler) {
    return add(URI, "PUT", handler);
}

HTTPServer* HTTPServer::del(std::string URI, std::function<HTTPResponse(HTTPRequest)> handler) {
    return add(URI, "DELETE", handler);
}

HTTPServer* HTTPServer::page(std::string URI, std::string path) {
    return add(URI, "GET", [this, path](HTTPRequest _) -> HTTPResponse {
        auto result = get_static_resource(path);
        if (std::holds_alternative<Err>(result)) {
            HTTPResponse response;
            Err err = std::get<Err>(result);
            response.status_code = err.code;
            response.status_message = err.message;
            return response;
        }
        return std::get<HTTPResponse>(result);
    });
}

void HTTPServer::debug_print() {
    std::lock_guard<std::mutex> lock(mutex);
    std::cout << "HTTPServer Debug Info:" << std::endl;
    std::cout << "Running: " << (running ? "Yes" : "No") << std::endl;
    std::cout << "Port: " << port << std::endl;
    std::cout << "IP: " << ip << std::endl;
    std::cout << "Static Directory: " << static_dir << std::endl;
    std::cout << "Number of Threads: " << num_threads << std::endl;
    std::cout << "Task Queue Size: " << task_queue.size() << std::endl;
    std::cout << "Max Cache Size: " << max_cache_size << std::endl;
}