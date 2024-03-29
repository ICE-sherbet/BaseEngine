﻿#include "RenderMof.h"

#include <Mof.h>
#include <Utilities/GraphicsUtilities.h>

#include "GameWindow.h"
#include "Material.h"
#include "MofShader.h"
using Mof::CGraphicsUtilities;
namespace base_engine {
void RenderMof::Initialize() {
  MofU32 sw = window::kWidth * 0.75f;
  MofU32 sh = window::kHeight * 0.75f;

  target_texture_.CreateTarget(sw, sh, PIXELFORMAT_R8G8B8A8_UNORM,
                               BUFFERACCESS_GPUREADWRITE);
  target_texture_2.CreateTarget(sw, sh, PIXELFORMAT_R8G8B8A8_UNORM,
                               BUFFERACCESS_GPUREADWRITE);
}

RenderMof::RenderMof() {}

void RenderMof::AddTexture(const ITexturePtr texture, const Vector& position,
                           const Vector& scale, float angle, const Rect& uv,
                           const Color& color,
                           Mof::TextureAlignment alignment) {
  CGraphicsUtilities::RenderScaleRotateTexture(
      roundf(position.x + camera_center_position_.x),
      roundf(position.y + camera_center_position_.y), scale.x, scale.y, angle,
      uv, color, alignment, texture);
}

void RenderMof::AddTexture(ITexturePtr texture, const Matrix44& wMat,
                           const Rect& uv, const Color& color,
                           const Vector3& pivot) {
  CMatrix44 camera_mat;
  camera_mat.SetTranslation(camera_center_position_.x,
                            camera_center_position_.y, 0);
  CGraphicsUtilities::RenderTexture(wMat * camera_mat, uv, color, pivot,
                                    texture);
}

void RenderMof::AddTexture(ITexturePtr texture, const Vector& position,
                           const Vector& scale, float angle, const Rect& uv,
                           const Color& color, Mof::TextureAlignment alignment,
                           const Material& material) {
  const auto shader =
      std::dynamic_pointer_cast<MofShader>(material.GetShader());
  CGraphicsUtilities::RenderTexture(
      roundf(position.x + camera_center_position_.x),
      roundf(position.y + camera_center_position_.y), color, alignment, texture,
      shader->GetShader(), shader->GetShaderBind());
}

void RenderMof::AddLine(const Vector& position1, const Vector& position2,
                        const Color& color) {
  CGraphicsUtilities::RenderLine(position1.x + camera_center_position_.x,
                                 position1.y + camera_center_position_.y,
                                 position2.x + camera_center_position_.x,
                                 position2.y + camera_center_position_.y,
                                 color);
}

void RenderMof::AddRect(const Rect& rect, const Color& color) {
  CGraphicsUtilities::RenderFillRect(rect.Left + camera_center_position_.x,
                                     rect.Top + camera_center_position_.y,
                                     rect.Right + camera_center_position_.x,
                                     rect.Bottom + camera_center_position_.y,
                                     color);
}

void RenderMof::AddRectFrame(const Rect& rect, const Color& color) {
  CGraphicsUtilities::RenderRect(rect.Left + camera_center_position_.x,
                                 rect.Top + camera_center_position_.y,
                                 rect.Right + camera_center_position_.x,
                                 rect.Bottom + camera_center_position_.y,
                                 color);
}

void RenderMof::AddCircle(const Circle& circle, const Color& color) {
  CGraphicsUtilities::RenderFillCircle(
      circle.Position.x + camera_center_position_.x,
      circle.Position.y + camera_center_position_.y, circle.r, color);
}

void RenderMof::AddCircleFrame(const Circle& circle, const Color& color) {
  CGraphicsUtilities::RenderCircle(
      circle.Position.x + camera_center_position_.x,
      circle.Position.y + camera_center_position_.y, circle.r, color);
}

void RenderMof::SetCameraPosition(const Vector& position) {
  camera_position_ = position;
  const auto& graphics = CGraphicsUtilities::GetGraphics();
  camera_center_position_ = {
      graphics->GetTargetWidth() / static_cast<float>(2) - camera_position_.x,
      graphics->GetTargetHeight() / static_cast<float>(2) - camera_position_.y};
}

void RenderMof::SetMaterial(const std::shared_ptr<Material>& material) {
  camera_material_ = material;
}

void RenderMof::Begin() {
  MofU32 sw = window::kWidth * 0.75f;
  MofU32 sh = window::kHeight * 0.75f;
  hold_render_target_buffer_ = g_pGraphics->GetRenderTarget();
  g_pGraphics->ClearTarget(0.2f, 0.2f, 0.6f, 0.0f, 0.0f, 0);

  g_pGraphics->SetRenderTarget(target_texture_.GetRenderTarget(),
                               g_pGraphics->GetDepthTarget());

  CGraphicsUtilities::RenderFillRect(0, 0, 1920, 1080, MOF_COLOR_HBLACK);
}

void RenderMof::End() {
  g_pGraphics->SetRenderTarget(hold_render_target_buffer_,
                               g_pGraphics->GetDepthTarget());

  CGraphicsUtilities::RenderTexture(0, 0, &target_texture_2);
}

IBaseEngineRender::Vector RenderMof::GetCameraPosition() {
  return camera_position_;
}

IBaseEngineRender::ITexturePtr RenderMof::GetTargetTexture() {
  return &target_texture_;
}

void RenderMof::Next() {
  g_pGraphics->SetRenderTarget(target_texture_2.GetRenderTarget(),
                               g_pGraphics->GetDepthTarget());

  CGraphicsUtilities::RenderTexture(0, 0, &target_texture_);

}
}  // namespace base_engine
