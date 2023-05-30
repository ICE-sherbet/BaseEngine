// @PhysicsEngineData.h
// @brief
// @author ICE
// @date 2023/05/09
// 
// @details

#pragma once
#include "PhysicsData.h"
#include "Ref.h"

namespace base_engine::physics
{
class PhysicsEngineData : public RefCounted {
public:
	PhysicsData physics_data_;
};
}
