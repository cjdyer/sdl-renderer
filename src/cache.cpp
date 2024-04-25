#include "cache.h"
#include <iostream>

LRUCache::LRUCache(size_t cap) : m_capacity(cap) {}

float LRUCache::get(const CachedRay& ray) {
    std::lock_guard<std::mutex> lck(m_mutex);
    auto it = m_cache.find(ray);
    if (it == m_cache.end()) {
        return -1.0;
    }
    // Hit: move to front
    m_use_order.splice(m_use_order.begin(), m_use_order, it->second.second);
    return it->second.first;
}

void LRUCache::put(const CachedRay& ray, const float& result) {
    std::lock_guard<std::mutex> lck(m_mutex);
    auto it = m_cache.find(ray);
    if (it != m_cache.end()) {
        // Update existing entry and move to front
        m_use_order.splice(m_use_order.begin(), m_use_order, it->second.second);
        it->second.first = result;
        return;
    }

    // Insert new entry
    if (m_cache.size() > m_capacity) {
        m_cache.erase(m_use_order.back());
        m_use_order.pop_back();
    }
    m_use_order.push_front(ray);
    m_cache[ray] = {result, m_use_order.begin()};
}