#pragma once

#include <memory>
#include <atomic>
#include <vector>
#include <cstdlib>

namespace nanotrader {

template<typename T>
class PoolAllocator {
private:
    static constexpr size_t ALIGNMENT = 64;
    static constexpr size_t HUGEPAGE_SIZE = 2 * 1024 * 1024; // 2MB
    
    struct alignas(ALIGNMENT) FreeNode {
        FreeNode* next;
    };
    
    std::atomic<FreeNode*> free_list_;
    std::vector<void*> allocated_chunks_;
    size_t pool_size_;
    size_t chunk_size_;
    
    void* allocate_chunk(size_t size);
    void setup_free_list(void* chunk, size_t chunk_size, size_t obj_size);

public:
    explicit PoolAllocator(size_t pool_size = 1000000);
    ~PoolAllocator();
    
    T* allocate();
    void deallocate(T* ptr);
    
    template<typename... Args>
    T* construct(Args&&... args) {
        T* ptr = allocate();
        if (ptr) {
            new (ptr) T(std::forward<Args>(args)...);
        }
        return ptr;
    }
    
    void destroy(T* ptr) {
        if (ptr) {
            ptr->~T();
            deallocate(ptr);
        }
    }
    
    size_t available_count() const;
    size_t capacity() const;
    
    PoolAllocator(const PoolAllocator&) = delete;
    PoolAllocator& operator=(const PoolAllocator&) = delete;
};

} // namespace nanotrader

#include "pool_allocator.tpp"