# Genesis Engine — Guia de Continuação

**Data:** Julho 2026  
**Estado atual:** Core v0.1.0 funcional (sem gráficos)  
**Repositório:** https://github.com/DUVALL-py/genesisengine  
**Plataforma alvo:** Linux (Ubuntu 24), GCC 13+, C++20  

---

## COMO COMPILAR E TESTAR (agora mesmo)

### Pré-requisitos (Linux)
```bash
sudo apt update
sudo apt install -y build-essential cmake ninja-build git python3 libclang-dev
```

### Build completo com testes
```bash
git clone https://github.com/DUVALL-py/genesisengine.git
cd genesisengine
mkdir build && cd build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Debug -DGENESIS_WARNINGS_AS_ERRORS=ON
ninja
ctest --output-on-failure
```

### Demo standalone (sem CMake, sem dependências)
```bash
cd genesisengine
g++ -std=c++20 -O2 -Iengine/core/include demo/main.cpp -o demo/genesis_demo
./demo/genesis_demo
```

---

## O QUE JÁ ESTÁ FEITO ✅

### engine/core/
| Ficheiro | Descrição |
|---|---|
| `Log.hpp` | Logger thread-safe com mutex, níveis TRACE/INFO/WARN/ERROR/FATAL |
| `Assert.hpp` | Macros GENESIS_ASSERT com mensagem e ficheiro/linha |
| `Platform.hpp` | Deteção de plataforma (Linux/Windows/Mac) e arquitetura |
| `Version.hpp` | Versão semântica da engine |
| `memory/Allocator.hpp` | Interface base IAllocator + stats |
| `memory/LinearAllocator.hpp` | Arena alloc em O(1), reset em O(1), zero fragmentação |
| `memory/PoolAllocator.hpp` | Pool de objetos fixed-size, freelist O(1) |
| `math/Vec.hpp` | Vec3 e Vec4: dot, cross, normalize, length, constexpr |
| `math/Mat4.hpp` | Mat4 4x4: identidade, translação, rotação, projeção perspetiva |
| `reflection/TypeInfo.hpp` | Metadados de tipo: nome, ID, propriedades |
| `reflection/TypeRegistry.hpp` | Registry global de tipos com lookup por nome/hash |
| `reflection/ReflectMacros.hpp` | Macros GENESIS_REFLECT_BEGIN/PROPERTY/END |
| `jobs/JobSystem.hpp` | Thread pool com work-stealing, submit, wait |
| `jobs/JobCounter.hpp` | Contador atómico para sincronização de jobs |
| `serialization/Serializer.hpp` | Interfaces ISerializer / IDeserializer |
| `serialization/JsonSerializer.hpp` | Serialização JSON com nlohmann/json |
| `serialization/ReflectionSerializer.hpp` | Auto-serialização via reflection |

### tests/core/
- `test_memory.cpp` — testes LinearAllocator e PoolAllocator
- `test_math.cpp` — testes Vec3, Vec4, Mat4
- `test_reflection.cpp` — testes TypeRegistry e macros
- `test_jobs.cpp` — testes JobSystem thread-safety
- `test_serialization.cpp` — testes JSON round-trip

### demo/
- `main.cpp` — demo standalone compilável sem CMake

---

## O QUE FALTA — POR ORDEM DE PRIORIDADE

---

### PRIORIDADE 1 — JANELA E INPUT
**Módulo:** `engine/platform/`  
**Dependências:** SDL3 ou GLFW 3.4  

#### Ficheiros a criar:
```
engine/platform/
  include/genesis/platform/
    Window.hpp          ← Interface IWindow (create, poll, swap, close)
    WindowSDL.hpp       ← Implementação SDL3
    Input.hpp           ← Estado de teclado/rato/gamepad
    InputSDL.hpp        ← Implementação SDL3
  src/
    WindowSDL.cpp
    InputSDL.cpp
  CMakeLists.txt
```

#### O que implementar:
- Criar janela com título, resolução, modo janela/fullscreen
- Loop principal: `pollEvents()` → `update()` → `render()` → `swapBuffers()`
- Callbacks: `onResize`, `onClose`, `onKeyDown/Up`, `onMouseMove/Click`
- Input snapshot por frame (evitar input durante update)

#### Exemplo de uso pretendido:
```cpp
genesis::platform::Window window("Genesis Engine", 1280, 720);
while (window.IsOpen()) {
    window.PollEvents();
    if (Input::IsKeyDown(Key::Escape)) window.Close();
    window.SwapBuffers();
}
```

---

### PRIORIDADE 2 — RENDERER BÁSICO (OpenGL)
**Módulo:** `engine/renderer/`  
**Dependências:** OpenGL 4.5, GLAD (loader), GLM (math já temos)  

