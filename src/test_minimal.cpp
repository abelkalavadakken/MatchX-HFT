#include <iostream>
#include "nanotrader/core/types.hpp"
#include "nanotrader/core/order.hpp"

using namespace nanotrader;

int main() {
    std::cout << "Testing basic types...\n";
    
    // Test Price
    Price p(100.50);
    std::cout << "Price: " << p.to_double() << "\n";
    
    // Test Order
    std::cout << "Order size: " << sizeof(Order) << " bytes\n";
    
    Order order(1, 100, Price(99.50), 1000, Side::Buy, OrderType::Limit, now());
    std::cout << "Order created successfully\n";
    std::cout << "Order ID: " << order.id << "\n";
    std::cout << "Price: $" << order.price.to_double() << "\n";
    std::cout << "Quantity: " << order.quantity << "\n";
    
    return 0;
}