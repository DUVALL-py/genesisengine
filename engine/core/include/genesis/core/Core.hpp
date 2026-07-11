#pragma once
// Header de conveniência: inclui toda a API pública do módulo Core.
// Módulos internos da engine devem preferir incluir apenas os headers
// específicos de que precisam, para manter tempos de build baixos.

#include "genesis/core/Assert.hpp"
#include "genesis/core/Log.hpp"
#include "genesis/core/Platform.hpp"
#include "genesis/core/jobs/JobCounter.hpp"
#include "genesis/core/jobs/JobSystem.hpp"
#include "genesis/core/reflection/ReflectMacros.hpp"
#include "genesis/core/reflection/TypeInfo.hpp"
#include "genesis/core/reflection/TypeRegistry.hpp"
#include "genesis/core/math/Mat4.hpp"
#include "genesis/core/math/Vec.hpp"
#include "genesis/core/memory/Allocator.hpp"
#include "genesis/core/memory/LinearAllocator.hpp"
#include "genesis/core/memory/PoolAllocator.hpp"
#include "genesis/core/serialization/Serializer.hpp"
#include "genesis/core/serialization/JsonSerializer.hpp"
#include "genesis/core/serialization/ReflectionSerializer.hpp"
