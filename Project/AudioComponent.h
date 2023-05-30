// @AudioComponent.h
// @brief
// @author ICE
// @date 2023/04/18
// 
// @details

#pragma once
#include "Asset.h"
#include "UUID.h"

namespace base_engine::component
{
struct AudioComponent
{
  AssetHandle audio_source = kNullUuid;

  bool is_playing = false;

  AudioComponent() = default;
  AudioComponent(const AudioComponent& other) = default;
};

}
