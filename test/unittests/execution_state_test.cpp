// evmone: Fast Ethereum Virtual Machine implementation
// Copyright 2020 The evmone Authors.
// SPDX-License-Identifier: Apache-2.0

#include "environment.hpp"
#include <evmone/analysis.hpp>
#include <type_traits>
#include <evmone/execution_state.cpp>

static_assert(std::is_default_constructible<evmone::ExecutionState>::value);
static_assert(!std::is_move_constructible<evmone::ExecutionState>::value);
static_assert(!std::is_copy_constructible<evmone::ExecutionState>::value);
static_assert(!std::is_move_assignable<evmone::ExecutionState>::value);
static_assert(!std::is_copy_assignable<evmone::ExecutionState>::value);

static_assert(std::is_default_constructible<evmone::execution_state>::value);
static_assert(!std::is_move_constructible<evmone::execution_state>::value);
static_assert(!std::is_copy_constructible<evmone::execution_state>::value);
static_assert(!std::is_move_assignable<evmone::execution_state>::value);
static_assert(!std::is_copy_assignable<evmone::execution_state>::value);

TEST(execution_state, construct)
{
    evmc_message msg{};
    msg.gas = -1;
    const evmc_host_interface host_interface{};
    const uint8_t code[]{0x0f};
    const evmone::ExecutionState st{
        msg, EVMC_MAX_REVISION, host_interface, nullptr, code, std::size(code)};

    EXPECT_EQ(st.gas_left, -1);
    EXPECT_EQ(st.stack.size(), 0);
    EXPECT_EQ(st.memory.size(), 0);
    EXPECT_EQ(st.msg, &msg);
    EXPECT_EQ(st.rev, EVMC_MAX_REVISION);
    EXPECT_EQ(st.return_data.size(), 0);
    EXPECT_EQ(st.code.data(), &code[0]);
    EXPECT_EQ(st.code.size(), std::size(code));
    EXPECT_EQ(st.status, EVMC_SUCCESS);
    EXPECT_EQ(st.output_offset, 0);
    EXPECT_EQ(st.output_size, 0);
}

TEST(execution_state, default_construct)
{
    const evmone::ExecutionState st;

    EXPECT_EQ(st.gas_left, 0);
    EXPECT_EQ(st.stack.size(), 0);
    EXPECT_EQ(st.memory.size(), 0);
    EXPECT_EQ(st.msg, nullptr);
    EXPECT_EQ(st.rev, EVMC_FRONTIER);
    EXPECT_EQ(st.return_data.size(), 0);
    EXPECT_EQ(st.code.data(), nullptr);
    EXPECT_EQ(st.code.size(), 0);
    EXPECT_EQ(st.status, EVMC_SUCCESS);
    EXPECT_EQ(st.output_offset, 0);
    EXPECT_EQ(st.output_size, 0);
}

TEST(execution_state, default_construct_advanced)
{
    const evmone::execution_state st;

    EXPECT_EQ(st.gas_left, 0);
    EXPECT_EQ(st.stack.size(), 0);
    EXPECT_EQ(st.memory.size(), 0);
    EXPECT_EQ(st.msg, nullptr);
    EXPECT_EQ(st.rev, EVMC_FRONTIER);
    EXPECT_EQ(st.return_data.size(), 0);
    EXPECT_EQ(st.code.data(), nullptr);
    EXPECT_EQ(st.code.size(), 0);
    EXPECT_EQ(st.status, EVMC_SUCCESS);
    EXPECT_EQ(st.output_offset, 0);
    EXPECT_EQ(st.output_size, 0);

    EXPECT_EQ(st.current_block_cost, 0u);
    EXPECT_EQ(st.analysis, nullptr);
}

TEST(execution_state, reset_advanced)
{
    const evmc_message msg{};
    const uint8_t code[]{0xff};
    evmone::code_analysis analysis;

    evmone::execution_state st;
    st.gas_left = 1;
    st.stack.push({});
    st.memory.resize(2);
    st.msg = &msg;
    st.rev = EVMC_BYZANTIUM;
    st.return_data.push_back('0');
    st.code = {code, std::size(code)};
    st.status = EVMC_FAILURE;
    st.output_offset = 3;
    st.output_size = 4;
    st.current_block_cost = 5;
    st.analysis = &analysis;

    EXPECT_EQ(st.gas_left, 1);
    EXPECT_EQ(st.stack.size(), 1);
    EXPECT_EQ(st.memory.size(), 2);
    EXPECT_EQ(st.msg, &msg);
    EXPECT_EQ(st.rev, EVMC_BYZANTIUM);
    EXPECT_EQ(st.return_data.size(), 1);
    EXPECT_EQ(st.code.data(), &code[0]);
    EXPECT_EQ(st.code.size(), 1);
    EXPECT_EQ(st.status, EVMC_FAILURE);
    EXPECT_EQ(st.output_offset, 3);
    EXPECT_EQ(st.output_size, 4u);
    EXPECT_EQ(st.current_block_cost, 5u);
    EXPECT_EQ(st.analysis, &analysis);

    {
        evmc_message msg2{};
        msg2.gas = 13;
        const evmc_host_interface host_interface2{};
        const uint8_t code2[]{0x80, 0x81};
        evmone::code_analysis analysis2;

        st.reset(
            msg2, EVMC_HOMESTEAD, host_interface2, nullptr, code2, std::size(code2), analysis2);

        // TODO: We are not able to test HostContext with current API. It may require an execution
        //       test.
        EXPECT_EQ(st.gas_left, 13);
        EXPECT_EQ(st.stack.size(), 0);
        EXPECT_EQ(st.memory.size(), 0);
        EXPECT_EQ(st.msg, &msg2);
        EXPECT_EQ(st.rev, EVMC_HOMESTEAD);
        EXPECT_EQ(st.return_data.size(), 0);
        EXPECT_EQ(st.code.data(), &code2[0]);
        EXPECT_EQ(st.code.size(), 2);
        EXPECT_EQ(st.status, EVMC_SUCCESS);
        EXPECT_EQ(st.output_offset, 0);
        EXPECT_EQ(st.output_size, 0);
        EXPECT_EQ(st.current_block_cost, 0u);
        EXPECT_EQ(st.analysis, &analysis2);
    }
}

TEST(execution_state, stack_clear)
{
    evmone::evm_stack stack;

    stack.clear();
    EXPECT_EQ(stack.size(), 0);
    EXPECT_EQ(stack.top_item + 1, stack.storage);

    stack.push({});
    EXPECT_EQ(stack.size(), 1);
    EXPECT_EQ(stack.top_item, stack.storage);

    stack.clear();
    EXPECT_EQ(stack.size(), 0);
    EXPECT_EQ(stack.top_item + 1, stack.storage);

    stack.clear();
    EXPECT_EQ(stack.size(), 0);
    EXPECT_EQ(stack.top_item + 1, stack.storage);
}
