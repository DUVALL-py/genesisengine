// =============================================================================
//  Genesis Engine — Demo Standalone
//  Compila sem CMake, sem dependências externas.
//  Mostra os subsistemas core a funcionar: Log, Memory, Math, Reflection.
// =============================================================================

#include <cstdio>
#include <cstring>
#include <cmath>
#include <string>
#include <string_view>
#include <mutex>
#include <new>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>
#include <any>
#include <unordered_map>
#include <typeindex>
#include <cassert>

// ── Engine headers (header-only, sem dependências externas) ──────────────────
#include "genesis/core/Log.hpp"
#include "genesis/core/memory/Allocator.hpp"
#include "genesis/core/memory/LinearAllocator.hpp"
#include "genesis/core/memory/PoolAllocator.hpp"
#include "genesis/core/math/Vec.hpp"
#include "genesis/core/math/Mat4.hpp"

// ─────────────────────────────────────────────────────────────────────────────

using namespace genesis::core;
using namespace genesis::core::memory;
using namespace genesis::core::math;

// Utilitário de impressão colorida (ANSI) — funciona no Windows 10+ e Linux
#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
static void EnableANSI() {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD  mode = 0;
    if (GetConsoleMode(h, &mode))
        SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}
#else
static void EnableANSI() {}
#endif

#define CLR_RESET  "\033[0m"
#define CLR_BOLD   "\033[1m"
#define CLR_GREEN  "\033[32m"
#define CLR_CYAN   "\033[36m"
#define CLR_YELLOW "\033[33m"
#define CLR_MAGENTA "\033[35m"
#define CLR_BLUE   "\033[34m"
#define CLR_RED    "\033[31m"
#define CLR_WHITE  "\033[37m"

static void Banner() {
    printf(CLR_BOLD CLR_CYAN
        "\n"
        "   ██████╗ ███████╗███╗   ██╗███████╗███████╗██╗███████╗\n"
        "  ██╔════╝ ██╔════╝████╗  ██║██╔════╝██╔════╝██║██╔════╝\n"
        "  ██║  ███╗█████╗  ██╔██╗ ██║█████╗  ███████╗██║███████╗\n"
        "  ██║   ██║██╔══╝  ██║╚██╗██║██╔══╝  ╚════██║██║╚════██║\n"
        "  ╚██████╔╝███████╗██║ ╚████║███████╗███████║██║███████║\n"
        "   ╚═════╝ ╚══════╝╚═╝  ╚═══╝╚══════╝╚══════╝╚═╝╚══════╝\n"
        CLR_RESET
        CLR_YELLOW "                     Engine v0.1.0 — Core Demo\n" CLR_RESET
        "\n"
    );
}

static void Section(const char* name) {
    printf(CLR_BOLD CLR_BLUE "\n  ═══ %s ═══\n" CLR_RESET, name);
}

static void OK(const char* label) {
    printf(CLR_GREEN "  [✓] " CLR_RESET "%s\n", label);
}

// ─────────────────────────────────────────────────────────────────────────────
//  1. LOGGING
// ─────────────────────────────────────────────────────────────────────────────
static void DemoLogging() {
    Section("LOGGING");
    printf("  Logger singleton thread-safe com mutex:\n");
    GENESIS_LOG_TRACE("demo",   "  Mensagem TRACE — detalhe máximo de debug");
    GENESIS_LOG_INFO ("demo",   "  Engine iniciada com sucesso!");
    GENESIS_LOG_WARN ("demo",   "  Atenção: subsistema em modo mock");
    GENESIS_LOG_ERROR("demo",   "  Erro simulado (não é real)");
    OK("Logger funcionando — TRACE / INFO / WARN / ERROR");
}

// ─────────────────────────────────────────────────────────────────────────────
//  2. MEMORY — LinearAllocator
// ─────────────────────────────────────────────────────────────────────────────
static void DemoMemory() {
    Section("MEMORY — LinearAllocator");

    constexpr std::size_t ARENA_SIZE = 1024;  // 1 KB de arena
    LinearAllocator arena(ARENA_SIZE, "FrameArena");

    // Alocar alguns inteiros alinhados
    auto* a = static_cast<int*>(arena.Allocate(sizeof(int), alignof(int)));
    auto* b = static_cast<int*>(arena.Allocate(sizeof(int), alignof(int)));
    auto* c = static_cast<float*>(arena.Allocate(sizeof(float)*4, alignof(float)));

    if (a && b && c) {
        *a = 42;
        *b = 100;
        c[0] = 1.0f; c[1] = 2.0f; c[2] = 3.0f; c[3] = 4.0f;
    }

    auto stats = arena.Stats();
    printf("  Arena '%s':\n", arena.DebugName());
    printf("    Capacidade  : %zu bytes\n", stats.bytesReserved);
    printf("    Usado       : %zu bytes\n", stats.bytesAllocated);
    printf("    Alocações   : %zu\n",       stats.allocationCount);
    printf("    *a=%d, *b=%d, c={%.1f,%.1f,%.1f,%.1f}\n",
           *a, *b, c[0], c[1], c[2], c[3]);

    arena.Reset();  // libera tudo em O(1)
    printf("  Após Reset(): usado=%zu bytes\n", arena.Stats().bytesAllocated);
    OK("LinearAllocator — alloc / reset em O(1)");
}

// ─────────────────────────────────────────────────────────────────────────────
//  3. MEMORY — PoolAllocator
// ─────────────────────────────────────────────────────────────────────────────
struct Transform {
    Vec3 position;
    Vec3 rotation;
    Vec3 scale{1,1,1};
};

