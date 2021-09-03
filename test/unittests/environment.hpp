#define _EVMONE_UNITTEST

#include <evmone/execution_state.hpp>
#include <gtest/gtest.h>

// https://google.github.io/googletest/advanced.html#global-set-up-and-tear-down
class EvmMemory : public ::testing::Environment {
 public:
  // define how to set up the environment.
  // void SetUp() override {
  //   static uint8_t stack_mem[257 * 1024];
  //   evmone::evm_memory::init(stack_mem, 257 * 1024);
  // }

  // define how to tear down the environment.
  // void TearDown() override {}
};

// TODO: Usage: 
// testing::Environment* const init = testing::AddGlobalTestEnvironment(new EvmMemory);
// or
// int main() {
//   testing::Environment* const init = testing::AddGlobalTestEnvironment(new EvmMemory);
//   return RUN_ALL_TESTS();
// }