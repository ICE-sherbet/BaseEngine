// @ILayer.h
// @brief
// @author ICE
// @date 2023/05/19
//
// @details

#pragma once
#include "Scene.h"
#include "ObjectEntity.h"

namespace base_engine {

class ILayer {
 public:
	virtual ~ILayer() = default;
  virtual void Initialize(const Ref<Scene>& scene) = 0;
  virtual void OnUpdate() = 0;
  virtual void OnRender() = 0;
  virtual void SetSceneContext(const Ref<Scene>& scene) = 0;
};
}  // namespace base_engine