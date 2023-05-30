// @IBaseEngineAudioEngine.h
// @brief
// @author ICE
// @date 2023/04/19
//
// @details

#pragma once
#include <memory>

#include "SoundBuffer.h"

namespace base_engine {
class IBaseEngineAudioEngine {
 public:
  virtual ~IBaseEngineAudioEngine() = default;

  static IBaseEngineAudioEngine* Create();

  virtual bool Play(std::shared_ptr<ISoundBuffer> buffer) = 0;
  virtual bool Stop(std::shared_ptr<ISoundBuffer> buffer) = 0;
};
}  // namespace base_engine
