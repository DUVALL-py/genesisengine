#pragma once
// =============================================================================
// Genesis::Core::Jobs::JobCounter
//
// Primitiva de sincronização do Job System. Cada job submetido com um
// counter incrementa-o antes de entrar na fila e decrementa-o ao terminar.
// Wait() bloqueia a thread chamadora até o counter chegar a zero — é assim
// que se expressam dependências ("rode B só depois que todo o grupo A
// terminar") sem precisar de um grafo de dependências explícito no MVP.
// =============================================================================

#include <atomic>
#include <condition_variable>
#include <mutex>

namespace genesis::core::jobs {

class JobCounter {
public:
    JobCounter() = default;
    JobCounter(const JobCounter&) = delete;
    JobCounter& operator=(const JobCounter&) = delete;

    void Add(int delta = 1) noexcept {
        m_count.fetch_add(delta, std::memory_order_acq_rel);
    }

    // Decrementa e acorda quem estiver em Wait() se chegar a zero.
    void Decrement() {
        const int previous = m_count.fetch_sub(1, std::memory_order_acq_rel);
        if (previous == 1) {
            std::scoped_lock lock(m_mutex);
            m_cv.notify_all();
        }
    }

    void Wait() {
        if (m_count.load(std::memory_order_acquire) == 0) return;
        std::unique_lock lock(m_mutex);
        m_cv.wait(lock, [this] { return m_count.load(std::memory_order_acquire) == 0; });
    }

    [[nodiscard]] int Value() const noexcept { return m_count.load(std::memory_order_acquire); }

private:
    std::atomic<int> m_count{0};
    std::mutex m_mutex;
    std::condition_variable m_cv;
};

}  // namespace genesis::core::jobs
