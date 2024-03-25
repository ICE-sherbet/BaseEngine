#include "Application.h"

#include <vulkan/vulkan.h>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "AssetManager.h"
#include "EditorCamera.h"
#include "IWindow.h"
#include "PrimitiveMeshFactory.h"
#include "Raytracer.h"
#include "Platform/Windows/Window.h"
#include "RenderCommandBuffer.h"
#include "Renderer2D.h"
#include "RendererApi.h"
#include "RendererTexture.h"
#include "VulkanImGuiLayer.h"
#include "VulkanShaderCompiler.h"
#include "VulkanSwapChain.h"

namespace base_engine {
Application* Application::instance_ = nullptr;

namespace {
VulkanSwapChain& GetSwapChain() {
  return static_cast<Window&>(Application::Get().GetWindow()).GetSwapChain();
}
}  // namespace

Application::Application(const ApplicationSpecification& spec)
    : render_thread_(spec.threading_policy) {
  std::filesystem::current_path("..\\Resource\\");

  instance_ = this;
  Renderer::Init();

  window_ = std::unique_ptr<IWindow>(base_engine::IWindow::Create());
  window_->Init();
}

inline VkTransformMatrixKHR toTransformMatrix(glm::mat4 matrix) {
  glm::mat4 temp = glm::transpose(matrix);
  VkTransformMatrixKHR out_matrix;
  memcpy(&out_matrix, &temp, sizeof(VkTransformMatrixKHR));
  return out_matrix;
}

void Application::Run() {
  is_running_ = true;
  VulkanImGuiLayer layer;
  layer.Init();

  auto renderer2d = Ref<Renderer2D>::Create();


  auto  // View行列を計算
      viewMat = glm::lookAt(
          glm::vec3(0.0, 0.0, 10.0),  // ワールド空間でのカメラの座標
          glm::vec3(0.0, 0.0, 0.0),  // 見ている位置の座標
          glm::vec3(
              0.0, 1.0,
              0.0)  // 上方向を示す。(0,1.0,0)に設定するとy軸が上になります
      );

  // Projection行列を計算
  auto projectionMat =
      glm::perspective(glm::radians(45.0f),  // ズームの度合い(通常90～30)
                       16.0f / 9.0f,         // アスペクト比
                       0.1f,     // 近くのクリッピング平面
                       100.0f);  // 遠くのクリッピング平面

  auto viewProjection = projectionMat * viewMat;

  Raytracer raytracer{};
  auto sphere = PrimitiveMeshFactory::CreateSphere(1);

  auto mesh = AssetManager::GetAsset<Mesh>(sphere);
  raytracer.SetupBottomLevelAS({mesh});

  ObjInstance instance{};
  instance.matrix = toTransformMatrix(glm::mat4{});
  instance.instanceId = 0;
  instance.mesh = sphere;
  std::vector instances{instance};
  raytracer.SetupTopLevelAS(instances);
  while (is_running_) {
    render_thread_.BlockUntilRenderComplete();

    render_thread_.Kick();
    window_->Update();

    Renderer::Submit([&]() mutable { GetSwapChain().BeginFrame(); });

    Application* app = this;

    raytracer.SetupTopLevelAS(instances);
    raytracer.UpdateRenderTarget();
    // Renderer::Submit([&layer]() { layer.Begin(); });
    // Renderer::Submit([&layer]() { layer.End(); });
    renderer2d->BeginScene(viewProjection, viewMat);
    auto p1 = glm::mat4{1};
    auto c1 = glm::vec4{1, 1, 1, 1};
    renderer2d->DrawQuad(p1, c1);
    auto p2 = glm::translate(p1, {0, 0, -3});
    renderer2d->DrawQuad(p2, c1);
    renderer2d->EndScene();
    Renderer::EndFrame();
    Renderer::Submit([&]() mutable { GetSwapChain().Present(); });

    current_frame_index_ = (current_frame_index_ + 1) % 3;
  }
}
}  // namespace base_engine
