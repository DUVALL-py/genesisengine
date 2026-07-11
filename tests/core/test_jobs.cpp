#include <atomic>
#include <catch2/catch_test_macros.hpp>
#include <numeric>
#include <vector>

#include "genesis/core/jobs/JobSystem.hpp"

using namespace genesis::core::jobs;

TEST_CASE("JobSystem runs a single job via KickAndWait", "[jobs]") {
    JobSystem js(2);
    std::atomic<bool> ran{false};

    js.KickAndWait([&ran] { ran = true; });

    REQUIRE(ran.load());
}

TEST_CASE("JobSystem runs many jobs sharing a counter and Wait() blocks until all finish", "[jobs]") {
    JobSystem js(4);
    JobCounter counter;
    std::atomic<int> completed{0};

    constexpr int kJobCount = 200;
    for (int i = 0; i < kJobCount; ++i) {
        js.Kick([&completed] { completed.fetch_add(1, std::memory_order_relaxed); }, &counter);
    }
    counter.Wait();

    REQUIRE(completed.load() == kJobCount);
    REQUIRE(counter.Value() == 0);
}

TEST_CASE("JobSystem::ParallelFor covers every element exactly once", "[jobs][parallel_for]") {
    JobSystem js(4);

    constexpr std::size_t kSize = 10'000;
    std::vector<int> data(kSize, 0);

    js.ParallelFor(kSize, 256, [&data](std::size_t start, std::size_t end) {
        for (std::size_t i = start; i < end; ++i) {
            data[i] = static_cast<int>(i) * 2;
        }
    });

    for (std::size_t i = 0; i < kSize; ++i) {
        REQUIRE(data[i] == static_cast<int>(i) * 2);
    }
}

TEST_CASE("JobSystem::ParallelFor sums a large array correctly", "[jobs][parallel_for]") {
    JobSystem js(4);

    constexpr std::size_t kSize = 1'000;
    std::vector<int> data(kSize, 1);
    std::vector<long long> partialSums(kSize / 100, 0);

    js.ParallelFor(kSize, 100, [&](std::size_t start, std::size_t end) {
        long long sum = 0;
        for (std::size_t i = start; i < end; ++i) sum += data[i];
        partialSums[start / 100] = sum;
    });

    const long long total = std::accumulate(partialSums.begin(), partialSums.end(), 0LL);
    REQUIRE(total == static_cast<long long>(kSize));
}

TEST_CASE("JobSystem reports the configured worker count", "[jobs]") {
    JobSystem js(3);
    REQUIRE(js.WorkerCount() == 3);
}
