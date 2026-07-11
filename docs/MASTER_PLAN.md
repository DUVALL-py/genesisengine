# Genesis Engine — Plano Mestre

## 0. Nota de escopo (leitura obrigatória antes do roadmap)

Uma engine no nível arquitetural de Unreal/Unity/Godot representa dezenas de
milhares de horas de engenharia especializada (renderização, física,
compiladores de shader, ferramentas de editor, etc.), tipicamente feita por
equipes de centenas de pessoas ao longo de anos. Este plano assume isso como
premissa e não como obstáculo: vamos construir uma **arquitetura real,
compilável e testável desde o primeiro módulo**, com qualidade de código de
produção, crescendo módulo a módulo. Cada módulo entregue é funcional e
verificável — não é stub decorativo. O ritmo será: **profundidade real em
poucos módulos de cada vez**, em vez de esqueletos vazios em todos os 25
módulos simultaneamente. Isso é o que realmente permite "nunca simplificar
por conveniência" e "sempre manter o projeto compilando".

---

## 1. Arquitetura Geral

### 1.1 Camadas

```
┌─────────────────────────────────────────────────────────┐
│  Ferramentas: Editor (cliente) | CLI | Agentes de IA     │
├─────────────────────────────────────────────────────────┤
│  API Layer (C API estável + bindings Python/C#/Lua)      │
├─────────────────────────────────────────────────────────┤
│  Runtime Modules                                         │
│  Scene | ECS | Physics | Animation | Audio | Networking   │
│  Renderer | Materials | Particles | Navigation | UI       │
│  Asset Pipeline | Procedural Gen | AI Runtime | Terrain   │
├─────────────────────────────────────────────────────────┤
│  Core (Memory, Jobs, Reflection, Serialization, Math,     │
│         Logging, Platform, Containers)                    │
├─────────────────────────────────────────────────────────┤
│  RHI — Render Hardware Interface                          │
│  Vulkan | DirectX12 | Metal | WebGPU | OpenGL(compat)      │
├─────────────────────────────────────────────────────────┤
│  Platform Abstraction (Windows/Linux/macOS, Threads, FS)  │
└─────────────────────────────────────────────────────────┘
```

Regra estrutural inegociável: **nada acima de "Core" pode ser importado por
"Core"**; o Editor só pode falar com os módulos via API pública (nunca acessa
internals); o RHI é a única camada que sabe qual backend gráfico está ativo.

### 1.2 Estrutura de diretórios

```
genesis-engine/
├── CMakeLists.txt                # build raiz, orquestra subprojetos
├── cmake/                        # toolchains, módulos CMake, sanitizers
├── docs/                         # MASTER_PLAN.md, docs técnicas por módulo, UML
├── engine/
│   ├── core/                     # Memory, Jobs, Reflection, Serialization, Math
│   ├── rhi/                      # abstração Vulkan/DX12/Metal/WebGPU/GL
│   ├── ecs/
│   ├── physics/
│   ├── animation/
│   ├── audio/
│   ├── renderer/
│   ├── materials/
│   ├── shaders/
│   ├── terrain/
│   ├── procgen/
│   ├── navigation/
│   ├── particles/
│   ├── networking/
│   ├── ui/
│   ├── asset_pipeline/
│   ├── scene/
│   ├── prefab/
│   ├── input/
│   ├── ai_runtime/
│   ├── scripting/                # bindings Lua/Python/C#
│   └── plugin_system/
├── editor/                       # cliente gráfico (dockspace, viewport, etc.)
├── tools/                        # build tools, geração de UML, asset importers
├── tests/                        # testes unitários por módulo (mesma árvore)
├── benchmarks/
├── examples/
└── third_party/                  # dependências vendored/FetchContent
```

Cada módulo é uma **biblioteca CMake independente** com seu próprio
`include/genesis/<modulo>/`, `src/`, `CMakeLists.txt`, testes e README técnico.
Nenhum módulo inclui headers de outro módulo por caminho relativo — sempre via
target CMake (`target_link_libraries`) e include público.

