// @SoundBuffer.h
// @brief
// @author ICE
// @date 2023/04/19
//
// @details

#pragma once
namespace base_engine {
__interface ISoundBuffer {
  bool Load(const char* path);
  bool Load(const char* path,size_t buffer_count);
  bool Play();
  bool Stop();
  void SetLoop(bool loop);
  bool IsLoop();
};
}  // namespace base_engine
