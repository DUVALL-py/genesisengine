#pragma once
// =============================================================================
// Genesis::Core::Log
//
// Logging mínimo, thread-safe, sem alocações no caminho quente de formatação
// de string simples. Pensado para ser substituído/estendido por sinks
// (arquivo, console, rede) no módulo posterior, mas já funcional agora.
// =============================================================================

#include <cstdio>
#include <mutex>
#include <string_view>

namespace genesis::core {

enum class LogLevel {
    Trace,
    Info,
    Warning,
    Error,
    Fatal,
};

[[nodiscard]] inline const char* ToString(LogLevel level) noexcept {
    switch (level) {
        case LogLevel::Trace:   return "TRACE";
        case LogLevel::Info:    return "INFO";
        case LogLevel::Warning: return "WARN";
        case LogLevel::Error:   return "ERROR";
        case LogLevel::Fatal:   return "FATAL";
    }
    return "?????";
}

class Logger {
public:
    static Logger& Instance() {
        static Logger instance;
        return instance;
    }

    void Log(LogLevel level, std::string_view category, std::string_view message) {
        if (level < m_minLevel) return;
        std::scoped_lock lock(m_mutex);
        std::fprintf(level >= LogLevel::Error ? stderr : stdout,
                     "[%s] [%s] %.*s\n",
                     ToString(level),
                     category.empty() ? "genesis" : category.data(),
                     static_cast<int>(message.size()), message.data());
    }

    void SetMinLevel(LogLevel level) noexcept { m_minLevel = level; }
    [[nodiscard]] LogLevel MinLevel() const noexcept { return m_minLevel; }

private:
    Logger() = default;
    std::mutex m_mutex;
    LogLevel m_minLevel = LogLevel::Trace;
};

}  // namespace genesis::core

#define GENESIS_LOG_TRACE(category, msg) \
    ::genesis::core::Logger::Instance().Log(::genesis::core::LogLevel::Trace, category, msg)
#define GENESIS_LOG_INFO(category, msg) \
    ::genesis::core::Logger::Instance().Log(::genesis::core::LogLevel::Info, category, msg)
#define GENESIS_LOG_WARN(category, msg) \
    ::genesis::core::Logger::Instance().Log(::genesis::core::LogLevel::Warning, category, msg)
#define GENESIS_LOG_ERROR(category, msg) \
    ::genesis::core::Logger::Instance().Log(::genesis::core::LogLevel::Error, category, msg)
#define GENESIS_LOG_FATAL(category, msg) \
    ::genesis::core::Logger::Instance().Log(::genesis::core::LogLevel::Fatal, category, msg)
