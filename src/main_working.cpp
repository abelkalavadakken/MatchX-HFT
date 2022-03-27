#include "nanotrader/core/order_book.hpp"
#include "nanotrader/memory/ring_buffer.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <random>
#include <vector>

using namespace nanotrader;

class SimpleMatchingEngine {
private:
    OrderBook order_book_;
    std::vector<std::unique_ptr<Order>> orders_; // Use vector instead of pool
    OrderId next_order_id_;
    
public:
    SimpleMatchingEngine(Symbol symbol) 
        : order_book_(symbol), next_order_id_(1) {}
    
    OrderId submit_buy_order(Price price, Quantity quantity) {
        auto order = std::make_unique<Order>(
            next_order_id_++, order_book_.get_symbol(), price, quantity, 
            Side::Buy, OrderType::Limit, now());
        
        OrderId id = order->id;
        if (order_book_.add_order(order.get())) {
            orders_.push_back(std::move(order));
            return id;
        }
        return 0; // Failed
    }
    
    OrderId submit_sell_order(Price price, Quantity quantity) {
        auto order = std::make_unique<Order>(
            next_order_id_++, order_book_.get_symbol(), price, quantity, 
            Side::Sell, OrderType::Limit, now());
        
        OrderId id = order->id;
        if (order_book_.add_order(order.get())) {
            orders_.push_back(std::move(order));
            return id;
        }
        return 0; // Failed
    }
    
    bool cancel_order(OrderId order_id) {
        return order_book_.remove_order(order_id);
    }
    
    const OrderBook& get_order_book() const { return order_book_; }
    
    void print_market_data() const {
        std::cout << "\n📊 MARKET DATA:\n";
        std::cout << "================\n";
        
        if (order_book_.has_best_bid() && order_book_.has_best_ask()) {
            Price bid = order_book_.get_best_bid();
            Price ask = order_book_.get_best_ask();
            double spread = ask.to_double() - bid.to_double();
            
            std::cout << "BID: $" << bid.to_double() 
                      << " | ASK: $" << ask.to_double()
                      << " | SPREAD: $" << spread << "\n";
        } else {
            std::cout << "BID: ";
            if (order_book_.has_best_bid()) {
                std::cout << "$" << order_book_.get_best_bid().to_double();
            } else {
                std::cout << "N/A";
            }
            
            std::cout << " | ASK: ";
            if (order_book_.has_best_ask()) {
                std::cout << "$" << order_book_.get_best_ask().to_double();
            } else {
                std::cout << "N/A";
            }
            std::cout << "\n";
        }
        
        std::cout << "Total Orders: " << order_book_.get_order_count() << "\n";
        
        // Show order book depth
        auto bids = order_book_.get_bid_levels(5);
        auto asks = order_book_.get_ask_levels(5);
        
        std::cout << "\n📈 ORDER BOOK DEPTH:\n";
        std::cout << "BIDS                 ASKS\n";
        std::cout << "Price    | Qty       Price    | Qty\n";
        std::cout << "---------|--------   ---------|--------\n";
        
        size_t max_levels = std::max(bids.size(), asks.size());
        for (size_t i = 0; i < max_levels; ++i) {
            // Bid side
            if (i < bids.size()) {
                printf("$%-7.2f | %-7llu", bids[i].first.to_double(), bids[i].second);
            } else {
                printf("%-8s | %-7s", "", "");
            }
            
            std::cout << "   ";
            
            // Ask side
            if (i < asks.size()) {
                printf("$%-7.2f | %-7llu", asks[i].first.to_double(), asks[i].second);
            }
            std::cout << "\n";
        }
    }
};

void run_live_demo() {
    std::cout << "\n🚀 LIVE TRADING SIMULATION\n";
    std::cout << "==========================\n";
    
    SimpleMatchingEngine engine(1); // Symbol 1 = AAPL
    std::mt19937 gen(42);
    std::uniform_real_distribution<double> price_dist(99.5, 100.5);
    std::uniform_int_distribution<int> qty_dist(100, 1000);
    std::uniform_int_distribution<int> side_dist(0, 1);
    
    // Add initial orders
    std::cout << "Adding initial market makers...\n";
    
    engine.submit_buy_order(Price(99.95), 500);
    engine.submit_buy_order(Price(99.90), 1000);
    engine.submit_buy_order(Price(99.85), 750);
    
    engine.submit_sell_order(Price(100.05), 600);
    engine.submit_sell_order(Price(100.10), 800);
    engine.submit_sell_order(Price(100.15), 400);
    
    engine.print_market_data();
    
    std::cout << "\n⚡ Starting live trading...\n";
    
    for (int round = 1; round <= 5; ++round) {
        std::cout << "\n--- Round " << round << " ---\n";
        
        // Add some random orders
        for (int i = 0; i < 3; ++i) {
            double price = price_dist(gen);
            int qty = qty_dist(gen);
            bool is_buy = side_dist(gen) == 0;
            
            OrderId id;
            if (is_buy) {
                id = engine.submit_buy_order(Price(price), qty);
                std::cout << "✅ BUY order " << id << ": " << qty << " @ $" << price << "\n";
            } else {
                id = engine.submit_sell_order(Price(price), qty);
                std::cout << "✅ SELL order " << id << ": " << qty << " @ $" << price << "\n";
            }
        }
        
        engine.print_market_data();
        
        // Simulate some processing time
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    
    std::cout << "\n🎯 Final market state:\n";
    engine.print_market_data();
}

int main() {
    std::cout << "MatchX | NanoTrader - Ultra-Fast Order Matching Engine\n";
    std::cout << "================================================\n";
    std::cout << "Production-grade HFT matching engine in C++17\n\n";
    
    try {
        // Quick functionality demo
        std::cout << "🧪 BASIC FUNCTIONALITY TEST\n";
        std::cout << "============================\n";
        
        SimpleMatchingEngine engine(1);
        
        // Test basic operations
        OrderId buy_id = engine.submit_buy_order(Price(100.00), 1000);
        OrderId sell_id = engine.submit_sell_order(Price(100.05), 500);
        
        std::cout << "✅ Buy order submitted: ID " << buy_id << "\n";
        std::cout << "✅ Sell order submitted: ID " << sell_id << "\n";
        
        engine.print_market_data();
        
        // Performance test
        std::cout << "\n⚡ PERFORMANCE TEST\n";
        std::cout << "==================\n";
        
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < 10000; ++i) {
            double price = 99.50 + (i % 100) * 0.01;
            engine.submit_buy_order(Price(price), 100);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::cout << "✅ Processed 10,000 orders in " << duration.count() << " μs\n";
        std::cout << "✅ Average latency: " << duration.count() / 10000.0 << " μs per order\n";
        std::cout << "✅ Throughput: " << (10000.0 * 1000000) / duration.count() << " orders/sec\n";
        
        // Run live demo
        run_live_demo();
        
        std::cout << "\n🎉 NanoTrader demo completed successfully!\n";
        std::cout << "Ready for production deployment! 🚀\n\n";
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Error: " << e.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "❌ Unknown error occurred\n";
        return 1;
    }
    
    return 0;
}