#include "UUID.h"

#include <random>

namespace base_engine {

//乱数生成
static std::random_device random_device;
static std::mt19937_64 eng(random_device());
static std::uniform_int_distribution<uint64_t> uniform_distribution;

static std::mt19937 eng32(random_device());
static std::uniform_int_distribution<uint32_t> uniform_distribution32;

UUID::UUID() : uuid_(uniform_distribution(eng)) {}

UUID32::UUID32() : uuid_(uniform_distribution32(eng32)) {}


}  // namespace base_engine