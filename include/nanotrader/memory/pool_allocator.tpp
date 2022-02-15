#pragma once

#include "pool_allocator.hpp"

namespace nanotrader {

template<typename T>
void* PoolAllocator<T>::allocate_chunk(size_t size) {
    // Simplified allocation for compatibility
    void* ptr = std::malloc(size);
    return ptr;
}

template<typename T>
void PoolAllocator<T>::setup_free_list(void* chunk, size_t chunk_size, size_t obj_size) {
    char* ptr = static_cast<char*>(chunk);
    char* end = ptr + chunk_size;
    
    FreeNode* current = free_list_.load();
    
    while (ptr + obj_size <= end) {
        FreeNode* node = reinterpret_cast<FreeNode*>(ptr);
        node->next = current;
        current = node;
        ptr += obj_size;
    }
    
    free_list_.store(current);
}

template<typename T>
PoolAllocator<T>::PoolAllocator(size_t pool_size) 
    : free_list_(nullptr)
    , pool_size_(pool_size)
    , chunk_size_(0) {
    
    static_assert(sizeof(T) >= sizeof(FreeNode*), 
                 "Object size must be at least pointer size");
    
    size_t obj_size = std::max(sizeof(T), sizeof(FreeNode));
    obj_size = (obj_size + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
    
    chunk_size_ = pool_size_ * obj_size;
    
    size_t aligned_chunk_size = (chunk_size_ + HUGEPAGE_SIZE - 1) & ~(HUGEPAGE_SIZE - 1);
    
    void* chunk = allocate_chunk(aligned_chunk_size);
    if (!chunk) {
        return; // Failed to allocate, pool will be empty
    }
    allocated_chunks_.push_back(chunk);
    
    setup_free_list(chunk, aligned_chunk_size, obj_size);
}

template<typename T>
PoolAllocator<T>::~PoolAllocator() {
    for (void* chunk : allocated_chunks_) {
        std::free(chunk);
    }
}

template<typename T>
T* PoolAllocator<T>::allocate() {
    FreeNode* node = free_list_.load();
    while (node) {
        if (free_list_.compare_exchange_weak(node, node->next)) {
            return reinterpret_cast<T*>(node);
        }
    }
    
    // Pool exhausted, could expand here or return nullptr
    return nullptr;
}

template<typename T>
void PoolAllocator<T>::deallocate(T* ptr) {
    if (!ptr) return;
    
    FreeNode* node = reinterpret_cast<FreeNode*>(ptr);
    FreeNode* head = free_list_.load();
    
    do {
        node->next = head;
    } while (!free_list_.compare_exchange_weak(head, node));
}

template<typename T>
size_t PoolAllocator<T>::available_count() const {
    size_t count = 0;
    FreeNode* node = free_list_.load();
    while (node) {
        ++count;
        node = node->next;
    }
    return count;
}

template<typename T>
size_t PoolAllocator<T>::capacity() const {
    return pool_size_;
}

} // namespace nanotrader