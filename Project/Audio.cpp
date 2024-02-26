#include "Audio.h"
#include "BaseEngineCore.h"
#include "IBaseEngineAudioEngine.h"

namespace base_engine {
Ref<Audio> AudioUtilities::Create(const std::filesystem::path& path) {
  // TODO バッファ数を変更できるようにする

  Ref<Audio> result = BASE_ENGINE(AudioEngine)->CreateAudio(path, 3);
  return result;
}

void AudioSerializer::Serialize(const AssetMetadata& metadata,
                                const Ref<Asset>& asset) const {}

bool AudioSerializer::TryLoadData(const AssetMetadata& metadata,
                                  Ref<Asset>& asset) const {
  asset = AudioUtilities::Create(metadata.file_path);
  asset->handle_ = metadata.handle;
  return true;
}

void AudioSerializer::GetRecognizedExtensions(
    std::list<std::string>* extensions) const {
  extensions->push_back(".mp3");
  extensions->push_back(".wav");
}

std::string AudioSerializer::GetAssetType(
    const std::filesystem::path& path) const {
  if (path.extension() == ".mp3" || path.extension() == ".wav") {
    return "Audio";
  }
  return "";
}
}  // namespace base_engine
