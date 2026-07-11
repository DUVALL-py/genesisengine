#pragma once
// =============================================================================
// Genesis::Core::Memory::PoolAllocator
//
// Allocator de blocos de tamanho fixo (todos os blocos têm o mesmo tamanho e
// alinhamento, definidos na construção). Aloca/libera em O(1) usando uma
// free list intrusiva armazenada nos próprios blocos livres. Uso típico:
// componentes de ECS, nós de física, partículas.
// =============================================================================

#include <cstddef>
#include <new>

#include "genesis/core/memory/Allocator.hpp"

namespace genesis::core::memory {

class PoolAllocator final : public IAllocator {
public:
    PoolAllocator(std::size_t blockSize, std::size_t blockCount, std::size_t blockAlignment = alignof(std::max_align_t),
                  const char* debugName = "PoolAllocator")
        : m_blockSize(blockSize < sizeof(void*) ? sizeof(void*) : blockSize),
          m_blockCount(blockCount),
          m_alignment(blockAlignment),
          m_debugName(debugName) {
        const std::size_t totalSize = m_blockSize * m_blockCount + m_alignment;
        m_begin = static_cast<std::byte*>(::operator new(totalSize, std::nothrow));
        if (m_begin != nullptr) {
            BuildFreeList();
        }
    }

    ~PoolAllocator() override { ::operator delete(m_begin); }

    PoolAllocator(const PoolAllocator&) = delete;
    PoolAllocator& operator=(const PoolAllocator&) = delete;

    [[nodiscard]] void* Allocate(std::size_t sizeBytes, std::size_t alignment) override {
        if (m_freeList == nullptr) return nullptr;
        if (sizeBytes > m_blockSize) return nullptr;
        if (alignment > m_alignment) return nullptr;

        void* block = m_freeList;
        m_freeList = *reinterpret_cast<void**>(m_freeList);
        m_bytesAllocated += m_blockSize;
        ++m_allocationCount;
        m_peak = m_bytesAllocated > m_peak ? m_bytesAllocated : m_peak;
        return block;
    }

    void Deallocate(void* ptr) override {
        if (ptr == nullptr) return;
        *reinterpret_cast<void**>(ptr) = m_freeList;
        m_freeList = ptr;
        m_bytesAllocated -= m_blockSize;
        --m_allocationCount;
    }

    [[nodiscard]] AllocationStats Stats() const noexcept override {
        return AllocationStats{
            .bytesAllocated = m_bytesAllocated,
            .bytesReserved = m_blockSize * m_blockCount,
            .allocationCount = m_allocationCount,
            .peakBytesAllocated = m_peak,
        };
    }

    [[nodiscard]] const char* DebugName() const noexcept override { return m_debugName; }
    [[nodiscard]] std::size_t BlockSize() const noexcept { return m_blockSize; }
    [[nodiscard]] std::size_t BlockCount() const noexcept { return m_blockCount; }

private:
    void BuildFreeList() {
        auto base = reinterpret_cast<std::uintptr_t>(m_begin);
        auto aligned = AlignUp(base, m_alignment);
        m_alignedBegin = reinterpret_cast<std::byte*>(aligned);

        m_freeList = nullptr;
        for (std::size_t i = m_blockCount; i-- > 0;) {
            void* block = m_alignedBegin + i * m_blockSize;
            *reinterpret_cast<void**>(block) = m_freeList;
            m_freeList = block;
        }
    }

    std::byte* m_begin = nullptr;
    std::byte* m_alignedBegin = nullptr;
    void* m_freeList = nullptr;
    std::size_t m_blockSize;
    std::size_t m_blockCount;
    std::size_t m_alignment;
    std::size_t m_bytesAllocated = 0;
    std::size_t m_allocationCount = 0;
    std::size_t m_peak = 0;
    const char* m_debugName;
};

}  // namespace genesis::core::memory
