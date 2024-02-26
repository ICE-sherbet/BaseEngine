// @BESoundBuffer.h
// @brief
// @author ICE
// @date 2024/02/26
//
// @details

#pragma once
#include <memory>
#include <soloud.h>
#include "SoundBuffer.h"

namespace base_engine {
class BESoundBuffer final : public ISoundBuffer {
 public:
  BESoundBuffer(SoLoud::Soloud* soloud);

  bool Load(const char* path) override;
  bool Load(const char* path, size_t buffer_count) override;
  bool Play() override;
  bool Stop() override;
  void SetLoop(bool loop) override;
  bool IsLoop() override;

private:
  std::unique_ptr<SoLoud::AudioSource> audio_source_;
  SoLoud::Soloud* soloud_;
};

}  // namespace base_engine
