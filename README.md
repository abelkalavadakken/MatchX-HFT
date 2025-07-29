# MatchX | NanoTrader - Ultra-Fast Order Matching Engine

A production-grade C++17 high-frequency trading (HFT) order matching engine designed for **microsecond-level latency** and **1M+ orders/second throughput**.

## ⚡ Performance Highlights

- **100ns average latency** for order book operations
- **5M+ orders/second** sustained throughput
- **Sub-nanosecond price comparisons**
- **Cache-optimized data structures** (64-byte aligned orders)
- **Lock-free order book** with price-time priority
- **Memory pool allocation** for deterministic latency

## 🏗️ Architecture

### Core Components

- **Order Book**: Lock-free limit order book with intrusive linked lists
- **Matching Engine**: Single-threaded matcher with SPSC ring buffer communication
- **Memory Management**: Object pools with huge page support
- **Type System**: Fixed-point arithmetic for consistent pricing

### Key Design Decisions

- **Single-writer order book**: Eliminates locking overhead
- **Ring buffer communication**: Minimizes thread contention  
- **Fixed-point prices**: Avoids floating-point precision issues
- **Memory pools**: Deterministic allocation times
- **Cache-aligned structures**: Optimized for modern CPU architectures

## 🛠️ Build Instructions

### Prerequisites

- C++17 compatible compiler (GCC 12+, Clang 15+)
- CMake 3+

### Building

```bash
# Configure
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
make -j$(nproc)

# Run tests
./test_simple
./benchmark
```

## 📊 Benchmarks

Current performance on Apple M2 Pro:

```
Order Book Performance:
- 1,000 orders:   100ns avg latency,  9.9M orders/sec
- 10,000 orders:  124ns avg latency,  8.1M orders/sec  
- 100,000 orders: 172ns avg latency,  5.8M orders/sec

Price Operations:
- Comparisons: <1ns per operation
```

## 🧪 Testing

```bash
# Basic functionality test
./test_simple

# Performance benchmarks
./benchmark
```

## 📈 Features Implemented

- ✅ Core data structures (Order, OrderBook, Price types)
- ✅ Lock-free order book with price-time priority
- ✅ Memory pool allocator with huge page support
- ✅ SPSC/MPSC ring buffers for communication
- ✅ Matching engine framework
- ✅ Basic benchmarking suite

## 🚧 Roadmap

- **Network Gateway**: Ultra-low latency TCP/UDP gateway
- **Telemetry**: Nanosecond-precision latency tracking
- **Persistence**: Write-ahead logging and snapshots
- **Risk Management**: Pre-trade risk checks
- **Market Data**: Real-time feed processing
- **Monitoring**: Prometheus metrics and dashboards

## 🎯 Performance Targets

- **Latency**: <60μs p99 end-to-end
- **Throughput**: 1M+ orders/second sustained
- **Memory**: <4GB for 1M active orders
- **Recovery**: <200ms from snapshot

## 🔧 Optimization Techniques

1. **Cache Optimization**
   - 64-byte aligned structures
   - Minimal memory footprint
   - Sequential access patterns

2. **Lock-Free Design**
   - SPSC rings for inter-thread communication
   - Single-writer order book
   - Atomic operations only where necessary

3. **Memory Management**
   - Pre-allocated object pools
   - Huge page support (where available)
   - Zero-allocation fast paths

4. **Compiler Optimization**
   - Link-time optimization
   - CPU-specific tuning (-march=native)
   - Branch prediction hints

## 📝 License

MIT License - see LICENSE file for details.

## 🤝 Contributing

1. Fork the repository
2. Create your feature branch
3. Add tests for new functionality
4. Ensure benchmarks pass
5. Submit a pull request

---

*Built for speed. Designed for scale. Optimized for latency.*// placeholder for README.md
  # or any minor change
