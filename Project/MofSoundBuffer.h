// @MofSoundBuffer.h
// @brief
// @author ICE
// @date 2023/04/19
//
// @details

#pragma once
#include <Sound/XAudio/XAudioSoundBuffer.h>

#include "SoundBuffer.h"

namespace base_engine {
class MofSoundBuffer : public ISoundBuffer {
 public:
  bool Load(const char* path) override;
  bool Load(const char* path, size_t buffer_count) override;

  bool Play() override;
  bool Stop() override;

  bool Release();
  void SetLoop(const bool loop) override;
  bool IsLoop() override;

private:
  Mof::CSoundBuffer sound_buffer_;
};
}  // namespace base_engine