### 1.3 Convenções de API AI-first

Toda funcionalidade pública é exposta em três camadas simultâneas:
1. **C++ API** (uso nativo por outros módulos/editor)
2. **C ABI estável** (`extern "C"`) — base para bindings
3. **Bindings de linguagem script** (Python/Lua/C#) gerados a partir da C ABI

Isso garante que um agente de IA controlando a engine via Python tem acesso
exatamente às mesmas operações que o editor gráfico usa internamente —
nenhuma feature "só existe no editor".

---

## 2. Diagrama de Dependências entre Módulos

```
                         Platform Abstraction
                                 │
                               Core
                     (Memory/Jobs/Reflection/
                      Serialization/Math/Log)
                                 │
              ┌──────────────────┼──────────────────┐
              │                  │                   │
             RHI               ECS              Scripting
              │                  │                   │
        ┌─────┴─────┐            │                    │
     Renderer    Materials       │                    │
        │            │           │                    │
    Particles    Shaders         │                    │
        │                        │                    │
        └───────────┬────────────┴──────────┬─────────┘
                     │                       │
                Scene System           Plugin System
                     │
     ┌───────┬───────┼───────┬────────┬─────────┬──────────┐
  Physics  Animation Audio Navigation Terrain  ProcGen   Networking
     │         │              │
     └────┬────┴──────────────┘
          │
      AI Runtime (Behavior Trees, GOAP, Utility AI)
          │
     Asset Pipeline ── Prefab System ── Input ── UI
          │
        Editor (cliente — consome tudo via API pública)
```

Regras derivadas do diagrama:
- `Core` e `Platform Abstraction` não dependem de nada dentro da engine.
- `RHI` depende só de `Core`. Backends gráficos concretos (Vulkan/DX12/...)
  são plugins do RHI, carregados em runtime quando possível.
- `Scene System` é o hub: qualquer módulo de simulação (Physics, Animation,
  Audio, Navigation) se registra nele via ECS, nunca diretamente entre si.
- `Editor` está no topo e é o único módulo autorizado a depender de "tudo",
  e mesmo assim só pela API pública.

---

## 3. Roadmap em Fases

### Fase MVP — "A engine liga e desenha um triângulo controlado por código"
Objetivo: provar a arquitetura de ponta a ponta com a menor superfície possível.

Módulos entregues (profundidade real, não stub):
- Platform Abstraction (Windows/Linux inicialmente; macOS depois)
- Core: Memory Manager (allocators: linear, pool, freelist), Logging,
  Assert/Error handling, tipos base, Math (vec/mat/quat), Containers básicos
- Job System (thread pool + fibers ou task graph simples)
- RHI mínimo com backend Vulkan (device, swapchain, pipeline simples)
- ECS mínimo (archetype-based ou sparse-set — a decidir na implementação)
- API C para: `CreateWindow`, `RenderFrame`, `CreateMesh`, `Shutdown`
- CLI/headless runner que um agente consegue invocar sem GUI

**Critérios de aceitação MVP**
- [ ] `cmake --build` limpo em Linux (CI mínima), zero warnings com `-Wall -Wextra`
- [ ] Testes unitários de Core (Memory, Math, Containers) passam 100%
- [ ] Um executável `genesis_headless` desenha um triângulo colorido via
      Vulkan e salva um PNG via `CaptureRGB()` sem abrir janela
- [ ] Documentação técnica gerada para os módulos entregues
- [ ] Nenhum módulo do MVP acessa código fora de sua dependência declarada
      no diagrama da seção 2

### Fase Alpha — "Cenas jogáveis simples, sem editor gráfico"
Adiciona: Scene System, Prefab System, Physics (rigid bodies + raycast),
Input, Audio básico (2D/3D), Scripting (Lua primeiro), Asset Pipeline
(import de meshes/texturas), Serialization completa, Reflection completa,
Forward+ renderer com PBR básico, Shadow Maps.

**Critérios de aceitação Alpha**
- [ ] Uma cena serializável (JSON/binário) com entidades físicas simuladas
      roda em `genesis_headless` a partir de um script Python externo, sem
      qualquer interação de mouse/teclado humana
- [ ] `PlayGame()`/`StopGame()` funcionam via API
- [ ] Cobertura de testes ≥ 70% nos módulos de Alpha
- [ ] Benchmark básico de Job System e ECS publicado em `benchmarks/`

### Fase Beta — "Editor funcional + pipeline de conteúdo completo"
Adiciona: Editor (viewport, hierarchy, inspector, asset browser), Animation
(skeletons, blend trees), Navigation (navmesh, crowd), Networking básico,
UI runtime, Terrain, Particles, Deferred Rendering, GPU culling, DX12/Metal
como backends adicionais do RHI, Asset Database semântico (busca por
descrição), captura de buffers de visão para IA (Depth/Normals/IDs).

**Critérios de aceitação Beta**
- [ ] Editor abre, carrega e salva um projeto de exemplo sem crashes
- [ ] Um agente de IA consegue montar uma cena completa (terreno + props +
      NPC com navmesh) só por chamadas de API, sem GUI
- [ ] Backends Vulkan e DX12 (ou Metal, dependendo da plataforma de CI)
      renderizam a mesma cena com paridade visual verificável
- [ ] Sistema de testes automáticos internos (agente que joga e detecta
      bugs) roda em pelo menos 1 cena de exemplo e reporta métricas

### Fase 1.0 — "Feature-complete para produção de um jogo pequeno/médio"
Adiciona: WebGPU backend, Mesh Shaders, Motion Matching, GOAP/Utility AI,
Voxel support, Procedural terrain avançado, Plugin marketplace interno,
C#/Visual Scripting, Shader Graph, Sequencer, Profiler/Debugger completos,
build pipeline multiplataforma (`BuildGame()` gera executável final).

**Critérios de aceitação 1.0**
- [ ] Jogo de exemplo completo (vertical slice) construído inteiramente
      via API por scripts, exportado com `BuildGame()` para pelo menos 2
      plataformas
- [ ] Documentação técnica + UML cobrindo 100% dos módulos públicos
- [ ] Suite de testes + benchmarks rodando em CI a cada módulo alterado

---

## 4. Ordem de Implementação Módulo-a-Módulo (dentro do MVP)

1. Platform Abstraction (mínimo: Linux)
2. Core::Memory (allocators + tracking)
3. Core::Logging + Assert
4. Core::Math
5. Core::Containers
6. Core::Job System
7. Core::Reflection (mínimo viável) — **decisão registrada**: geração via
   parsing real do AST (libclang), não registro manual por macro. Ver
   `tools/reflect/README.md`.
8. Core::Serialization (depende de Reflection)
9. RHI (interface abstrata)
10. RHI::Vulkan backend
11. ECS mínimo
12. API C pública do MVP + `genesis_headless`

A cada item concluído: build limpo + testes passando + commit antes de
avançar para o próximo. É exatamente essa disciplina que vamos seguir agora,
começando pelo item 1 e 2.

---

## 5. Convenções de Engenharia (válidas para todos os módulos)

- **Padrão**: C++20, `-Wall -Wextra -Wpedantic -Werror` em CI (relaxado
  localmente durante desenvolvimento ativo de um módulo novo).
- **Build**: CMake ≥ 3.24, `FetchContent` para dependências externas
  (ex.: Vulkan headers, GLFW, Catch2), nunca vendored manualmente quando
  evitável.
- **Testes**: Catch2, um executável de teste por módulo, integrado via
  `ctest`.
- **Naming**: `PascalCase` para tipos, `camelCase` para funções/métodos,
  `snake_case` para arquivos, namespace raiz `genesis::<modulo>`.
- **Erros**: sem exceções na hot path do runtime; `Result<T, Error>` ou
  códigos de erro explícitos; exceções toleradas apenas em ferramentas
  offline (asset pipeline, editor).
- **Documentação**: comentário Doxygen em toda API pública; diagramas UML
  gerados por script em `tools/` a partir dos headers.
