#include "nanotrader/core/matching_engine.hpp"
#include <algorithm>

namespace nanotrader {

// MatchingEngine constructor
MatchingEngine::MatchingEngine() : order_allocator_(1000000) {}

// Private methods
OrderBook* MatchingEngine::get_or_create_book(Symbol symbol) {
    auto it = order_books_.find(symbol);
    if (it != order_books_.end()) {
        return it->second.get();
    }
    
    auto book = std::make_unique<OrderBook>(symbol);
    OrderBook* book_ptr = book.get();
    order_books_[symbol] = std::move(book);
    return book_ptr;
}

}