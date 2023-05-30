#include "IBaseEngineAssetManager.h"

#include "EditorAssetManager.h"
using namespace base_engine;
IBaseEngineAssetManager* IBaseEngineAssetManager::Create()
{
  return new EditorAssetManager;
}
