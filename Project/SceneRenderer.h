// @SceneRenderer.h
// @brief
// @author ICE
// @date 2023/03/22
//
// @details

#pragma once
#include <glm/fwd.hpp>

#include "Application.h"
#include "Camera.h"
#include "FrameBuffer.h"
#include "Material.h"
#include "Ref.h"
#include "RenderCommandBuffer.h"
#include "RendererTieringSetting.h"
#include "StaticMesh.h"
#include <vulkan/vulkan.h>
namespace base_engine {
class Scene;

namespace ShaderDef {
enum class AOMethod { None = 0, GTAO = 1 << 1 };
}  // namespace ShaderDef

struct SceneRendererOptions {
  bool ShowGrid = true;
  bool ShowSelectedInWireframe = false;

  enum class PhysicsColliderView { SelectedEntity = 0, All = 1 };

  bool ShowPhysicsColliders = false;
  PhysicsColliderView PhysicsColliderMode = PhysicsColliderView::SelectedEntity;
  bool ShowPhysicsCollidersOnTop = false;
  glm::vec4 SimplePhysicsCollidersColor = glm::vec4{0.2f, 1.0f, 0.2f, 1.0f};
  glm::vec4 ComplexPhysicsCollidersColor = glm::vec4{0.5f, 0.5f, 1.0f, 1.0f};

  // General AO
  float AOShadowTolerance = 1.0f;

  // GTAO
  bool EnableGTAO = true;
  bool GTAOBentNormals = false;
  int GTAODenoisePasses = 4;

  // SSR
  bool EnableSSR = false;
  ShaderDef::AOMethod ReflectionOcclusionMethod = ShaderDef::AOMethod::None;
};

struct SSROptionsUB {
  // SSR
  glm::vec2 HZBUvFactor;
  glm::vec2 FadeIn = {0.1f, 0.15f};
  float Brightness = 0.7f;
  float DepthTolerance = 0.8f;
  float FacingReflectionsFading = 0.1f;
  int MaxSteps = 70;
  uint32_t NumDepthMips;
  float RoughnessDepthTolerance = 1.0f;
  bool HalfRes = true;
  char Padding[3]{0, 0, 0};
  bool EnableConeTracing = true;
  char Padding1[3]{0, 0, 0};
  float LuminanceFactor = 1.0f;
};

struct SceneRendererCamera {
  Camera Camera;
  glm::mat4 ViewMatrix;
  float Near, Far;
  float FOV;
};

struct BloomSettings {
  bool Enabled = true;
  float Threshold = 1.0f;
  float Knee = 0.1f;
  float UpsampleScale = 1.0f;
  float Intensity = 1.0f;
  float DirtIntensity = 1.0f;
};

struct DOFSettings {
  bool Enabled = false;
  float FocusDistance = 0.0f;
  float BlurSize = 1.0f;
};

struct SceneRendererSpecification {
  renderer::RendererTieringSettings Tiering;
  uint32_t NumShadowCascades = 4;

  bool EnableEdgeOutlineEffect = false;
  bool JumpFloodPass = true;
};

class SceneRenderer : public RefCounted {
 public:
  struct Statistics {
    uint32_t DrawCalls = 0;
    uint32_t Meshes = 0;
    uint32_t Instances = 0;
    uint32_t SavedDraws = 0;

    float TotalGPUTime = 0.0f;
  };
  SceneRenderer(Ref<Scene> scene, SceneRendererSpecification specification =
                                      SceneRendererSpecification());
  virtual ~SceneRenderer(){}

  void Init();

  void Shutdown() {}

  void ResetUniformBuffer() {}

  struct GPUTimeQueries {
    uint32_t DirShadowMapPassQuery = 0;
    uint32_t SpotShadowMapPassQuery = 0;
    uint32_t DepthPrePassQuery = 0;
    uint32_t HierarchicalDepthQuery = 0;
    uint32_t PreIntegrationQuery = 0;
    uint32_t LightCullingPassQuery = 0;
    uint32_t GeometryPassQuery = 0;
    uint32_t PreConvolutionQuery = 0;
    uint32_t GTAOPassQuery = 0;
    uint32_t GTAODenoisePassQuery = 0;
    uint32_t AOCompositePassQuery = 0;
    uint32_t SSRQuery = 0;
    uint32_t SSRCompositeQuery = 0;
    uint32_t BloomComputePassQuery = 0;
    uint32_t JumpFloodPassQuery = 0;
    uint32_t CompositePassQuery = 0;
  } m_GPUTimeQueries;

  Statistics m_Statistics;

 private:
  void SetupRenderPass() {

  }

  void SetupModelBuffers()
  {
	  
  }
  SceneRendererSpecification specification_;
  Ref<RenderCommandBuffer> command_buffer_;


  Ref<Scene> scene_;
};
}  // namespace base_engine
