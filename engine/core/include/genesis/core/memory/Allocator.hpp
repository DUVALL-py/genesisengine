#pragma once
// =============================================================================
// Genesis::Core::Memory::Allocator
//
// Interface base para todos os allocators da engine. Nenhum sistema de
// runtime deve chamar new/malloc diretamente — tudo passa por um Allocator,
// para permitir tracking, budgets por subsistema, e allocators customizados
// (linear, pool, stack, freelist) sem tocar no código cliente.
// =============================================================================

#include <cstddef>
#include <cstdint>

namespace genesis::core::memory {

struct AllocationStats {
    std::size_t bytesAllocated = 0;   // bytes atualmente em uso
    std::size_t bytesReserved = 0;    // capacidade total do allocator
    std::size_t allocationCount = 0;  // alocações ativas
    std::size_t peakBytesAllocated = 0;
};

class IAllocator {
public:
    virtual ~IAllocator() = default;

    // alignment deve ser potência de 2. Retorna nullptr em falha (nunca lança).
    [[nodiscard]] virtual void* Allocate(std::size_t sizeBytes, std::size_t alignment = alignof(std::max_align_t)) = 0;

    // Nem todo allocator suporta free individual (ex.: LinearAllocator não suporta;
    // Deallocate() é no-op nesse caso — ver Reset()).
    virtual void Deallocate(void* ptr) = 0;

    [[nodiscard]] virtual AllocationStats Stats() const noexcept = 0;

    [[nodiscard]] virtual const char* DebugName() const noexcept = 0;
};

[[nodiscard]] constexpr bool IsPowerOfTwo(std::size_t value) noexcept {
    return value != 0 && (value & (value - 1)) == 0;
}

[[nodiscard]] constexpr std::uintptr_t AlignUp(std::uintptr_t address, std::size_t alignment) noexcept {
    const std::uintptr_t mask = static_cast<std::uintptr_t>(alignment) - 1;
    return (address + mask) & ~mask;
}

}  // namespace genesis::core::memory
