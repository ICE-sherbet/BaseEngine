#include "AssetManager.h"

#include "EditorAssetManager.h"
namespace base_engine {
const AssetRegistry& AssetManager::GetAssetRegistry() {
  return static_cast<EditorAssetManager*>(BASE_ENGINE(AssetManager))
      ->GetAssetRegistry();
}

EditorAssetManager* AssetManager::GetEditorAssetManager()
{
  return static_cast<EditorAssetManager*>(BASE_ENGINE(AssetManager));
}
}  // namespace base_engine