#### Ficheiros a criar:
```
engine/renderer/
  include/genesis/renderer/
    RenderContext.hpp   ← Interface: init, shutdown, beginFrame, endFrame
    Shader.hpp          ← Compilar GLSL, uniforms
    Buffer.hpp          ← VBO, IBO, VAO
    Texture.hpp         ← Upload de imagem para GPU
    Mesh.hpp            ← Vértices + índices + draw call
    Camera.hpp          ← View/Proj matrix, frustum
    RenderCommand.hpp   ← Comandos: clear, drawIndexed, setViewport
  src/
    opengl/
      OpenGLContext.cpp
      OpenGLShader.cpp
      OpenGLBuffer.cpp
      OpenGLTexture.cpp
  CMakeLists.txt
  shaders/
    basic.vert          ← Vertex shader mínimo
    basic.frag          ← Fragment shader mínimo
```

#### O que implementar (por fases):
1. **Fase 2a** — Triângulo na ecrã (hello world gráfico)
2. **Fase 2b** — Quad com textura
3. **Fase 2c** — Malha 3D com câmara orbital
4. **Fase 2d** — Múltiplos draw calls + batching básico

#### Exemplo de uso pretendido:
```cpp
Shader shader("shaders/basic.vert", "shaders/basic.frag");
Mesh cube = Mesh::CreateCube();
Camera cam({0,0,5}, {0,0,0});

while (window.IsOpen()) {
    RenderCommand::Clear({0.1f, 0.1f, 0.1f, 1.0f});
    shader.Bind();
    shader.SetMat4("uMVP", cam.ViewProjection() * transform);
    cube.Draw();
    window.SwapBuffers();
}
```

---

### PRIORIDADE 3 — ECS (Entity Component System)
**Módulo:** `engine/ecs/`  
**Dependências:** Nenhuma (header-only)  

#### Ficheiros a criar:
```
engine/ecs/
  include/genesis/ecs/
    Entity.hpp          ← ID de entidade (uint32 + geração)
    Registry.hpp        ← Registo central de entidades e componentes
    ComponentPool.hpp   ← Storage de componentes (SoA, cache-friendly)
    System.hpp          ← Interface ISystem: update(dt)
    View.hpp            ← Iteração de entidades com componentes específicos
    CommonComponents.hpp ← Transform, Tag, Hierarchy, Active
  src/
    Registry.cpp
  CMakeLists.txt
```

#### O que implementar:
- `Entity` = ID único (geração para detetar entidades mortas)
- `Registry::Create()` → Entity, `Registry::Destroy(entity)`
- `Registry::Add<T>(entity, args...)` → T&
- `Registry::Get<T>(entity)` → T&
- `Registry::View<T, U...>()` → iterador de entidades com componentes T e U
- Sistemas registados com prioridade e execução ordenada

#### Componentes base a criar:
```cpp
struct Transform { Vec3 position; Quat rotation; Vec3 scale; };
struct Tag       { std::string name; };
struct Hierarchy { Entity parent; std::vector<Entity> children; };
struct Active    { bool isActive = true; };
struct Camera    { float fov; float near; float far; };
struct MeshRenderer { Handle<Mesh> mesh; Handle<Material> material; };
struct Light     { Vec3 color; float intensity; LightType type; };
```

---

### PRIORIDADE 4 — ASSET MANAGER
**Módulo:** `engine/assets/`  
**Dependências:** stb_image (imagens), tinyobjloader (OBJ), nlohmann/json (já temos)  

#### Ficheiros a criar:
```
engine/assets/
  include/genesis/assets/
    Handle.hpp          ← Handle<T> tipado: ID + geração
    AssetManager.hpp    ← Load, unload, cache, reference counting
    AssetLoader.hpp     ← Interface IAssetLoader<T>
    ImageLoader.hpp     ← PNG/JPG/BMP via stb_image
    MeshLoader.hpp      ← OBJ via tinyobjloader
    ShaderLoader.hpp    ← Ler GLSL de disco
  src/
    AssetManager.cpp
    ImageLoader.cpp
    MeshLoader.cpp
  CMakeLists.txt
```

#### O que implementar:
- `AssetManager::Load<Texture>("assets/player.png")` → Handle<Texture>
- Cache automático: ficheiro já carregado não é recarregado
- Reference counting: asset descarregado quando Handle sai de scope
- Hot-reload em modo debug: detetar mudanças no disco e recarregar

---

### PRIORIDADE 5 — AUDIO BÁSICO
**Módulo:** `engine/audio/`  
**Dependências:** miniaudio (single-header, sem instalação)  

#### Ficheiros a criar:
```
engine/audio/
  include/genesis/audio/
    AudioEngine.hpp     ← Init, shutdown
    Sound.hpp           ← Handle para som carregado
    AudioSource.hpp     ← Componente ECS: posição, volume, pitch, loop
    AudioListener.hpp   ← Componente ECS: posição do ouvidor (câmara)
  src/
    AudioEngine.cpp
  CMakeLists.txt
```

---

### PRIORIDADE 6 — FÍSICA BÁSICA
**Módulo:** `engine/physics/`  
**Dependências:** Jolt Physics (open-source, C++17)  

#### O que implementar:
- `RigidBody` — corpo rígido dinâmico/estático/cinemático
- `Collider` — formas: box, sphere, capsule, mesh
- `PhysicsWorld` — step de simulação, gravity
- Queries: raycast, overlap sphere, shape cast

