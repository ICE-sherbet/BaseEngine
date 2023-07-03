// @AudioComponent.h
// @brief
// @author ICE
// @date 2023/04/18
//
// @details

#pragma once
#include "Asset.h"
#include "ComponentDB.h"
#include "ComponentProperty.h"
#include "UUID.h"
namespace base_engine::component {
struct AudioComponent {
  BE_COMPONENT(AudioComponent)
  AssetHandle audio_source = kNullUuid;

  bool is_playing = false;

  AudioComponent() = default;
  AudioComponent(const AudioComponent& other) = default;

  [[nodiscard]] AssetHandle AudioSource() const { return audio_source; }
  void SetAudioSource(const AssetHandle& audio_source) {
    this->audio_source = audio_source;
  }

  [[nodiscard]] bool IsPlaying() const { return is_playing; }
  void SetIsPlaying(bool is_playing) { this->is_playing = is_playing; }

  static void _Bind();
};

}  // namespace base_engine::component
