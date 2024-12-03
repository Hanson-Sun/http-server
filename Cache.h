#pragma once

#include <unordered_map>
#include <list>
#include <string>
#include <mutex>
#include <variant>

template <typename T, typename U>
class Cache {
public:
    Cache(size_t max_size);
    std::variant<U, std::monostate> get(const T& key);
    void put(const T& key, const U& value);

private:
    size_t max_size;
    std::list<std::pair<T, U>> items; // stores the items
    std::unordered_map<T, typename std::list<std::pair<T, U>>::iterator> cache_map; // map to list iterator
    std::mutex cache_mutex;

    void evict();
};

#include "Cache.tpp"