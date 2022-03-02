#include "nanotrader/core/order_book.hpp"
#include <iostream>
#include <chrono>
#include <random>
#include <vector>

using namespace nanotrader;
using namespace std::chrono;

class SimpleBenchmark {
private:
    std::mt19937 gen_;
    std::uniform_real_distribution<double> price_dist_;
    std::uniform_int_distribution<uint64_t> qty_dist_;
    std::uniform_int_distribution<int> side_dist_;
    
public:
    SimpleBenchmark() 
        : gen_(42)
        , price_dist_(99.0, 101.0)
        , qty_dist_(100, 5000)
        , side_dist_(0, 1) {}
    
    Order generate_order(OrderId id, Symbol symbol) {
        double price = price_dist_(gen_);
        uint64_t qty = qty_dist_(gen_);
        Side side = side_dist_(gen_) == 0 ? Side::Buy : Side::Sell;
        
        return Order(id, symbol, Price(price), qty, side, OrderType::Limit, now());
    }
    
    void benchmark_order_book(size_t num_orders) {
        std::cout << "\n=== Order Book Benchmark ===\n";
        std::cout << "Orders to process: " << num_orders << "\n";
        
        OrderBook book(1);
        std::vector<Order> orders;
        orders.reserve(num_orders);
        
        // Generate orders
        std::cout << "Generating orders...\n";
        for (size_t i = 0; i < num_orders; ++i) {
            orders.emplace_back(generate_order(i + 1, 1));
        }
        
        // Benchmark order addition
        auto start = high_resolution_clock::now();
        
        for (auto& order : orders) {
            book.add_order(&order);
        }
        
        auto end = high_resolution_clock::now();
        auto duration = duration_cast<nanoseconds>(end - start);
        
        double avg_latency_ns = static_cast<double>(duration.count()) / num_orders;
        double throughput = num_orders * 1e9 / duration.count();
        
        std::cout << "\nResults:\n";
        std::cout << "Total time: " << duration.count() / 1e6 << " ms\n";
        std::cout << "Average latency: " << avg_latency_ns << " ns per order\n";
        std::cout << "Throughput: " << static_cast<uint64_t>(throughput) << " orders/sec\n";
        
        std::cout << "\nFinal order book state:\n";
        std::cout << "Total orders: " << book.get_order_count() << "\n";
        if (book.has_best_bid()) {
            std::cout << "Best bid: $" << book.get_best_bid().to_double() << "\n";
        }
        if (book.has_best_ask()) {
            std::cout << "Best ask: $" << book.get_best_ask().to_double() << "\n";
        }
        
        // Show top levels
        auto bid_levels = book.get_bid_levels(3);
        std::cout << "\nTop 3 bid levels:\n";
        for (const auto& [price, qty] : bid_levels) {
            std::cout << "  $" << price.to_double() << " - " << qty << " shares\n";
        }
        
        auto ask_levels = book.get_ask_levels(3);
        std::cout << "\nTop 3 ask levels:\n";
        for (const auto& [price, qty] : ask_levels) {
            std::cout << "  $" << price.to_double() << " - " << qty << " shares\n";
        }
    }
    
    void benchmark_price_operations() {
        std::cout << "\n=== Price Operations Benchmark ===\n";
        
        const size_t num_ops = 10000000;
        std::vector<Price> prices;
        prices.reserve(num_ops);
        
        // Generate prices
        for (size_t i = 0; i < num_ops; ++i) {
            prices.emplace_back(Price(price_dist_(gen_)));
        }
        
        // Benchmark price comparisons
        auto start = high_resolution_clock::now();
        
        size_t count = 0;
        for (size_t i = 1; i < prices.size(); ++i) {
            if (prices[i] > prices[i-1]) {
                count++;
            }
        }
        
        auto end = high_resolution_clock::now();
        auto duration = duration_cast<nanoseconds>(end - start);
        
        double avg_latency_ns = static_cast<double>(duration.count()) / (num_ops - 1);
        
        std::cout << "Price comparisons: " << num_ops - 1 << "\n";
        std::cout << "Average latency: " << avg_latency_ns << " ns per comparison\n";
        std::cout << "Greater count: " << count << "\n";
    }
};

int main() {
    std::cout << "NanoTrader Performance Benchmarks\n";
    std::cout << "=================================\n";
    
    SimpleBenchmark bench;
    
    // Test different order counts
    std::vector<size_t> order_counts = {1000, 10000, 100000};
    
    for (size_t count : order_counts) {
        bench.benchmark_order_book(count);
    }
    
    bench.benchmark_price_operations();
    
    std::cout << "\nBenchmarks completed!\n";
    
    return 0;
}