// @Renderer2D.h
// @brief
// @author ICE
// @date 2023/11/23
//
// @details

#pragma once
#include <glm/glm.hpp>

#include "IndexBuffer.h"
#include "Material.h"
#include "Ref.h"
#include "RenderCommandBuffer.h"
#include "RenderPass.h"

namespace base_engine {
class Renderer2D : public RefCounted {
 public:
  explicit Renderer2D();
  virtual ~Renderer2D();

  void BeginScene(const glm::mat4& viewProj, const glm::mat4& view);
  void EndScene();
  void DrawQuad(const glm::mat4& transform, const glm::vec4& color);

 private:
  void Init();
  void Shutdown();

  void AddQuadBuffer();

  Ref<RenderCommandBuffer> render_command_buffer_;

  struct QuadVertex {
    glm::vec3 Position;
    glm::vec4 Color;
    glm::vec2 TexCoord;
    float TexIndex;
    float TilingFactor;
  };

  using VertexBufferPerFrame = std::vector<Ref<VertexBuffer>>;
  Ref<RenderPass> quad_pass_;
  std::vector<VertexBufferPerFrame> quad_vertex_buffers_;
  Ref<IndexBuffer> quad_index_buffer_;
  Ref<Material> quad_material_;

  uint32_t quad_index_count_ = 0;
  using QuadVertexBasePerFrame = std::vector<QuadVertex*>;
  std::vector<QuadVertexBasePerFrame> quad_vertex_buffer_bases_;
  std::vector<QuadVertex*> quad_vertex_buffer_ptr_;
  uint32_t quad_buffer_write_index_ = 0;

  Ref<UniformBufferSet> ubs_camera_;

  //UniformBuffer
  struct UBCamera {
    glm::mat4 ViewProjection;
  };
  std::array<Ref<RendererTexture2D>, 32> texture_slots_;
  Ref<RendererTexture2D> none_texture_;
};
}  // namespace base_engine
