#include "BEAudioEngine.h"

#include "Assert.h"
#include "BEAudio.h"

namespace base_engine {
bool BEAudioEngine::Play(std::shared_ptr<ISoundBuffer> buffer) {
  return buffer->Play();
}

bool BEAudioEngine::Stop(std::shared_ptr<ISoundBuffer> buffer) { return false; }

void BEAudioEngine::Initialize() {
  soloud_ = std::make_unique<SoLoud::Soloud>();
  const auto success = soloud_->init() == SoLoud::SO_NO_ERROR;
  BE_CORE_ASSERT(success, "Failed to initialize SoLoud");
}

BEAudioEngine::~BEAudioEngine() {
  if (soloud_) {
    soloud_->deinit();
    soloud_.reset();
  }
}

Ref<Audio> BEAudioEngine::CreateAudio(const std::filesystem::path& path,
                                      int buffer_count) {
  return new BEAudio(soloud_.get(), path.string(), buffer_count);
}
}  // namespace base_engine
