#pragma once
// =============================================================================
// Genesis::Core::Jobs::JobSystem
//
// Thread pool com fila compartilhada (mutex + condition_variable). Não é
// work-stealing/fiber-based ainda — isso é uma otimização futura que não
// muda a API pública (Kick/KickAndWait/ParallelFor continuam iguais).
// Todo sistema de runtime (Physics, Animation, Renderer, ...) agenda
// trabalho paralelo através desta API, nunca criando std::thread diretamente.
// =============================================================================

#include <cstddef>
#include <functional>
#include <thread>
#include <vector>

#include "genesis/core/jobs/JobCounter.hpp"

namespace genesis::core::jobs {

using JobFunction = std::function<void()>;

class JobSystem {
public:
    // threadCount == 0 significa "auto": hardware_concurrency() - 1, mínimo 1.
    explicit JobSystem(std::size_t threadCount = 0);
    ~JobSystem();

    JobSystem(const JobSystem&) = delete;
    JobSystem& operator=(const JobSystem&) = delete;

    // Agenda um job. Se counter != nullptr, counter->Add(1) é chamado antes
    // de enfileirar e counter->Decrement() é chamado ao concluir.
    void Kick(JobFunction job, JobCounter* counter = nullptr);

    // Agenda e bloqueia até o job (e tudo que compartilhar o counter) terminar.
    void KickAndWait(JobFunction job);

    // Divide [0, itemCount) em blocos de tamanho `granularity` e executa
    // `body(start, end)` para cada bloco em paralelo, esperando todos terminarem.
    void ParallelFor(std::size_t itemCount, std::size_t granularity,
                      const std::function<void(std::size_t start, std::size_t end)>& body);

    [[nodiscard]] std::size_t WorkerCount() const noexcept { return m_workers.size(); }

private:
    void WorkerLoop();

    struct QueuedJob {
        JobFunction fn;
        JobCounter* counter;
    };

    std::vector<std::thread> m_workers;
    std::vector<QueuedJob> m_queue;
    std::mutex m_queueMutex;
    std::condition_variable m_queueCv;
    bool m_stopping = false;
};

}  // namespace genesis::core::jobs
