#include "Cache.h"

template <typename T, typename U>
Cache<T, U>::Cache(size_t max_size) : max_size(max_size) {}

template <typename T, typename U>
std::variant<U, std::monostate> Cache<T, U>::get(const T& key) {
    std::lock_guard<std::mutex> lock(cache_mutex);
    auto it = cache_map.find(key);
    if (it == cache_map.end()) {
        return std::monostate();
    }

    items.splice(items.begin(), items, it->second);
    return it->second->second;
}

template <typename T, typename U>
void Cache<T, U>::put(const T& key, const U& value) {
    std::lock_guard<std::mutex> lock(cache_mutex);
    auto it = cache_map.find(key);
    if (it != cache_map.end()) {
        it->second->second = value;
        items.splice(items.begin(), items, it->second);
    } else {
        items.emplace_front(key, value);
        cache_map[key] = items.begin();
        if (cache_map.size() > max_size) {
            evict();
        }
    }
}

template <typename T, typename U>
void Cache<T, U>::evict() {
    auto last = items.end();
    --last;
    cache_map.erase(last->first);
    items.pop_back();
}