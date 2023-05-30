#include "AudioGlue.h"

#include "AssetManager.h"
#include "Audio.h"
#include "EntityGlue.h"
#include "IBaseEngineAudioEngine.h"

namespace base_engine::glue::internal_calls {
void AudioComponentSetAudio(uint64_t entity_id, const AssetHandle* in_handle) {
  auto entity = GetEntity(entity_id);
  auto& audio_component = entity.GetComponent<component::AudioComponent>();
  audio_component.audio_source = *in_handle;
}

void AudioComponentGetAudio(uint64_t entity_id, AssetHandle* out_handle) {
  auto entity = GetEntity(entity_id);
  const auto& audio_component =
      entity.GetComponent<component::AudioComponent>();
  if (!audio_component.audio_source) return;
  if (!AssetManager::IsAssetHandleValid(audio_component.audio_source)) return;

  *out_handle = audio_component.audio_source;
}

void AudioComponentStop(uint64_t entity_id, const AssetHandle* in_handle) {
  auto entity = GetEntity(entity_id);
  const auto& audio_component =
      entity.GetComponent<component::AudioComponent>();
  if (!audio_component.audio_source) return;

  if (!AssetManager::IsAssetHandleValid(audio_component.audio_source)) return;

  if (const auto sound =
          AssetManager::GetAsset<Audio>(audio_component.audio_source);
      BASE_ENGINE(AudioEngine)->Stop(sound->GetBuffer())) {
  }
}

void AudioComponentPlay(uint64_t entity_id, const AssetHandle* in_handle) {
  auto entity = GetEntity(entity_id);
  const auto& audio_component =
      entity.GetComponent<component::AudioComponent>();
  if (!audio_component.audio_source) return;
  if (!AssetManager::IsAssetHandleValid(audio_component.audio_source)) return;

  if (const auto sound =
          AssetManager::GetAsset<Audio>(audio_component.audio_source);
      BASE_ENGINE(AudioEngine)->Play(sound->GetBuffer())) {
  }
}
void AudioEnginePlayOneShot(const AssetHandle* in_handle) {
  const auto sound = AssetManager::GetAsset<Audio>(*in_handle)->GetBuffer();
  sound->SetLoop(false);
  BASE_ENGINE(AudioEngine)->Play(sound);
}

void AudioEnginePlay(const AssetHandle* in_handle) {
  const auto sound = AssetManager::GetAsset<Audio>(*in_handle)->GetBuffer();
  sound->SetLoop(false);
  BASE_ENGINE(AudioEngine)->Play(sound);
}
void AudioEnginePlayLoop(const AssetHandle* in_handle) {
  const auto sound = AssetManager::GetAsset<Audio>(*in_handle)->GetBuffer();
  sound->SetLoop(true);
  BASE_ENGINE(AudioEngine)->Play(sound);
}
}  // namespace base_engine::glue::internal_calls
