// @BEAudio.h
// @brief
// @author ICE
// @date 2024/02/26
// 
// @details

#pragma once
#include <soloud.h>

#include "Audio.h"

namespace base_engine
{
class BEAudio final : public Audio {
 public:
  explicit BEAudio(const SoLoud::Soloud* soloud, const std::string& file_path, size_t buffer_count = 1);
  std::shared_ptr<ISoundBuffer> GetBuffer() const override;

 private:

  const SoLoud::Soloud* soloud_;
  std::shared_ptr<ISoundBuffer> sound_;
};
}  // namespace base_engine
