#include "nanotrader/core/matching_engine.hpp"
#include <algorithm>

namespace nanotrader {

// MatchingEngine constructor
MatchingEngine::MatchingEngine() : order_allocator_(1000000) {}

}