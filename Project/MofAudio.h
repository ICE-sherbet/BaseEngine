// @MofAudio.h
// @brief
// @author ICE
// @date 2023/04/18
//
// @details

#pragma once
#include "Audio.h"
#include "MofSoundBuffer.h"

namespace base_engine {
class MofAudio final : public Audio {
 public:
	explicit MofAudio(const std::string& file_path,size_t buffer_count = 1);
	std::shared_ptr<ISoundBuffer> GetBuffer() const override;

private:
  struct Deleter;
  std::shared_ptr<ISoundBuffer> sound_;
};
}  // namespace base_engine
