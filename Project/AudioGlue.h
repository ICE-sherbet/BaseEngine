// @AudioGlue.h
// @brief
// @author ICE
// @date 2023/04/19
//
// @details

#pragma once
#include <cstdint>

#include "Asset.h"
#include "MonoGlueMacro.h"

namespace base_engine::glue::internal_calls {

void AudioComponentSetAudio(uint64_t entity_id, const AssetHandle* in_handle);
void AudioComponentGetAudio(uint64_t entity_id, AssetHandle* out_handle);
void AudioComponentStop(uint64_t entity_id, const AssetHandle* in_handle);
void AudioComponentPlay(uint64_t entity_id, const AssetHandle* in_handle);
void AudioEnginePlayOneShot(const AssetHandle* in_handle);
void AudioEnginePlay(const AssetHandle* in_handle);
void AudioEnginePlayLoop(const AssetHandle* in_handle);

struct AudioGlue {
  void operator()() const {
    BE_ADD_INTERNAL_CALL(AudioComponentSetAudio);
    BE_ADD_INTERNAL_CALL(AudioComponentGetAudio);
    BE_ADD_INTERNAL_CALL(AudioComponentStop);
    BE_ADD_INTERNAL_CALL(AudioComponentPlay);
    BE_ADD_INTERNAL_CALL(AudioEnginePlayOneShot);
    BE_ADD_INTERNAL_CALL(AudioEnginePlay);
    BE_ADD_INTERNAL_CALL(AudioEnginePlayLoop);
  }
};
}  // namespace base_engine::glue::internal_calls
