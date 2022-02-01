#include "nanotrader/core/order_book.hpp"
#include <algorithm>

namespace nanotrader {

// OrderBook implementations
OrderBook::OrderBook(Symbol symbol) noexcept 
    : symbol_(symbol)
    , best_bid_(Price{})
    , best_ask_(Price{})
    , has_best_bid_(false)
    , has_best_ask_(false) {
    buy_levels_.reserve(10000);
    sell_levels_.reserve(10000);
    orders_.reserve(100000);
}

void OrderBook::update_best_bid() noexcept {
    Price new_best_bid{};
    bool found = false;
    
    for (const auto& [price_raw, level] : buy_levels_) {
        if (!level.is_empty()) {
            Price price{price_raw};
            if (!found || price > new_best_bid) {
                new_best_bid = price;
                found = true;
            }
        }
    }
    
    best_bid_ = new_best_bid;
    has_best_bid_ = found;
}

void OrderBook::update_best_ask() noexcept {
    Price new_best_ask{};
    bool found = false;
    
    for (const auto& [price_raw, level] : sell_levels_) {
        if (!level.is_empty()) {
            Price price{price_raw};
            if (!found || price < new_best_ask) {
                new_best_ask = price;
                found = true;
            }
        }
    }
    
    best_ask_ = new_best_ask;
    has_best_ask_ = found;
}

void OrderBook::cleanup_empty_level(PriceLevelMap& levels, int64_t price_raw) noexcept {
    auto it = levels.find(price_raw);
    if (it != levels.end() && it->second.is_empty()) {
        levels.erase(it);
    }
}

bool OrderBook::add_order(Order* order) noexcept {
    if (orders_.find(order->id) != orders_.end()) {
        return false;
    }
    
    orders_[order->id] = order;
    
    if (order->is_buy()) {
        auto& level = buy_levels_[order->price.raw_value()];
        level.price = order->price;
        level.add_order(order);
        
        if (!has_best_bid_ || order->price > best_bid_) {
            best_bid_ = order->price;
            has_best_bid_ = true;
        }
    } else {
        auto& level = sell_levels_[order->price.raw_value()];
        level.price = order->price;
        level.add_order(order);
        
        if (!has_best_ask_ || order->price < best_ask_) {
            best_ask_ = order->price;
            has_best_ask_ = true;
        }
    }
    
    return true;
}

bool OrderBook::remove_order(OrderId order_id) noexcept {
    auto it = orders_.find(order_id);
    if (it == orders_.end()) {
        return false;
    }
    
    Order* order = it->second;
    orders_.erase(it);
    
    if (order->is_buy()) {
        auto& level = buy_levels_[order->price.raw_value()];
        level.remove_order(order);
        
        if (level.is_empty()) {
            cleanup_empty_level(buy_levels_, order->price.raw_value());
            if (has_best_bid_ && order->price == best_bid_) {
                update_best_bid();
            }
        }
    } else {
        auto& level = sell_levels_[order->price.raw_value()];
        level.remove_order(order);
        
        if (level.is_empty()) {
            cleanup_empty_level(sell_levels_, order->price.raw_value());
            if (has_best_ask_ && order->price == best_ask_) {
                update_best_ask();
            }
        }
    }
    
    return true;
}

void OrderBook::update_order_quantity(OrderId order_id, Quantity old_quantity) noexcept {
    auto it = orders_.find(order_id);
    if (it == orders_.end()) {
        return;
    }
    
    Order* order = it->second;
    
    if (order->is_buy()) {
        auto& level = buy_levels_[order->price.raw_value()];
        level.update_quantity(order, old_quantity);
    } else {
        auto& level = sell_levels_[order->price.raw_value()];
        level.update_quantity(order, old_quantity);
    }
}

Order* OrderBook::get_order(OrderId order_id) const noexcept {
    auto it = orders_.find(order_id);
    return (it != orders_.end()) ? it->second : nullptr;
}

Price OrderBook::get_best_bid() const noexcept {
    return has_best_bid_ ? best_bid_ : Price{};
}

Price OrderBook::get_best_ask() const noexcept {
    return has_best_ask_ ? best_ask_ : Price{};
}

bool OrderBook::has_best_bid() const noexcept {
    return has_best_bid_;
}

bool OrderBook::has_best_ask() const noexcept {
    return has_best_ask_;
}

const PriceLevel* OrderBook::get_buy_level(Price price) const noexcept {
    auto it = buy_levels_.find(price.raw_value());
    return (it != buy_levels_.end()) ? &it->second : nullptr;
}

const PriceLevel* OrderBook::get_sell_level(Price price) const noexcept {
    auto it = sell_levels_.find(price.raw_value());
    return (it != sell_levels_.end()) ? &it->second : nullptr;
}

Symbol OrderBook::get_symbol() const noexcept {
    return symbol_;
}

size_t OrderBook::get_order_count() const noexcept {
    return orders_.size();
}

std::vector<std::pair<Price, Quantity>> OrderBook::get_bid_levels(size_t depth) const {
    std::vector<std::pair<Price, Quantity>> levels;
    levels.reserve(depth);
    
    std::vector<std::pair<Price, Quantity>> all_levels;
    for (const auto& [price_raw, level] : buy_levels_) {
        if (!level.is_empty()) {
            all_levels.emplace_back(Price{price_raw}, level.total_quantity);
        }
    }
    
    std::sort(all_levels.begin(), all_levels.end(), 
             [](const auto& a, const auto& b) { return a.first > b.first; });
    
    for (size_t i = 0; i < std::min(depth, all_levels.size()); ++i) {
        levels.push_back(all_levels[i]);
    }
    
    return levels;
}

std::vector<std::pair<Price, Quantity>> OrderBook::get_ask_levels(size_t depth) const {
    std::vector<std::pair<Price, Quantity>> levels;
    levels.reserve(depth);
    
    std::vector<std::pair<Price, Quantity>> all_levels;
    for (const auto& [price_raw, level] : sell_levels_) {
        if (!level.is_empty()) {
            all_levels.emplace_back(Price{price_raw}, level.total_quantity);
        }
    }
    
    std::sort(all_levels.begin(), all_levels.end(), 
             [](const auto& a, const auto& b) { return a.first < b.first; });
    
    for (size_t i = 0; i < std::min(depth, all_levels.size()); ++i) {
        levels.push_back(all_levels[i]);
    }
    
    return levels;
}

void OrderBook::clear() noexcept {
    buy_levels_.clear();
    sell_levels_.clear();
    orders_.clear();
    has_best_bid_ = false;
    has_best_ask_ = false;
}

} // namespace nanotrader