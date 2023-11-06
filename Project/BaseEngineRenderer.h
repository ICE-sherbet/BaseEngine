// @BaseEngineRenderer.h
// @brief
// @author ICE
// @date 2023/10/16
//
// @details

#pragma once
#include "Ref.h"
#include "RendererContext.h"

namespace base_engine {
class IBaseEngineRendererContext {
 public:
  static IBaseEngineRendererContext* Create();
  virtual ~IBaseEngineRendererContext() = default;

  virtual void Init() = 0;
  
  virtual Ref<RendererContext> GetRendererContext() = 0;
};
}  // namespace base_engine