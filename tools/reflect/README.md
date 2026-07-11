# Genesis Reflection Tool

Gera metadados de reflexão a partir de **parsing real do AST C++** (via
libclang), não de registro manual por macro.

## Como funciona

1. Um header anota uma classe/struct com `GENESIS_CLASS()` e campos com
   `GENESIS_PROPERTY()` (ver `engine/core/include/genesis/core/reflection/ReflectMacros.hpp`).
2. Essas macros só produzem `__attribute__((annotate(...)))` quando a macro
   `GENESIS_REFLECT_TOOL` está definida — o que só acontece quando esta
   ferramenta invoca o parser. Numa build normal da engine (GCC) elas
   expandem para nada: zero custo de compilação/runtime.
3. `genesis_reflect.py` usa `clang.cindex` para parsear o header de verdade,
   encontra as declarações anotadas e emite um `<Nome>.generated.cpp` que
   registra `TypeInfo`/`PropertyInfo` em `genesis::core::reflection::TypeRegistry`,
   usando `offsetof(...)` calculado pelo **compilador real** ao compilar o
   arquivo gerado (não é o Python que calcula offsets de memória).

## Requisitos

```
pip install -r tools/reflect/requirements.txt
```

O pacote `libclang` do PyPI já inclui o binário `libclang.so`; não é preciso
instalar `libclang-dev` via apt. A ferramenta detecta automaticamente os
diretórios de include padrão do `g++` do sistema (necessário porque o
libclang empacotado não traz os headers padrão da libstdc++).

## Integração com CMake

Ver `cmake/GenesisReflection.cmake` — a função `genesis_add_reflected_header()`
registra um custom command que roda esta ferramenta antes da compilação e
adiciona o `.generated.cpp` resultante como fonte do alvo. Exemplo em
`tests/core/CMakeLists.txt`.

## Tipos suportados hoje

`float`, `int`, `bool`, `genesis::core::math::Vec3`. Adicionar um novo tipo
requer: (1) entrada em `TYPE_MAP` neste script, (2) especialização de
`PropertyTypeOf<T>` em `genesis/core/reflection/TypeInfo.hpp`.

## Limitações conhecidas (MVP)

- Não resolve herança entre tipos refletidos ainda.
- Não reflete métodos, só campos de dados (`GENESIS_FUNCTION()` é trabalho
  futuro, junto com Serialization).
- Um `.generated.cpp` por header, sem cache incremental além do que o
  próprio CMake já oferece via `DEPENDS`.
