// evmone: Fast Ethereum Virtual Machine implementation
// Copyright 2019-2020 The evmone Authors.
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <evmc/evmc.hpp>
#include <intx/intx.hpp>
#include <string>
#include <vector>

namespace evmone
{
using uint256 = intx::uint256;
using bytes = std::basic_string<uint8_t>;
using bytes_view = std::basic_string_view<uint8_t>;


/// The stack for 256-bit EVM words.
///
/// This implementation reserves memory inplace for all possible stack items (1024),
/// so this type is big. Make sure it is allocated on heap.
struct evm_stack
{
    /// The maximum number of stack items.
    static constexpr auto limit = 1024;

    /// The pointer to the top item, or below the stack bottom if stack is empty.
    intx::uint256* top_item;

    /// The storage allocated for maximum possible number of items.
    /// This is also the pointer to the bottom item.
    /// Items are aligned to 256 bits for better packing in cache lines.
    alignas(sizeof(intx::uint256)) intx::uint256 storage[limit];

    /// Default constructor. Sets the top_item pointer to below the stack bottom.
    evm_stack() noexcept { clear(); }

    /// The current number of items on the stack.
    [[nodiscard]] int size() const noexcept { return static_cast<int>(top_item + 1 - storage); }

    /// Returns the reference to the top item.
    [[nodiscard]] intx::uint256& top() noexcept { return *top_item; }

    /// Returns the reference to the stack item on given position from the stack top.
    [[nodiscard]] intx::uint256& operator[](int index) noexcept { return *(top_item - index); }

    /// Pushes an item on the stack. The stack limit is not checked.
    void push(const intx::uint256& item) noexcept { *++top_item = item; }

    /// Returns an item popped from the top of the stack.
    intx::uint256 pop() noexcept { return *top_item--; }

    /// Clears the stack by resetting its size to 0 (sets the top_item pointer to below the stack
    /// bottom).
    [[clang::no_sanitize("bounds")]] void clear() noexcept { top_item = storage - 1; }
};

/// The EVM memory.
///
/// Use 512 KB as max_evm_memory_size of evm_memory,
/// since there is only 1 MB heap memory to run contracts in CKB VM.
class evm_memory
{
    inline static uint8_t* m_memory;
    inline static uint8_t* end;
    inline static uint8_t* used_ptr;
    uint8_t* begin = nullptr;

public:
    /// The initial memory allocation
    static void init(uint8_t stack_mem[], const size_t len) {
        m_memory = stack_mem;
        end = m_memory + len;
        used_ptr = m_memory;
    }

    evm_memory() noexcept {
        begin = used_ptr;
    }

    evm_memory(const evm_memory&) = delete;
    evm_memory& operator=(const evm_memory&) = delete;

    uint8_t& operator[](size_t idx) noexcept {
        return m_memory[static_cast<size_t>(begin - m_memory) + idx];
    }

    [[nodiscard]] size_t size() const noexcept {
        return static_cast<size_t>(used_ptr - begin);
    }

    bool update_used_ptr(size_t new_position) {
        used_ptr = begin + new_position;

        // out of bounds
        if (used_ptr > end) return false;

        return true;
    }

    /// get total used memory (bytes) of m_memory
    uint32_t used_memory() const noexcept {
        return static_cast<uint32_t>((used_ptr - m_memory));
    }

    bool resize(size_t new_size) {
        used_ptr = begin + new_size;

        // out of bounds
        if (used_ptr > end) {
            return false;
        }

        // TODO: try to allocate more memory to m_memory
        // and catch OUT_OF_MEMORY error if failed
        return true;
    }

    void clear() noexcept {
        printf("[evmone.evm_memory] clear()");
        memset(begin, 0, used_ptr - begin);
        used_ptr = begin;
    }
    ~evm_memory() {
        clear();
    }
};

/// Generic execution state for generic instructions implementations.
struct ExecutionState
{
    int64_t gas_left = 0;
    evm_stack stack;
    evm_memory memory;
    const evmc_message* msg = nullptr;
    evmc::HostContext host;
    evmc_revision rev = {};
    bytes return_data;
    bytes_view code;
    evmc_status_code status = EVMC_SUCCESS;
    size_t output_offset = 0;
    size_t output_size = 0;

    ExecutionState() noexcept = default;

    ExecutionState(const evmc_message& message, evmc_revision revision,
        const evmc_host_interface& host_interface, evmc_host_context* host_ctx,
        const uint8_t* code_ptr, size_t code_size) noexcept
      : gas_left{message.gas},
        msg{&message},
        host{host_interface, host_ctx},
        rev{revision},
        code{code_ptr, code_size}
    {}

    /// Resets the contents of the ExecutionState so that it could be reused.
    void reset(const evmc_message& message, evmc_revision revision,
        const evmc_host_interface& host_interface, evmc_host_context* host_ctx,
        const uint8_t* code_ptr, size_t code_size) noexcept
    {
        printf("[evmone] ExecutionState::reset");
        gas_left = message.gas;
        stack.clear();
        memory.clear();
        msg = &message;
        host = {host_interface, host_ctx};
        rev = revision;
        return_data.clear();
        code = {code_ptr, code_size};
        status = EVMC_SUCCESS;
        output_offset = 0;
        output_size = 0;
    }
};
}  // namespace evmone
