#include "AudioComponent.h"

namespace base_engine::component {
void AudioComponent::_Bind() {
  ComponentDB::BindMethod("SetAudioSource", &AudioComponent::SetAudioSource);
  ComponentDB::BindMethod("GetAudioSource", &AudioComponent::AudioSource);
  ADD_PROPERTY(PropertyInfo(VariantType::kAssetHandle, "audio_source"),
               "SetAudioSource", "GetAudioSource");

  ComponentDB::BindMethod("SetIsPlaying", &AudioComponent::SetIsPlaying);
  ComponentDB::BindMethod("GetIsPlaying", &AudioComponent::IsPlaying);
  ADD_PROPERTY(PropertyInfo(VariantType::kBool, "isPlaying"), "SetIsPlaying",
               "GetIsPlaying");

}
}  // namespace base_engine::component
