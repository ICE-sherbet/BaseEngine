#include "RendererSubmit.h"

#include "RendererApi.h"

namespace base_engine {
RendererSubmit::RendererSubmit() {
  command_queue_[0] = new RenderCommandQueue();
  command_queue_[1] = new RenderCommandQueue();
}

RendererSubmit::~RendererSubmit() {
  delete command_queue_[0];
  delete command_queue_[1];
}

void RendererSubmit::Execute() {
  command_queue_[queue_submission_index_]->Execute();
}

void RendererSubmit::SwapQueues() {
  queue_submission_index_ =
      (queue_submission_index_ + 1) % kRenderCommandQueueCount;
}

RenderCommandQueue& RendererSubmit::GetRenderResourceReleaseQueue(
    uint32_t index) {
  return resource_free_queue_[index];
}

RenderCommandQueue& RendererSubmit::GetRenderCommandQueue() {
  return *command_queue_[queue_submission_index_];
}

RenderCommandQueue& RendererSubmit::GetRenderCommandQueue() const {
  return *command_queue_[queue_submission_index_];
}

uint32_t RendererSubmit::RT_GetCurrentFrameIndex() {
  return Renderer::RT_GetCurrentFrameIndex();
}
}  // namespace base_engine
