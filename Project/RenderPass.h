// @RenderPass.h
// @brief
// @author ICE
// @date 2023/11/24
//
// @details

#pragma once
#include <string>
#include <glm/vec4.hpp>

#include "Image.h"
#include "Pipeline.h"
#include "Ref.h"
#include "RendererTexture.h"
#include "UniformBuffer.h"
#include "UniformBufferSet.h"

namespace base_engine {

	struct RenderPassSpecification {
  Ref<Pipeline> Pipeline;
  std::string DebugName;
  glm::vec4 MarkerColor;
};

class RenderPass : public RefCounted{
 public:
  virtual ~RenderPass() = default;

  virtual RenderPassSpecification& GetSpecification() = 0;
  virtual const RenderPassSpecification& GetSpecification() const = 0;

  virtual void SetInput(std::string_view name,
                        Ref<UniformBufferSet> uniformBufferSet) = 0;
  virtual void SetInput(std::string_view name,
                        Ref<UniformBuffer> uniformBuffer) = 0;

  virtual void SetInput(std::string_view name, Ref<RendererTexture2D> texture) = 0;

  virtual void SetInput(std::string_view name, Ref<Image2D> image) = 0;

  virtual Ref<Image2D> GetOutput(uint32_t index) = 0;
  virtual Ref<Image2D> GetDepthOutput() = 0;
  virtual uint32_t GetFirstSetIndex() const = 0;

  virtual Ref<FrameBuffer> GetTargetFrameBuffer() const = 0;
  virtual Ref<Pipeline> GetPipeline() const = 0;

  virtual bool Validate() = 0;
  virtual void Bake() = 0;
  virtual bool Baked() const = 0;
  virtual void Prepare() = 0;

  static Ref<RenderPass> Create(const RenderPassSpecification& spec);
};
}  // namespace base_engine
