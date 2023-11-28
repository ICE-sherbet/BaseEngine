// @RenderCommandBuffer.h
// @brief
// @author ICE
// @date 2023/11/23
// 
// @details

#pragma once
#include <string>

#include "Ref.h"

namespace base_engine
{
class RenderCommandBuffer : public RefCounted
{
 public:
  virtual ~RenderCommandBuffer() = default;

  virtual void Begin() = 0;
  virtual void End() = 0;
  virtual void Submit() = 0;
  static Ref<RenderCommandBuffer> Create(uint32_t count = 0,
                                         const std::string& debugName = "");
};
	
};
