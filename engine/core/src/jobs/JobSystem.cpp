#include "genesis/core/jobs/JobSystem.hpp"

#include <algorithm>

#include "genesis/core/Log.hpp"

namespace genesis::core::jobs {

JobSystem::JobSystem(std::size_t threadCount) {
    if (threadCount == 0) {
        const unsigned hw = std::thread::hardware_concurrency();
        threadCount = hw > 1 ? static_cast<std::size_t>(hw - 1) : 1;
    }

    m_workers.reserve(threadCount);
    for (std::size_t i = 0; i < threadCount; ++i) {
        m_workers.emplace_back([this] { WorkerLoop(); });
    }

    GENESIS_LOG_INFO("JobSystem", "started");
}

JobSystem::~JobSystem() {
    {
        std::scoped_lock lock(m_queueMutex);
        m_stopping = true;
    }
    m_queueCv.notify_all();
    for (auto& worker : m_workers) {
        if (worker.joinable()) worker.join();
    }
}

void JobSystem::Kick(JobFunction job, JobCounter* counter) {
    if (counter != nullptr) counter->Add(1);
    {
        std::scoped_lock lock(m_queueMutex);
        m_queue.push_back(QueuedJob{std::move(job), counter});
    }
    m_queueCv.notify_one();
}

void JobSystem::KickAndWait(JobFunction job) {
    JobCounter counter;
    Kick(std::move(job), &counter);
    counter.Wait();
}

void JobSystem::ParallelFor(std::size_t itemCount, std::size_t granularity,
                             const std::function<void(std::size_t, std::size_t)>& body) {
    if (itemCount == 0) return;
    if (granularity == 0) granularity = 1;

    JobCounter counter;
    for (std::size_t start = 0; start < itemCount; start += granularity) {
        const std::size_t end = std::min(start + granularity, itemCount);
        Kick([&body, start, end] { body(start, end); }, &counter);
    }
    counter.Wait();
}

void JobSystem::WorkerLoop() {
    for (;;) {
        QueuedJob job;
        {
            std::unique_lock lock(m_queueMutex);
            m_queueCv.wait(lock, [this] { return m_stopping || !m_queue.empty(); });

            if (m_stopping && m_queue.empty()) return;

            job = std::move(m_queue.back());
            m_queue.pop_back();
        }

        job.fn();
        if (job.counter != nullptr) job.counter->Decrement();
    }
}

}  // namespace genesis::core::jobs
