// @RendererContext.h
// @brief
// @author ICE
// @date 2023/10/30
//
// @details

#pragma once
#include "Ref.h"

namespace base_engine {

class RendererContext : public RefCounted {
 public:
  virtual ~RendererContext() = default;
  virtual void Init() = 0;
  static Ref<RendererContext> Create();
};
}  // namespace base_engine
