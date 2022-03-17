#include "nanotrader/core/matching_engine.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <random>

using namespace nanotrader;

int main() {
    std::cout << "MatchX | NanoTrader - Ultra-Fast Order Matching Engine\n";
    std::cout << "===========================================\n\n";
    
    try {
        MatchingEngine engine;
        engine.start();
        std::cout << "Engine started successfully\n";
    
    // Simple demo
    std::cout << "Running basic functionality test...\n";
    
    // Create some test orders
    Symbol symbol = 1; // AAPL
    OrderId next_order_id = 1;
    
    // Add buy order
    Order buy_order(next_order_id++, symbol, Price(100.50), 1000, 
                   Side::Buy, OrderType::Limit, now());
    OrderRequest buy_request(OrderRequest::Type::Add, buy_order);
    
    if (engine.submit_order(buy_request)) {
        std::cout << "✓ Buy order submitted: " << buy_order.id 
                  << " @ $" << buy_order.price.to_double() 
                  << " qty=" << buy_order.quantity << "\n";
    }
    
    // Add sell order at higher price
    Order sell_order1(next_order_id++, symbol, Price(100.60), 500, 
                     Side::Sell, OrderType::Limit, now());
    OrderRequest sell_request1(OrderRequest::Type::Add, sell_order1);
    
    if (engine.submit_order(sell_request1)) {
        std::cout << "✓ Sell order submitted: " << sell_order1.id 
                  << " @ $" << sell_order1.price.to_double() 
                  << " qty=" << sell_order1.quantity << "\n";
    }
    
    // Add sell order that should match
    Order sell_order2(next_order_id++, symbol, Price(100.40), 800, 
                     Side::Sell, OrderType::Limit, now());
    OrderRequest sell_request2(OrderRequest::Type::Add, sell_order2);
    
    if (engine.submit_order(sell_request2)) {
        std::cout << "✓ Matching sell order submitted: " << sell_order2.id 
                  << " @ $" << sell_order2.price.to_double() 
                  << " qty=" << sell_order2.quantity << "\n";
    }
    
    // Process orders
    engine.process_orders();
    
    // Check results
    MatchResult result;
    while (engine.get_result(result)) {
        std::cout << "\nOrder " << result.order_id << " - Status: ";
        switch (result.status) {
            case MatchResult::Status::Added:
                std::cout << "Added";
                break;
            case MatchResult::Status::Matched:
                std::cout << "Matched";
                break;
            case MatchResult::Status::Cancelled:
                std::cout << "Cancelled";
                break;
            case MatchResult::Status::Modified:
                std::cout << "Modified";
                break;
            case MatchResult::Status::Rejected:
                std::cout << "Rejected";
                break;
        }
        std::cout << "\n";
        
        if (!result.trades.empty()) {
            std::cout << "  Trades generated: " << result.trades.size() << "\n";
            for (const auto& trade : result.trades) {
                std::cout << "    Maker: " << trade.maker_order_id 
                          << " Taker: " << trade.taker_order_id
                          << " Price: $" << trade.price.to_double()
                          << " Qty: " << trade.quantity << "\n";
            }
        }
    }
    
    // Show order book state
    const OrderBook* book = engine.get_order_book(symbol);
    if (book) {
        std::cout << "\nOrder Book State:\n";
        std::cout << "Best Bid: ";
        if (book->has_best_bid()) {
            std::cout << "$" << book->get_best_bid().to_double();
        } else {
            std::cout << "None";
        }
        std::cout << "\n";
        
        std::cout << "Best Ask: ";
        if (book->has_best_ask()) {
            std::cout << "$" << book->get_best_ask().to_double();
        } else {
            std::cout << "None";
        }
        std::cout << "\n";
        
        std::cout << "Total Orders: " << book->get_order_count() << "\n";
    }
    
    std::cout << "\nTotal Processed Orders: " << engine.get_processed_orders() << "\n";
    std::cout << "Available Order Capacity: " << engine.get_available_order_capacity() << "\n";
    
        engine.stop();
        
        std::cout << "\nNanoTrader demo completed successfully!\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "Unknown error occurred\n";
        return 1;
    }
    
    return 0;
}