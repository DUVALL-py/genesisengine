#pragma once
// =============================================================================
// Genesis::Core::Memory::LinearAllocator
//
// Allocator de arena: aloca sempre avançando um ponteiro, nunca libera
// individualmente. Ideal para dados de tempo de vida de 1 frame ou de 1 job.
// Reset() volta o ponteiro ao início em O(1).
// =============================================================================

#include <cstddef>
#include <new>

#include "genesis/core/memory/Allocator.hpp"

namespace genesis::core::memory {

class LinearAllocator final : public IAllocator {
public:
    explicit LinearAllocator(std::size_t capacityBytes, const char* debugName = "LinearAllocator")
        : m_capacity(capacityBytes), m_debugName(debugName) {
        m_begin = static_cast<std::byte*>(::operator new(capacityBytes, std::nothrow));
        m_current = m_begin;
    }

    ~LinearAllocator() override { ::operator delete(m_begin); }

    LinearAllocator(const LinearAllocator&) = delete;
    LinearAllocator& operator=(const LinearAllocator&) = delete;

    [[nodiscard]] void* Allocate(std::size_t sizeBytes, std::size_t alignment) override {
        if (m_begin == nullptr || sizeBytes == 0) return nullptr;
        if (!IsPowerOfTwo(alignment)) return nullptr;

        auto currentAddr = reinterpret_cast<std::uintptr_t>(m_current);
        auto alignedAddr = AlignUp(currentAddr, alignment);
        auto endAddr = alignedAddr + sizeBytes;
        auto capacityEndAddr = reinterpret_cast<std::uintptr_t>(m_begin) + m_capacity;

        if (endAddr > capacityEndAddr) {
            return nullptr;  // arena esgotada
        }

        m_current = reinterpret_cast<std::byte*>(endAddr);
        m_bytesAllocated = static_cast<std::size_t>(endAddr - reinterpret_cast<std::uintptr_t>(m_begin));
        m_peak = m_bytesAllocated > m_peak ? m_bytesAllocated : m_peak;
        ++m_allocationCount;
        return reinterpret_cast<void*>(alignedAddr);
    }

    // No-op intencional: LinearAllocator libera tudo de uma vez via Reset().
    void Deallocate(void* /*ptr*/) override {}

    void Reset() noexcept {
        m_current = m_begin;
        m_bytesAllocated = 0;
        m_allocationCount = 0;
    }

    [[nodiscard]] AllocationStats Stats() const noexcept override {
        return AllocationStats{
            .bytesAllocated = m_bytesAllocated,
            .bytesReserved = m_capacity,
            .allocationCount = m_allocationCount,
            .peakBytesAllocated = m_peak,
        };
    }

    [[nodiscard]] const char* DebugName() const noexcept override { return m_debugName; }

private:
    std::byte* m_begin = nullptr;
    std::byte* m_current = nullptr;
    std::size_t m_capacity = 0;
    std::size_t m_bytesAllocated = 0;
    std::size_t m_allocationCount = 0;
    std::size_t m_peak = 0;
    const char* m_debugName;
};

}  // namespace genesis::core::memory
