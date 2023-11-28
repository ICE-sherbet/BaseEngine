#include "Renderer2D.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "FrameBuffer.h"
#include "Pipeline.h"
#include "RenderPass.h"
#include "RendererApi.h"
#include "VulkanShaderCompiler.h"

namespace base_engine {
constexpr int kMaxVertices = 5000 * 4;
constexpr int kMaxIndices = 5000 * 6;

namespace {
glm::vec4 quad_vertex_positions[4];

}

Renderer2D::Renderer2D() { Init(); }

Renderer2D::~Renderer2D() {}
glm::mat4 CameraProj;
void Renderer2D::BeginScene(const glm::mat4& viewProj, const glm::mat4& view) {
  Renderer::Submit([ubsCamera = ubs_camera_, viewProj]() mutable {
    uint32_t bufferIndex = Renderer::RT_GetCurrentFrameIndex();
    ubsCamera->RT_Get()->RT_SetData(&viewProj, sizeof(UBCamera));
  });

  quad_index_count_ = 0;
  uint32_t frameIndex = Renderer::GetCurrentFrameIndex();

  for (uint32_t i = 0; i < quad_vertex_buffer_ptr_.size(); i++)
    quad_vertex_buffer_ptr_[i] = quad_vertex_buffer_bases_[i][frameIndex];

  for (uint32_t i = 1; i < texture_slots_.size(); i++)
    texture_slots_[i] = nullptr;
}

void Renderer2D::EndScene() {
  uint32_t frameIndex = Renderer::GetCurrentFrameIndex();

  render_command_buffer_->Begin();

  uint32_t dataSize = 0;

  for (uint32_t i = 0; i <= quad_buffer_write_index_; i++) {
    dataSize = (uint32_t)((uint8_t*)quad_vertex_buffer_ptr_[i] -
                          (uint8_t*)quad_vertex_buffer_bases_[i][frameIndex]);
    if (dataSize) {
      uint32_t indexCount = i == quad_buffer_write_index_
                                ? quad_index_count_ - (kMaxIndices * i)
                                : kMaxIndices;
      quad_vertex_buffers_[i][frameIndex]->SetData(
          quad_vertex_buffer_bases_[i][frameIndex], dataSize);

      for (uint32_t i = 0; i < texture_slots_.size(); i++) {
        if (texture_slots_[i])
          quad_material_->Set("u_Textures", texture_slots_[i], i);
        else
          quad_material_->Set("u_Textures", none_texture_, i);
      }

      Renderer::BeginRenderPass(render_command_buffer_, quad_pass_);
      Renderer::RenderGeometry(render_command_buffer_, quad_pass_->GetPipeline(),
                               quad_material_,
                               quad_vertex_buffers_[i][frameIndex],
                               quad_index_buffer_, glm::mat4(1.0f), indexCount);
      Renderer::EndRenderPass(render_command_buffer_);
    }
  }

  render_command_buffer_->End();
  render_command_buffer_->Submit();
}

void Renderer2D::DrawQuad(const glm::mat4& transform, const glm::vec4& color) {
  uint32_t frameIndex = Renderer::GetCurrentFrameIndex();

  constexpr size_t quadVertexCount = 4;
  const float textureIndex = 0.0f;  // White Texture
  constexpr glm::vec2 textureCoords[] = {
      {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};
  const float tilingFactor = 1.0f;

  quad_buffer_write_index_ = quad_index_count_ / kMaxIndices;
  if (quad_buffer_write_index_ >= quad_vertex_buffer_bases_.size()) {
    AddQuadBuffer();
    quad_vertex_buffer_ptr_.emplace_back();  // TODO(Yan): check
    quad_vertex_buffer_ptr_[quad_buffer_write_index_] =
        quad_vertex_buffer_bases_[quad_buffer_write_index_][frameIndex];
  }

  auto& bufferPtr = quad_vertex_buffer_ptr_[quad_buffer_write_index_];

  for (size_t i = 0; i < quadVertexCount; i++) {
    bufferPtr->Position = transform * quad_vertex_positions[i];
    auto t = CameraProj * glm::vec4{bufferPtr->Position, 1};

    bufferPtr->Color = color;
    bufferPtr->TexCoord = textureCoords[i];
    bufferPtr->TexIndex = textureIndex;
    bufferPtr->TilingFactor = tilingFactor;
    bufferPtr++;
  }

  quad_index_count_ += 6;
}

void Renderer2D::Init() {
  render_command_buffer_ = RenderCommandBuffer::Create(0, "Renderer2D");
  ubs_camera_ = UniformBufferSet::Create(sizeof(UBCamera));

  FrameBufferSpecification framebuffer_spec;
  framebuffer_spec.Attachments = {ImageFormat::RGBA32F, ImageFormat::Depth};
  framebuffer_spec.Samples = 1;
  framebuffer_spec.ClearColorOnLoad = false;
  framebuffer_spec.ClearColor = {1.0f, 0.5f, 0.5f, 1.0f};
  framebuffer_spec.DebugName = "Renderer2D FrameBuffer";

  Ref<FrameBuffer> framebuffer = FrameBuffer::Create(framebuffer_spec);
  {
    PipelineSpecification pipelineSpecification;
    pipelineSpecification.DebugName = "Renderer2D-Quad";

    auto shader =
        VulkanShaderCompiler::Compile("Renderer2D.glsl", false, false);
    pipelineSpecification.Shader = shader;
    pipelineSpecification.BackfaceCulling = false;
    pipelineSpecification.TargetFrameBuffer = framebuffer;
    pipelineSpecification.Layout = {{ShaderDataType::kFloat3, "a_Position"},
                                    {ShaderDataType::kFloat4, "a_Color"},
                                    {ShaderDataType::kFloat2, "a_TexCoord"},
                                    {ShaderDataType::kFloat, "a_TexIndex"},
                                    {ShaderDataType::kFloat, "a_TilingFactor"}};

    RenderPassSpecification quadSpec;
    quadSpec.DebugName = "Renderer2D-Quad";
    quadSpec.Pipeline = Pipeline::Create(pipelineSpecification);
    quad_pass_ = RenderPass::Create(quadSpec);
    quad_pass_->SetInput("Camera", ubs_camera_);
    BE_CORE_VERIFY(quad_pass_->Validate());
    quad_pass_->Bake();

    quad_vertex_buffers_.resize(1);
    quad_vertex_buffer_bases_.resize(1);
    quad_vertex_buffer_ptr_.resize(1);

    quad_vertex_buffers_[0].resize(3);
    quad_vertex_buffer_bases_[0].resize(3);

    for (uint32_t i = 0; i < 3; i++) {
      uint64_t allocationSize = kMaxVertices * sizeof(QuadVertex);
      quad_vertex_buffers_[0][i] = VertexBuffer::Create(allocationSize);
      quad_vertex_buffer_bases_[0][i] = new QuadVertex[kMaxVertices];
    }

    uint32_t* quadIndices = new uint32_t[kMaxIndices];

    uint32_t offset = 0;
    for (uint32_t i = 0; i < kMaxIndices; i += 6) {
      quadIndices[i + 0] = offset + 0;
      quadIndices[i + 1] = offset + 1;
      quadIndices[i + 2] = offset + 2;

      quadIndices[i + 3] = offset + 2;
      quadIndices[i + 4] = offset + 3;
      quadIndices[i + 5] = offset + 0;

      offset += 4;
    }

    {
      uint64_t allocationSize = kMaxIndices * sizeof(uint32_t);
      quad_index_buffer_ = IndexBuffer::Create(quadIndices, allocationSize);
    }
    delete[] quadIndices;
  }

  none_texture_ = Renderer::GetWhiteTexture();

  texture_slots_[0] = none_texture_;

  quad_material_ =
      Material::Create(quad_pass_->GetPipeline()->GetShader(), "QuadMaterial");

  const float size = 0.5f;
  const float start = -size;
  const float end = size * 1;
  const float z = 0;
  quad_vertex_positions[3] = {start, start, z, 1.0f};
  quad_vertex_positions[0] = {start, end, z, 1.0f};
  quad_vertex_positions[1] = {end, end, z, 1.0f};
  quad_vertex_positions[2] = {end, start, z, 1.0f};
}

void Renderer2D::Shutdown() {}

void Renderer2D::AddQuadBuffer() {
  VertexBufferPerFrame& newVertexBuffer = quad_vertex_buffers_.emplace_back();
  QuadVertexBasePerFrame& newVertexBufferBase =
      quad_vertex_buffer_bases_.emplace_back();

  newVertexBuffer.resize(3);
  newVertexBufferBase.resize(3);
  for (uint32_t i = 0; i < 3; i++) {
    uint64_t allocationSize = kMaxVertices * sizeof(QuadVertex);
    newVertexBuffer[i] = VertexBuffer::Create(allocationSize);
    newVertexBufferBase[i] = new QuadVertex[kMaxVertices];
  }
}
}  // namespace base_engine