---

### PRIORIDADE 7 — SISTEMA DE CENAS
**Módulo:** `engine/scene/`  

#### Ficheiros a criar:
```
engine/scene/
  include/genesis/scene/
    Scene.hpp           ← Contém Registry ECS + lista de sistemas
    SceneManager.hpp    ← Load, unload, transição entre cenas
    SceneSerializer.hpp ← Guardar/carregar cena em JSON
  src/
    Scene.cpp
    SceneManager.cpp
    SceneSerializer.cpp
```

---

### PRIORIDADE 8 — SCRIPTING (OPCIONAL)
**Dependências:** Lua 5.4 + sol2 (binding C++/Lua)  

#### O que implementar:
- Expor ECS, Input, Math à Lua
- Script como componente ECS (`LuaScript { std::string path }`)
- Callbacks: `onStart()`, `onUpdate(dt)`, `onDestroy()`

---

## ARQUITETURA DO ENGINE LOOP (quando tudo estiver pronto)

```
main()
  └── Application::Run()
        ├── Init()
        │     ├── Window::Create()
        │     ├── RenderContext::Init()
        │     ├── AudioEngine::Init()
        │     ├── PhysicsWorld::Init()
        │     └── SceneManager::LoadInitialScene()
        │
        └── Loop (até fechar):
              ├── Window::PollEvents()
              ├── Input::Update()
              ├── SceneManager::Update(dt)
              │     ├── PhysicsSystem::Update(dt)
              │     ├── ScriptSystem::Update(dt)
              │     ├── AnimationSystem::Update(dt)
              │     └── AudioSystem::Update(dt)
              ├── RenderSystem::Render()
              │     ├── ShadowPass
              │     ├── GeometryPass
              │     ├── LightingPass
              │     └── PostProcessPass
              └── Window::SwapBuffers()
```

---

## ESTRUTURA DE PASTAS FINAL PREVISTA

```
genesisengine/
├── engine/
│   ├── core/           ✅ FEITO
│   ├── platform/       ❌ Janela + Input (próximo)
│   ├── renderer/       ❌ OpenGL/Vulkan
│   ├── ecs/            ❌ Entity Component System
│   ├── assets/         ❌ Asset Manager
│   ├── audio/          ❌ miniaudio
│   ├── physics/        ❌ Jolt Physics
│   ├── scene/          ❌ Cenas e serialização
│   └── scripting/      ❌ Lua (opcional)
├── editor/             ❌ Editor visual (futuro distante)
├── sandbox/            ❌ Jogo de teste/demo
├── tests/
│   └── core/           ✅ FEITO
├── demo/               ✅ FEITO (standalone)
├── docs/
├── tools/
├── CONTINUACAO.md      ← ESTE FICHEIRO
└── CMakeLists.txt
```

---

## DEPENDÊNCIAS A ADICIONAR (por módulo)

| Módulo | Biblioteca | Como obter |
|---|---|---|
| platform | SDL3 | `apt install libsdl3-dev` |
| renderer | GLAD | Gerar em glad.dav1d.de |
| assets | stb_image | Single-header, copiar para vendor/ |
| assets | tinyobjloader | Single-header, copiar para vendor/ |
| audio | miniaudio | Single-header, copiar para vendor/ |
| physics | Jolt Physics | FetchContent no CMake |
| scripting | Lua + sol2 | FetchContent no CMake |

---

## CONVENÇÕES DE CÓDIGO

- **Namespace:** `genesis::core::`, `genesis::renderer::`, `genesis::ecs::`, etc.
- **Naming:** PascalCase para classes, camelCase para variáveis, UPPER_CASE para macros
- **Headers:** Sempre `#pragma once`, nunca include guards
- **Erros:** Nunca lançar exceções — usar `std::expected<T, Error>` (C++23) ou callbacks
- **Alocações:** Nunca usar `new`/`malloc` diretamente — sempre passar por IAllocator
- **Threads:** Proteger estado partilhado com `std::mutex` ou operações atómicas
- **Constexpr:** Usar sempre que possível (math, utils)
- **[[nodiscard]]:** Aplicar a todas as funções que retornam valores importantes

---

## COMPILAR NO LINUX (passo a passo completo)

```bash
# 1. Instalar dependências
sudo apt install build-essential cmake ninja-build git python3

# 2. Clonar o repositório
git clone https://github.com/DUVALL-py/genesisengine.git
cd genesisengine

# 3. Configurar CMake
mkdir build && cd build
cmake .. \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_COMPILER=g++ \
  -DGENESIS_WARNINGS_AS_ERRORS=ON

# 4. Compilar
ninja -j$(nproc)

# 5. Correr testes
ctest --output-on-failure

# 6. Correr demo standalone (sem CMake)
cd ..
g++ -std=c++20 -O2 -Iengine/core/include demo/main.cpp -o demo/genesis_demo
./demo/genesis_demo
```

---

**Boa sorte! O core está sólido — agora é só construir por cima.**
