#pragma once
// Fixture de teste do Reflection System. Este header propositalmente mistura
// tipos simples (float, int, bool) e um tipo composto (Vec3) para provar que
// o parser via libclang resolve tipos de verdade, não apenas os literais
// "float"/"int" — algo que uma abordagem baseada em regex teria dificuldade
// de fazer corretamente (ex.: typedefs, namespaces, tipos aninhados).

#include "genesis/core/math/Vec.hpp"
#include "genesis/core/reflection/ReflectMacros.hpp"

namespace genesis::test {

struct GENESIS_CLASS() ReflectedTransform {
    GENESIS_PROPERTY() float positionX = 0.0f;
    GENESIS_PROPERTY() float positionY = 0.0f;
    GENESIS_PROPERTY() float positionZ = 0.0f;
    GENESIS_PROPERTY() float scale = 1.0f;
    GENESIS_PROPERTY() int id = 0;
    GENESIS_PROPERTY() bool active = true;
    GENESIS_PROPERTY() genesis::core::math::Vec3 velocity;

    // Propositalmente NÃO anotado: prova que o gerador só reflete campos
    // marcados, ignorando os demais.
    float internalCache = 0.0f;
};

}  // namespace genesis::test
