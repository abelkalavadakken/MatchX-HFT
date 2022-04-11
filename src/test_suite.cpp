#include "nanotrader/core/order_book.hpp"
#include "nanotrader/memory/ring_buffer.hpp"
#include <iostream>
#include <cassert>

using namespace nanotrader;

void test_price_operations() {
    std::cout << "Testing Price operations... ";
    
    Price p1(100.50);
    Price p2(100.60);
    Price p3(100.50);
    
    assert(p1 < p2);
    assert(p2 > p1);
    assert(p1 == p3);
    assert(p1 != p2);
    assert(p1 <= p2);
    assert(p1 <= p3);
    assert(p2 >= p1);
    
    assert(p1.to_double() == 100.50);
    assert(p1.raw_value() == 100500000);
    
    std::cout << "✓ PASSED\n";
}

void test_order_creation() {
    std::cout << "Testing Order creation... ";
    
    Order order(1, 100, Price(99.75), 1000, Side::Buy, OrderType::Limit, now());
    
    assert(order.id == 1);
    assert(order.symbol == 100);
    assert(order.price == Price(99.75));
    assert(order.quantity == 1000);
    assert(order.remaining_quantity == 1000);
    assert(order.side == Side::Buy);
    assert(order.type == OrderType::Limit);
    assert(order.is_buy());
    assert(!order.is_sell());
    assert(order.is_limit());
    assert(!order.is_market());
    assert(!order.is_filled());
    
    // Test fill
    order.fill(300);
    assert(order.remaining_quantity == 700);
    assert(!order.is_filled());
    
    order.fill(700);
    assert(order.remaining_quantity == 0);
    assert(order.is_filled());
    
    std::cout << "✓ PASSED\n";
}

void test_order_book_basic() {
    std::cout << "Testing OrderBook basic operations... ";
    
    OrderBook book(1);
    
    // Initially empty
    assert(!book.has_best_bid());
    assert(!book.has_best_ask());
    assert(book.get_order_count() == 0);
    
    // Add buy order
    Order buy_order(1, 1, Price(100.50), 1000, Side::Buy, OrderType::Limit, now());
    assert(book.add_order(&buy_order));
    
    assert(book.has_best_bid());
    assert(!book.has_best_ask());
    assert(book.get_best_bid() == Price(100.50));
    assert(book.get_order_count() == 1);
    
    // Add sell order
    Order sell_order(2, 1, Price(100.60), 500, Side::Sell, OrderType::Limit, now());
    assert(book.add_order(&sell_order));
    
    assert(book.has_best_bid());
    assert(book.has_best_ask());
    assert(book.get_best_bid() == Price(100.50));
    assert(book.get_best_ask() == Price(100.60));
    assert(book.get_order_count() == 2);
    
    // Add better buy order
    Order better_buy(3, 1, Price(100.55), 300, Side::Buy, OrderType::Limit, now());
    assert(book.add_order(&better_buy));
    
    assert(book.get_best_bid() == Price(100.55));
    assert(book.get_order_count() == 3);
    
    std::cout << "✓ PASSED\n";
}

void test_order_book_removal() {
    std::cout << "Testing OrderBook order removal... ";
    
    OrderBook book(1);
    
    Order order1(1, 1, Price(100.50), 1000, Side::Buy, OrderType::Limit, now());
    Order order2(2, 1, Price(100.60), 500, Side::Sell, OrderType::Limit, now());
    
    book.add_order(&order1);
    book.add_order(&order2);
    
    assert(book.get_order_count() == 2);
    assert(book.get_order(1) == &order1);
    assert(book.get_order(2) == &order2);
    
    // Remove buy order
    assert(book.remove_order(1));
    assert(book.get_order_count() == 1);
    assert(book.get_order(1) == nullptr);
    assert(!book.has_best_bid());
    
    // Remove sell order
    assert(book.remove_order(2));
    assert(book.get_order_count() == 0);
    assert(book.get_order(2) == nullptr);
    assert(!book.has_best_ask());
    
    // Try to remove non-existent order
    assert(!book.remove_order(999));
    
    std::cout << "✓ PASSED\n";
}

void test_order_book_price_levels() {
    std::cout << "Testing OrderBook price levels... ";
    
    OrderBook book(1);
    
    // Add multiple orders at same price
    Order buy1(1, 1, Price(100.50), 1000, Side::Buy, OrderType::Limit, now());
    Order buy2(2, 1, Price(100.50), 500, Side::Buy, OrderType::Limit, now());
    Order buy3(3, 1, Price(100.40), 300, Side::Buy, OrderType::Limit, now());
    
    book.add_order(&buy1);
    book.add_order(&buy2);
    book.add_order(&buy3);
    
    const PriceLevel* level = book.get_buy_level(Price(100.50));
    assert(level != nullptr);
    assert(level->total_quantity == 1500);
    assert(level->head == &buy1);  // FIFO order
    
    auto bid_levels = book.get_bid_levels(5);
    assert(bid_levels.size() == 2);
    assert(bid_levels[0].first == Price(100.50));  // Highest first
    assert(bid_levels[0].second == 1500);
    assert(bid_levels[1].first == Price(100.40));
    assert(bid_levels[1].second == 300);
    
    std::cout << "✓ PASSED\n";
}

void test_ring_buffer() {
    std::cout << "Testing SPSC Ring Buffer... ";
    
    SPSCRingBuffer<int, 8> buffer;
    
    assert(buffer.empty());
    assert(!buffer.full());
    assert(buffer.size() == 0);
    assert(buffer.capacity() == 7);  // Size - 1
    
    // Fill buffer
    for (int i = 0; i < 7; ++i) {
        assert(buffer.try_push(i));
    }
    
    assert(buffer.full());
    assert(!buffer.empty());
    assert(buffer.size() == 7);
    
    // Buffer full, should fail
    assert(!buffer.try_push(999));
    
    // Pop some items
    int item;
    for (int i = 0; i < 3; ++i) {
        assert(buffer.try_pop(item));
        assert(item == i);
    }
    
    assert(buffer.size() == 4);
    assert(!buffer.full());
    
    // Pop remaining
    for (int i = 3; i < 7; ++i) {
        assert(buffer.try_pop(item));
        assert(item == i);
    }
    
    assert(buffer.empty());
    assert(!buffer.try_pop(item));
    
    std::cout << "✓ PASSED\n";
}

int main() {
    std::cout << "NanoTrader Test Suite\n";
    std::cout << "====================\n\n";
    
    try {
        test_price_operations();
        test_order_creation();
        test_order_book_basic();
        test_order_book_removal();
        test_order_book_price_levels();
        test_ring_buffer();
        
        std::cout << "\n🎉 All tests PASSED!\n";
        
    } catch (const std::exception& e) {
        std::cerr << "\n❌ Test FAILED: " << e.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "\n❌ Test FAILED with unknown error\n";
        return 1;
    }
    
    return 0;
}