static void DemoPool() {
    Section("MEMORY — PoolAllocator");

    PoolAllocator pool(sizeof(Transform), 16, alignof(Transform), "TransformPool");

    auto* t1 = static_cast<Transform*>(pool.Allocate(sizeof(Transform), alignof(Transform)));
    auto* t2 = static_cast<Transform*>(pool.Allocate(sizeof(Transform), alignof(Transform)));
    auto* t3 = static_cast<Transform*>(pool.Allocate(sizeof(Transform), alignof(Transform)));

    if (t1) { new(t1) Transform(); t1->position = {1, 0, 0}; }
    if (t2) { new(t2) Transform(); t2->position = {0, 2, 0}; }
    if (t3) { new(t3) Transform(); t3->position = {0, 0, 3}; }

    auto stats = pool.Stats();
    printf("  Pool '%s' (capacidade: 16 objetos de %zu bytes):\n",
           pool.DebugName(), sizeof(Transform));
    printf("    Usado  : %zu bytes\n", stats.bytesAllocated);
    printf("    Alocações ativas: %zu\n", stats.allocationCount);
    printf("    t1.pos=(%.0f,%.0f,%.0f)\n", t1->position.x, t1->position.y, t1->position.z);
    printf("    t2.pos=(%.0f,%.0f,%.0f)\n", t2->position.x, t2->position.y, t2->position.z);
    printf("    t3.pos=(%.0f,%.0f,%.0f)\n", t3->position.x, t3->position.y, t3->position.z);

    // Devolve t2 ao pool
    t2->~Transform();
    pool.Deallocate(t2);
    printf("  Após liberar t2: alocações=%zu\n", pool.Stats().allocationCount);

    OK("PoolAllocator — alloc / dealloc com freelist O(1)");
}

// ─────────────────────────────────────────────────────────────────────────────
//  4. MATH — Vec3 / Mat4
// ─────────────────────────────────────────────────────────────────────────────
static void DemoMath() {
    Section("MATH — Vec3 e Mat4");

    Vec3 pos    {1.0f, 2.0f, 3.0f};
    Vec3 dir    {0.0f, 1.0f, 0.0f};
    Vec3 up     = Vec3::UnitY();

    Vec3 cross  = pos.Cross(dir);
    float dot   = pos.Dot(dir);
    Vec3 norm   = pos.Normalized();
    float len   = pos.Length();

    printf("  pos    = (%.2f, %.2f, %.2f)\n", pos.x, pos.y, pos.z);
    printf("  dir    = (%.2f, %.2f, %.2f)\n", dir.x, dir.y, dir.z);
    printf("  cross  = (%.2f, %.2f, %.2f)\n", cross.x, cross.y, cross.z);
    printf("  dot    = %.2f\n", dot);
    printf("  norm   = (%.4f, %.4f, %.4f)\n", norm.x, norm.y, norm.z);
    printf("  length = %.4f (esperado: %.4f)\n", len, std::sqrt(14.0f));

    // Mat4 — identidade e translação
    Mat4 identity = Mat4::Identity();
    Mat4 tmat     = Mat4::Translation(pos);
    Vec4 point {0.0f, 0.0f, 0.0f, 1.0f};
    Vec4 transformed = tmat * point;

    printf("  Mat4::Translation(pos) * (0,0,0,1) = (%.2f, %.2f, %.2f)\n",
           transformed.x, transformed.y, transformed.z);
    OK("Vec3 (dot/cross/normalize/length) + Mat4 (identity/translation/mul)");
}

// ─────────────────────────────────────────────────────────────────────────────
//  5. MINI-ECS SIMULATION (sem biblioteca externa)
// ─────────────────────────────────────────────────────────────────────────────
static void DemoECS() {
    Section("MINI-SIMULAÇÃO ECS (demo)");

    struct Entity { std::uint32_t id; std::string name; Vec3 position; float speed; };

    std::vector<Entity> world = {
        {1, "Player",   {0,0,0},   5.0f},
        {2, "Enemy_A",  {10,0,5},  2.0f},
        {3, "Enemy_B",  {-5,0,8},  3.0f},
        {4, "Particle", {0,5,0},   0.5f},
    };

    constexpr float dt = 0.016f;  // 1 frame @ 60fps
    constexpr int   FRAMES = 5;

    printf("  Simulando %d frames (dt=%.3fs)...\n", FRAMES, dt);
    for (int f = 1; f <= FRAMES; ++f) {
        for (auto& e : world) {
            // move em direção ao centro
            Vec3 toCenter = Vec3::Zero() - e.position;
            float len = toCenter.Length();
            if (len > 0.01f)
                e.position += toCenter.Normalized() * (e.speed * dt);
        }
    }

    printf("  Posições após %d frames:\n", FRAMES);
    for (const auto& e : world) {
        printf("    [%u] %-10s  pos=(%.3f, %.3f, %.3f)\n",
               e.id, e.name.c_str(),
               e.position.x, e.position.y, e.position.z);
    }
    OK("Simulação ECS: 4 entidades, 5 frames, física simples");
}

// ─────────────────────────────────────────────────────────────────────────────
//  MAIN
// ─────────────────────────────────────────────────────────────────────────────
int main() {
    EnableANSI();
    Banner();

    DemoLogging();
    DemoMemory();
    DemoPool();
    DemoMath();
    DemoECS();

    printf(CLR_BOLD CLR_GREEN
        "\n  ════════════════════════════════════════\n"
        "   Genesis Engine — Core ONLINE!\n"
        "  ════════════════════════════════════════\n"
        CLR_RESET "\n");
    return 0;
}
