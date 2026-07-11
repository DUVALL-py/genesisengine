#include <catch2/catch_test_macros.hpp>

#include "genesis/core/memory/LinearAllocator.hpp"
#include "genesis/core/memory/PoolAllocator.hpp"

using namespace genesis::core::memory;

TEST_CASE("LinearAllocator allocates sequentially and respects capacity", "[memory][linear]") {
    LinearAllocator alloc(256, "TestLinear");

    void* a = alloc.Allocate(64, 16);
    void* b = alloc.Allocate(64, 16);

    REQUIRE(a != nullptr);
    REQUIRE(b != nullptr);
    REQUIRE(a != b);

    auto stats = alloc.Stats();
    REQUIRE(stats.allocationCount == 2);
    REQUIRE(stats.bytesAllocated <= stats.bytesReserved);
}

TEST_CASE("LinearAllocator returns nullptr when arena is exhausted", "[memory][linear]") {
    LinearAllocator alloc(64, "TestLinearSmall");

    void* a = alloc.Allocate(48, 8);
    REQUIRE(a != nullptr);

    void* b = alloc.Allocate(48, 8);  // não cabe mais
    REQUIRE(b == nullptr);
}

TEST_CASE("LinearAllocator Reset reclaims all memory", "[memory][linear]") {
    LinearAllocator alloc(128, "TestLinearReset");

    void* a = alloc.Allocate(64, 8);
    REQUIRE(a != nullptr);
    REQUIRE(alloc.Stats().allocationCount == 1);

    alloc.Reset();
    REQUIRE(alloc.Stats().allocationCount == 0);
    REQUIRE(alloc.Stats().bytesAllocated == 0);

    void* b = alloc.Allocate(64, 8);
    REQUIRE(b == a);  // deve reutilizar o início da arena
}

TEST_CASE("PoolAllocator allocates and frees fixed-size blocks in O(1)", "[memory][pool]") {
    PoolAllocator pool(32, 4, 16, "TestPool");

    void* p1 = pool.Allocate(32, 16);
    void* p2 = pool.Allocate(32, 16);
    void* p3 = pool.Allocate(32, 16);
    void* p4 = pool.Allocate(32, 16);

    REQUIRE(p1 != nullptr);
    REQUIRE(p2 != nullptr);
    REQUIRE(p3 != nullptr);
    REQUIRE(p4 != nullptr);

    // Pool tem apenas 4 blocos: a 5ª alocação deve falhar.
    void* p5 = pool.Allocate(32, 16);
    REQUIRE(p5 == nullptr);

    REQUIRE(pool.Stats().allocationCount == 4);

    pool.Deallocate(p2);
    REQUIRE(pool.Stats().allocationCount == 3);

    void* p6 = pool.Allocate(32, 16);
    REQUIRE(p6 == p2);  // deve reutilizar o bloco liberado
}

TEST_CASE("PoolAllocator rejects allocations larger than block size", "[memory][pool]") {
    PoolAllocator pool(16, 2, 16, "TestPoolReject");
    void* p = pool.Allocate(64, 16);
    REQUIRE(p == nullptr);
}
