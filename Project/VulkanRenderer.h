// @VulkanRenderer.h
// @brief
// @author ICE
// @date 2023/11/06
//
// @details

#pragma once
#include "RendererApi.h"

namespace base_engine {

class VulkanRenderer : public RendererApi {
 public:
  void Init() override;
  void Shutdown() override;

  void BeginFrame() override;
  void EndFrame() override;

private:
};
}  // namespace base_engine