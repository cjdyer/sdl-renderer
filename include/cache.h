#ifndef __CACHE_H__
#define __CACHE_H__

#include <list>
#include <unordered_map>
#include <mutex>
#include <unistd.h>
#include "types.h"

class LRUCache {
public:
    LRUCache(size_t cap);
    ~LRUCache() {};

    float get(const CachedRay& ray);
    void put(const CachedRay& ray, const float& result);

private:
    std::list<CachedRay> m_use_order;
    std::unordered_map<CachedRay, std::pair<float, std::list<CachedRay>::iterator>> m_cache;
    size_t m_capacity;
    std::mutex m_mutex;
};

#endif // __CACHE_H__