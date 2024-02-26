// @IBaseEngineAudioEngine.h
// @brief
// @author ICE
// @date 2023/04/19
//
// @details

#pragma once
#include <memory>

#include "Audio.h"
#include "SoundBuffer.h"

namespace base_engine {
class IBaseEngineAudioEngine {
 public:
  virtual ~IBaseEngineAudioEngine() = default;

  static IBaseEngineAudioEngine* Create();

  virtual void Initialize() = 0;

  virtual Ref<Audio> CreateAudio(const std::filesystem::path& path,int buffer_count) = 0;

  virtual bool Play(std::shared_ptr<ISoundBuffer> buffer) = 0;
  virtual bool Stop(std::shared_ptr<ISoundBuffer> buffer) = 0;
};
}  // namespace base_engine
