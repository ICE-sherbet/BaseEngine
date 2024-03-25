#include "MofSoundBuffer.h"
#if defined(__MOF__)

namespace base_engine {
bool MofSoundBuffer::Load(const char* path) { return sound_buffer_.Load(path); }

bool MofSoundBuffer::Load(const char* path, const size_t buffer_count) {
  sound_buffer_.SetBufferCount(buffer_count);
  return sound_buffer_.Load(path);
}

bool MofSoundBuffer::Play() { return sound_buffer_.Play(); }

bool MofSoundBuffer::Stop() { return sound_buffer_.Stop(); }

bool MofSoundBuffer::Release() { return sound_buffer_.Release(); }

void MofSoundBuffer::SetLoop(const bool loop) { sound_buffer_.SetLoop(loop); }

bool MofSoundBuffer::IsLoop() { return sound_buffer_.IsLoop(); }
}  // namespace base_engine
#endif