#include "MofAudioEngine.h"

namespace base_engine {
bool MofAudioEngine::Play(const std::shared_ptr<ISoundBuffer> buffer) {
  return buffer->Play();
}

bool MofAudioEngine::Stop(const std::shared_ptr<ISoundBuffer> buffer) {
  return buffer->Stop();
}
}  // namespace base_engine
