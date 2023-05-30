// @Math.h
// @brief
// @author ICE
// @date 2023/03/07
//
// @details

#pragma once
#include "DependenciesDefine.h"

#ifdef _USE_MOF_LIBRARY_
#include <Math/Quaternion.h>
#include <Math/Matrix44.h>
#include <Math/Matrix33.h>
#include <Math/Vector2.h>
#include <Math/Vector3.h>
#include <Math/Vector4.h>

namespace base_engine {
using Floating = MofFloat;
}

#elif _BE_LIBRARY_

#endif