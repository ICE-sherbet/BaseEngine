// @MofTexture.h
// @brief
// @author ICE
// @date 2023/04/04
//
// @details

#pragma once

#if defined(__MOF__)

#include <Graphics/DirectX11/DX11Texture.h>
#include <Graphics/Texture.h>

#include "Texture.h"
namespace base_engine {
class MofTexture final : public Texture {
 public:
  MofTexture(const std::string path) {
    texture_ = new Mof::CTexture;
    texture_->Load(path.c_str());
  }
  void* GetTexture() override { return texture_->GetTexture(); }
  ~MofTexture() override { texture_->Release(); }
  Mof::LPTexture texture_;
};
}  // namespace base_engine

#endif
