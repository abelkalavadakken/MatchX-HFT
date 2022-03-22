#include "nanotrader/core/order_book.hpp"
#include <iostream>

using namespace nanotrader;

int main() {
    std::cout << "NanoTrader - Simple Order Book Test\n";
    std::cout << "===================================\n\n";
    
    try {
        // Create order book
        Symbol symbol = 1;
        OrderBook book(symbol);
        
        std::cout << "Order book created for symbol " << symbol << "\n";
        
        // Create some orders manually
        Order buy_order1(1, symbol, Price(100.50), 1000, Side::Buy, OrderType::Limit, now());
        Order buy_order2(2, symbol, Price(100.40), 500, Side::Buy, OrderType::Limit, now());
        Order sell_order1(3, symbol, Price(100.60), 800, Side::Sell, OrderType::Limit, now());
        
        // Add orders to book
        bool added1 = book.add_order(&buy_order1);
        bool added2 = book.add_order(&buy_order2);
        bool added3 = book.add_order(&sell_order1);
        
        std::cout << "Buy order 1 added: " << (added1 ? "Yes" : "No") << "\n";
        std::cout << "Buy order 2 added: " << (added2 ? "Yes" : "No") << "\n";
        std::cout << "Sell order 1 added: " << (added3 ? "Yes" : "No") << "\n";
        
        // Check best bid/ask
        if (book.has_best_bid()) {
            std::cout << "Best bid: $" << book.get_best_bid().to_double() << "\n";
        }
        
        if (book.has_best_ask()) {
            std::cout << "Best ask: $" << book.get_best_ask().to_double() << "\n";
        }
        
        std::cout << "Total orders: " << book.get_order_count() << "\n";
        
        // Test level-by-level view
        auto bid_levels = book.get_bid_levels(5);
        std::cout << "\nBid levels:\n";
        for (const auto& [price, qty] : bid_levels) {
            std::cout << "  $" << price.to_double() << " - " << qty << " shares\n";
        }
        
        auto ask_levels = book.get_ask_levels(5);
        std::cout << "\nAsk levels:\n";
        for (const auto& [price, qty] : ask_levels) {
            std::cout << "  $" << price.to_double() << " - " << qty << " shares\n";
        }
        
        std::cout << "\nTest completed successfully!\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "Unknown error occurred\n";
        return 1;
    }
    
    return 0;
}