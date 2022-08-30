#pragma once

#include <atomic>
#include <cstddef>
#include <memory>
#include <type_traits>

namespace nanotrader {

template<typename T, size_t Size>
class SPSCRingBuffer {
private:
    static_assert((Size & (Size - 1)) == 0, "Size must be power of 2");
    static constexpr size_t MASK = Size - 1;
    static constexpr size_t CACHE_LINE_SIZE = 64;
    
    alignas(CACHE_LINE_SIZE) std::atomic<size_t> head_{0};
    alignas(CACHE_LINE_SIZE) std::atomic<size_t> tail_{0};
    alignas(CACHE_LINE_SIZE) T buffer_[Size];
    
    size_t cached_head_{0};
    size_t cached_tail_{0};

public:
    SPSCRingBuffer() = default;
    
    template<typename... Args>
    bool try_push(Args&&... args) {
        const size_t current_tail = tail_.load(std::memory_order_relaxed);
        const size_t next_tail = (current_tail + 1) & MASK;
        
        if (next_tail == cached_head_) {
            cached_head_ = head_.load(std::memory_order_acquire);
            if (next_tail == cached_head_) {
                return false;
            }
        }
        
        new (&buffer_[current_tail]) T(std::forward<Args>(args)...);
        tail_.store(next_tail, std::memory_order_release);
        
        return true;
    }
    
    bool try_pop(T& item) {
        const size_t current_head = head_.load(std::memory_order_relaxed);
        
        if (current_head == cached_tail_) {
            cached_tail_ = tail_.load(std::memory_order_acquire);
            if (current_head == cached_tail_) {
                return false;
            }
        }
        
        item = std::move(buffer_[current_head]);
        
        if constexpr (!std::is_trivially_destructible_v<T>) {
            buffer_[current_head].~T();
        }
        
        head_.store((current_head + 1) & MASK, std::memory_order_release);
        
        return true;
    }
    
    template<typename Func>
    size_t try_pop_batch(Func&& func, size_t max_items = Size) {
        const size_t current_head = head_.load(std::memory_order_relaxed);
        
        if (current_head == cached_tail_) {
            cached_tail_ = tail_.load(std::memory_order_acquire);
            if (current_head == cached_tail_) {
                return 0;
            }
        }
        
        size_t available = (cached_tail_ - current_head) & MASK;
        size_t to_pop = std::min(available, max_items);
        
        for (size_t i = 0; i < to_pop; ++i) {
            size_t idx = (current_head + i) & MASK;
            func(std::move(buffer_[idx]));
            
            if constexpr (!std::is_trivially_destructible_v<T>) {
                buffer_[idx].~T();
            }
        }
        
        head_.store((current_head + to_pop) & MASK, std::memory_order_release);
        
        return to_pop;
    }
    
    bool empty() const {
        return head_.load(std::memory_order_acquire) == 
               tail_.load(std::memory_order_acquire);
    }
    
    bool full() const {
        const size_t current_tail = tail_.load(std::memory_order_acquire);
        const size_t current_head = head_.load(std::memory_order_acquire);
        return ((current_tail + 1) & MASK) == current_head;
    }
    
    size_t size() const {
        const size_t current_tail = tail_.load(std::memory_order_acquire);
        const size_t current_head = head_.load(std::memory_order_acquire);
        return (current_tail - current_head) & MASK;
    }
    
    size_t capacity() const {
        return Size - 1;
    }
    
    SPSCRingBuffer(const SPSCRingBuffer&) = delete;
    SPSCRingBuffer& operator=(const SPSCRingBuffer&) = delete;
};

template<typename T>
class MPSCRingBuffer {
private:
    static constexpr size_t CACHE_LINE_SIZE = 64;
    
    struct alignas(CACHE_LINE_SIZE) Node {
        std::atomic<Node*> next{nullptr};
        T data;
        
        Node() = default;
        
        template<typename... Args>
        Node(Args&&... args) : data(std::forward<Args>(args)...) {}
    };
    
    alignas(CACHE_LINE_SIZE) std::atomic<Node*> head_{nullptr};
    alignas(CACHE_LINE_SIZE) std::atomic<Node*> tail_{nullptr};
    
    Node* stub_;

public:
    MPSCRingBuffer() {
        stub_ = new Node();
        head_.store(stub_);
        tail_.store(stub_);
    }
    
    ~MPSCRingBuffer() {
        while (Node* node = head_.load()) {
            head_.store(node->next);
            delete node;
        }
    }
    
    template<typename... Args>
    void push(Args&&... args) {
        Node* node = new Node(std::forward<Args>(args)...);
        Node* prev_tail = tail_.exchange(node);
        prev_tail->next.store(node);
    }
    
    bool try_pop(T& item) {
        Node* head = head_.load();
        Node* next = head->next.load();
        
        if (next == nullptr) {
            return false;
        }
        
        item = std::move(next->data);
        head_.store(next);
        delete head;
        
        return true;
    }
    
    bool empty() const {
        Node* head = head_.load();
        return head->next.load() == nullptr;
    }
    
    MPSCRingBuffer(const MPSCRingBuffer&) = delete;
    MPSCRingBuffer& operator=(const MPSCRingBuffer&) = delete;
};

} // namespace nanotraderstatic constexpr bool ENABLE_PREFETCH = true;
bool try_pop(T& item) noexcept;
void debug_print() const { printf("Head=%zu Tail=%zu\n", head_.load(), tail_.load()); }
// Refactor tail advancement logic with comment
void reset() { head_.store(0); tail_.store(0); }
bool nearly_full() const { return size() >= capacity() - 1; }
// Group atomic loads into local vars for clarity
// Replace redundant std::move with move_if_noexcept
