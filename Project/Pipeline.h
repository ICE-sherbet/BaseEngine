// @Pipeline.h
// @brief
// @author ICE
// @date 2023/11/24
//
// @details

#pragma once
#include <string>

#include "FrameBuffer.h"
#include "Ref.h"
#include "Shader.h"
#include "VertexBuffer.h"

namespace base_engine {
enum class PrimitiveTopology {
  None = 0,
  Points,
  Lines,
  Triangles,
  LineStrip,
  TriangleStrip,
  TriangleFan
};

enum class DepthCompareOperator {
  None = 0,
  Never,
  NotEqual,
  Less,
  LessOrEqual,
  Greater,
  GreaterOrEqual,
  Equal,
  Always,
};
struct PipelineSpecification {
  Ref<Shader> Shader;
  Ref<FrameBuffer> TargetFrameBuffer;

  VertexBufferLayout Layout;
  PrimitiveTopology Topology = PrimitiveTopology::Triangles;
  DepthCompareOperator DepthOperator = DepthCompareOperator::GreaterOrEqual;

  bool BackfaceCulling = true;
  bool DepthTest = true;
  bool DepthWrite = true;
  bool Wireframe = false;
  float LineWidth = 1.0f;

  std::string DebugName;
};
class Pipeline : public RefCounted {
 public:
  virtual ~Pipeline() = default;

  virtual PipelineSpecification& GetSpecification() = 0;
  virtual const PipelineSpecification& GetSpecification() const = 0;

  virtual void Invalidate() = 0;

  virtual Ref<Shader> GetShader() const = 0;

  static Ref<Pipeline> Create(const PipelineSpecification& spec);
};
}  // namespace base_engine
