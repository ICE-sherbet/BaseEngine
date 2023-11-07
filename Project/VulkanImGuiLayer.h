// @VulkanImGuiLayer.h
// @brief
// @author ICE
// @date 2023/11/07
//
// @details

#pragma once
#include "ImGuiLayer.h"

namespace base_engine {
class VulkanImGuiLayer : public ImGuiLayer {
 public:
  void Init() override;

  void Begin() override;
  void End() override;
};
}  // namespace base_engine
