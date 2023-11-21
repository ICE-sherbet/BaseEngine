// @RendererAPI.h
// @brief
// @author ICE
// @date 2023/11/06
//
// @details

#pragma once
#include "RenderThread.h"
#include "RendererSubmit.h"

namespace base_engine {

enum class RendererApiType { kNone, kVulkan };

class RendererApi {
 public:
  virtual void Init() = 0;
  virtual void Shutdown() = 0;

  virtual void BeginFrame() = 0;
  virtual void EndFrame() = 0;

  static constexpr RendererApiType Current() { return current_renderer_api_; }

 private:
  static constexpr RendererApiType current_renderer_api_ =
      RendererApiType::kVulkan;
};

class Renderer {
 public:
  static void Init();
  static void Shutdown();

  static void BeginFrame();
  static void EndFrame();

  template <typename FuncT>
  static void Submit(FuncT&& func) {
    submit_->Submit(std::forward<FuncT>(func));
  }

  template <typename FuncT>
  static void SubmitResourceFree(FuncT&& func) {
    submit_->SubmitResourceFree(std::forward<FuncT>(func));
  }
  static void RenderThreadFunc(RenderThread* render_thread);
  static void WaitAndRender(RenderThread* render_thread);
  static void SwapQueues();
  static uint32_t RT_GetCurrentFrameIndex();
  static RenderCommandQueue& GetRenderResourceReleaseQueue(uint32_t index);

 private:
  static inline RendererSubmit* submit_ = nullptr;
};

}  // namespace base_engine
