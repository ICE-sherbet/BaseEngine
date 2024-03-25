// @Math.h
// @brief
// @author ICE
// @date 2023/03/07
//
// @details

#pragma once
#include "DependenciesDefine.h"

#ifdef _USE_MOF_LIBRARY_
#include <Math/Matrix33.h>
#include <Math/Matrix44.h>
#include <Math/Quaternion.h>
#include <Math/Vector2.h>
#include <Math/Vector3.h>
#include <Math/Vector4.h>

namespace base_engine {
using Floating = MofFloat;
}

#elif defined(_USE_GLM_LIBRARY_)
#include <glm/glm.hpp>
#include <glm/detail/type_quat.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace base_engine {
using Floating = float;
}
#endif
