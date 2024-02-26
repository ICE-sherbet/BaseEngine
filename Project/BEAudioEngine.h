// @BEAudioEngine.h
// @brief
// @author ICE
// @date 2024/02/26
// 
// @details

#pragma once
#include <array>
#include <soloud.h>

#include "IBaseEngineAudioEngine.h"

namespace base_engine
{
class BEAudioEngine final : public IBaseEngineAudioEngine {
 public:
  bool Play(std::shared_ptr<ISoundBuffer> buffer) override;
  bool Stop(std::shared_ptr<ISoundBuffer> buffer) override;
  void Initialize() override;
  ~BEAudioEngine() override;
  Ref<Audio> CreateAudio(const std::filesystem::path& path, int buffer_count) override;

  static constexpr size_t MaxBusCount = 4;
 private:
  std::unique_ptr<SoLoud::Soloud> soloud_;
  std::array<SoLoud::Bus, MaxBusCount> buses_;
};
}  // namespace base_engine