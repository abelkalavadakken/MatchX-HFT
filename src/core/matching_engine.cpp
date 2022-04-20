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

void MatchingEngine::match_order(OrderBook* book, Order* incoming_order, std::vector<Trade>& trades) {
    if (incoming_order->is_buy()) {
        match_buy_order(book, incoming_order, trades);
    } else {
        match_sell_order(book, incoming_order, trades);
    }
}

void MatchingEngine::match_buy_order(OrderBook* book, Order* buy_order, std::vector<Trade>& trades) {
    while (buy_order->remaining_quantity > 0 && book->has_best_ask()) {
        Price best_ask = book->get_best_ask();
        
        if (buy_order->is_market() || buy_order->price >= best_ask) {
            const PriceLevel* level = book->get_sell_level(best_ask);
            if (!level || level->is_empty()) {
                break;
            }
            
            Order* sell_order = level->head;
            if (!sell_order) {
                break;
            }
            
            Quantity fill_quantity = std::min(buy_order->remaining_quantity, 
                                             sell_order->remaining_quantity);
            
            trades.emplace_back(sell_order->id, buy_order->id, 
                              buy_order->symbol, best_ask, fill_quantity, now());
            
            Quantity old_sell_qty = sell_order->remaining_quantity;
            sell_order->fill(fill_quantity);
            buy_order->fill(fill_quantity);
            
            if (sell_order->is_filled()) {
                book->remove_order(sell_order->id);
                order_allocator_.destroy(sell_order);
            } else {
                book->update_order_quantity(sell_order->id, old_sell_qty);
            }
        } else {
            break;
        }
    }
}

void MatchingEngine::match_sell_order(OrderBook* book, Order* sell_order, std::vector<Trade>& trades) {
    while (sell_order->remaining_quantity > 0 && book->has_best_bid()) {
        Price best_bid = book->get_best_bid();
        
        if (sell_order->is_market() || sell_order->price <= best_bid) {
            const PriceLevel* level = book->get_buy_level(best_bid);
            if (!level || level->is_empty()) {
                break;
            }
            
            Order* buy_order = level->head;
            if (!buy_order) {
                break;
            }
            
            Quantity fill_quantity = std::min(sell_order->remaining_quantity, 
                                             buy_order->remaining_quantity);
            
            trades.emplace_back(buy_order->id, sell_order->id, 
                              sell_order->symbol, best_bid, fill_quantity, now());
            
            Quantity old_buy_qty = buy_order->remaining_quantity;
            buy_order->fill(fill_quantity);
            sell_order->fill(fill_quantity);
            
            if (buy_order->is_filled()) {
                book->remove_order(buy_order->id);
                order_allocator_.destroy(buy_order);
            } else {
                book->update_order_quantity(buy_order->id, old_buy_qty);
            }
        } else {
            break;
        }
    }
}

MatchResult MatchingEngine::process_add_order(const OrderRequest& request) {
    OrderBook* book = get_or_create_book(request.order.symbol);
    Order* order = order_allocator_.construct(request.order);
    
    if (!order) {
        return MatchResult(MatchResult::Status::Rejected, request.order.id);
    }
    
    MatchResult result(MatchResult::Status::Added, request.order.id);
    
    if (request.order.is_market() || 
        (request.order.is_buy() && book->has_best_ask() && 
         request.order.price >= book->get_best_ask()) ||
        (request.order.is_sell() && book->has_best_bid() && 
         request.order.price <= book->get_best_bid())) {
        
        match_order(book, order, result.trades);
        
        if (!result.trades.empty()) {
            result.status = MatchResult::Status::Matched;
        }
    }
    
    if (order->remaining_quantity > 0 && !order->is_ioc() && !order->is_fok()) {
        book->add_order(order);
    } else if (order->is_fok() && order->remaining_quantity > 0) {
        result.status = MatchResult::Status::Rejected;
        result.trades.clear();
        order_allocator_.destroy(order);
    } else if (order->remaining_quantity == 0 || order->is_ioc()) {
        order_allocator_.destroy(order);
    }
    
    return result;
}

MatchResult MatchingEngine::process_cancel_order(const OrderRequest& request) {
    OrderBook* book = get_or_create_book(request.order.symbol);
    Order* order = book->get_order(request.order.id);
    
    if (!order) {
        return MatchResult(MatchResult::Status::Rejected, request.order.id);
    }
    
    book->remove_order(request.order.id);
    order_allocator_.destroy(order);
    
    return MatchResult(MatchResult::Status::Cancelled, request.order.id);
}

MatchResult MatchingEngine::process_modify_order(const OrderRequest& request) {
    OrderBook* book = get_or_create_book(request.order.symbol);
    Order* order = book->get_order(request.order.id);
    
    if (!order) {
        return MatchResult(MatchResult::Status::Rejected, request.order.id);
    }
    
    if (request.new_quantity == 0) {
        book->remove_order(request.order.id);
        order_allocator_.destroy(order);
        return MatchResult(MatchResult::Status::Cancelled, request.order.id);
    }
    
    Quantity old_quantity = order->remaining_quantity;
    order->remaining_quantity = request.new_quantity;
    order->quantity = request.new_quantity;
    
    book->update_order_quantity(order->id, old_quantity);
    
    return MatchResult(MatchResult::Status::Modified, request.order.id);
}

// Public methods
bool MatchingEngine::submit_order(const OrderRequest& request) {
    return input_buffer_.try_push(request);
}

bool MatchingEngine::get_result(MatchResult& result) {
    return output_buffer_.try_pop(result);
}

void MatchingEngine::process_orders() {
    OrderRequest request;
    while (input_buffer_.try_pop(request)) {
        MatchResult result;
        
        switch (request.type) {
            case OrderRequest::Type::Add:
                result = process_add_order(request);
                break;
            case OrderRequest::Type::Cancel:
                result = process_cancel_order(request);
                break;
            case OrderRequest::Type::Modify:
                result = process_modify_order(request);
                break;
        }
        
        if (!output_buffer_.try_push(std::move(result))) {
            // Output buffer full, could implement backpressure
            break;
        }
        
        processed_orders_.fetch_add(1, std::memory_order_relaxed);
    }
}

void MatchingEngine::start() {
    running_.store(true);
}

void MatchingEngine::stop() {
    running_.store(false);
}

bool MatchingEngine::is_running() const {
    return running_.load();
}

uint64_t MatchingEngine::get_processed_orders() const {
    return processed_orders_.load();
}

OrderBook* MatchingEngine::get_order_book(Symbol symbol) {
    auto it = order_books_.find(symbol);
    return (it != order_books_.end()) ? it->second.get() : nullptr;
}

const OrderBook* MatchingEngine::get_order_book(Symbol symbol) const {
    auto it = order_books_.find(symbol);
    return (it != order_books_.end()) ? it->second.get() : nullptr;
}

size_t MatchingEngine::get_order_book_count() const {
    return order_books_.size();
}

size_t MatchingEngine::get_total_orders() const {
    size_t total = 0;
    for (const auto& [symbol, book] : order_books_) {
        total += book->get_order_count();
    }
    return total;
}

size_t MatchingEngine::get_available_order_capacity() const {
    return order_allocator_.available_count();
}

void MatchingEngine::clear_all_books() {
    order_books_.clear();
    processed_orders_.store(0);
}

} // namespace nanotrader