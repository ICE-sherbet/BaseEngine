﻿#include "GameData.h"

#include <Utilities/GraphicsUtilities.h>

#include <filesystem>

#include "AudioVolume.h"
#include "BaseEngineCore.h"
#include "ComponentServiceLocator.h"
#include "IBaseEngineRender.h"
#include "IBaseEngineTexture.h"
#include "Material.h"
#include "MofShader.h"
#include "ResourceContainer.h"
#include "StageContainer.h"
#include "TexturePaths.h"
#include "TransitionFadeSystem.h"
#include "VentGroupLocator.h"
using namespace base_engine;
/**
 * \brief Resourceフォルダ内にある画像ファイルを再帰的に探索し全てロードを行う
 */
void ResourceFolderTextureAllRegister() {
  namespace fs = std::filesystem;
  for (const auto& entry : fs::recursive_directory_iterator(".")) {
    const auto& path = entry.path();
    if (path.extension() != ".png") continue;
    BASE_ENGINE(Texture)->Load(path.generic_string());
  }
  BASE_ENGINE(Texture)->Load("Default");
}

void MaterialCreate() {
  using namespace base_engine;

  using namespace std::literals::string_literals;
  {
    const auto p =
        ResourceContainer::CreatePack<ResourceContainer::MaterialResourcePack>(
            "TestShader");
    const asset_system::ResourcePtr<Material> material_ptr =
        std::make_shared<asset_system::Resource<Material>>();
    const auto shader = std::make_shared<MofShader>("Shader/TestShader.hlsl");
    shader->CreateParameter({"cbGameParam"s, PropertyType::kBuffer, 16});
    const auto& res = material_ptr->Register(0);
    res->SetShader(shader);
    p->Register<Material>(material_ptr);
  }
}
void RegisterSystem(Game* game) {
  ServiceLocator::Instance().RegisterInstance<ITransitionFadeSystem>(
      std::make_shared<TransitionFadeSystem>(game));
}

void GameData::Register() {
  BASE_ENGINE(Render)->SetCameraPosition(
      {Mof::CGraphicsUtilities::GetGraphics()->GetTargetWidth() / 2.0f,
       Mof::CGraphicsUtilities::GetGraphics()->GetTargetHeight() / 2.0f});

  ResourceFolderTextureAllRegister();
  const auto stage_container = std::make_shared<StageContainer>();
  stage_container->Initialize();
  ServiceLocator::Instance().RegisterInstance(stage_container);

  ServiceLocator::Instance().RegisterInstance(
      std::make_shared<VentGroupLocator>());
  ServiceLocator::Instance().RegisterInstance(std::make_shared<AudioVolume>());

  MaterialCreate();
  RegisterSystem(game_);
}
