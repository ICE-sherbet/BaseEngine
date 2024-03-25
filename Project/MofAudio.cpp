#include "MofAudio.h"

#include "MofSoundBuffer.h"
#if defined(__MOF__)

#include "Mof.h"
namespace base_engine {

struct MofAudio::Deleter {
  void operator()(MofSoundBuffer* sound_buffer) const {
    sound_buffer->Release();
  }
};

MofAudio::MofAudio(const std::string& file_path, size_t buffer_count) {
  sound_ =
      std::shared_ptr<ISoundBuffer>(new MofSoundBuffer, MofAudio::Deleter{});
  sound_->Load(file_path.c_str(), buffer_count);
}

std::shared_ptr<ISoundBuffer> MofAudio::GetBuffer() const { return sound_; }
}  // namespace base_engine
#endif