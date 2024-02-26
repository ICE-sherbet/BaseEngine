#if defined(__MOF__)
#include "MofAudioEngine.h"

#include "MofAudio.h"

namespace base_engine {
bool MofAudioEngine::Play(const std::shared_ptr<ISoundBuffer> buffer) {
  return buffer->Play();
}

bool MofAudioEngine::Stop(const std::shared_ptr<ISoundBuffer> buffer) {
  return buffer->Stop();
}

void MofAudioEngine::Initialize()
{

}

Ref<Audio> MofAudioEngine::CreateAudio(const std::filesystem::path& path, int buffer_count)
{
  return new MofAudio(path.string(), buffer_count);
}
}  // namespace base_engine

#endif