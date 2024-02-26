#include "BEAudio.h"

#include "BESoundBuffer.h"
#include "soloud.h"
#include "soloud_speech.h"
#include "soloud_thread.h"

namespace base_engine
{
BEAudio::BEAudio(const SoLoud::Soloud* soloud,const std::string& file_path,
                 size_t buffer_count) {

  soloud_ = soloud;

  sound_ = std::shared_ptr<BESoundBuffer>();
  sound_->Load(file_path.c_str(), buffer_count);
}

std::shared_ptr<ISoundBuffer> BEAudio::GetBuffer() const {
	return sound_;
}
}
