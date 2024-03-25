#include "Game.h"


#include <iostream>

#include "AssetManager.h"
#include "Audio.h"
#include "CSharpScriptEngine.h"
#include "ConnectableObject.h"
#include "DataComponents.h"
#include "EditorLayer.h"
#include "IBaseEngineRender.h"
#include "MethodBind.h"
#include "ObjectEntity.h"
#include "PhysicsObjectFactory.h"
#include "Profiler.h"
#include "Scene.h"
#include "SceneSerializer.h"
#include "Script.h"
#include "TestComponent.h"
#include "Texture.h"

namespace base_engine {


bool Game::Initialize() {
  class Hoge {
    int ans = 0;

   public:
    void Sub(int n) { ans -= n; }
    int Get() { return ans; }
    void Plus(int n) { ans += n; }
    void Test(std::string text, float v1, int v2) {
      std::cout << "Hoge:" << text << v1 << v2 << std::endl;
    }
  } hoge;
  class Piyo {
   public:
    void Test2(std::string text, float v1, int v2) {
      std::cout << "Piyo:" << text << v1 << v2 << std::endl;
    }
  } piyo;
  ConnectableObject object;
  object.Connect("Sub-method",
                 make_callable_function_pointer(&hoge, &Hoge::Sub));
  object.Connect("Plus-method",
                 make_callable_function_pointer(&hoge, &Hoge::Plus));
  object.Connect("Test-method",
                 make_callable_function_pointer(&hoge, &Hoge::Test));
  object.Connect("Test-method",
                 make_callable_function_pointer(&piyo, &Piyo::Test2));
  object.EmitSignal("Sub-method", 1);
  object.EmitSignal("Plus-method", 2);
  object.EmitSignal("Test-method", "test", 0.1f, 1);
  Variant a;
  MethodBindTR m(&Hoge::Get);
  auto result = m.call(&hoge,nullptr,0);
  make_callable_function_pointer(&hoge, &Hoge::Get).Call(nullptr,0,a);
  scene_ = Ref<Scene>::Create("Sample");
  //BASE_ENGINE(Render)->Initialize();
  BASE_ENGINE(AssetManager)->Initialize();
  ComponentDB::Initialize();
  
  editor_layer_ = std::make_unique<editor::EditorLayer>(this);
  editor_layer_->Initialize(scene_);

	// TODO [���t�@�N�^�����O][Script] Script����̏��������������t�@�N�^�����O
  {
    CSharpScriptEngine::GetInstance()->SetScene(scene_);
    CSharpScriptEngine::GetInstance()->InitMono();
  }

  if (false)
  // TODO �f�o�b�O�p�̐����������N���X������
  {
    auto entity = scene_->CreateEntity();
    physics::PhysicsObjectFactory physics_object_factory{scene_};
    physics_object_factory.CreateCircle(entity, {0, 0}, 50, 1, 1);
    auto entity2 = scene_->CreateEntity("Player2");
    physics_object_factory.CreateCircle(entity2, {500, 0}, 50, 1, 1);

    using namespace component;
    auto& sprite = entity.AddComponent<component::SpriteRendererComponent>();
    auto& sprite2 = entity2.AddComponent<component::SpriteRendererComponent>();
    auto& sound = entity.AddComponent<component::AudioComponent>();
    sound.audio_source = AssetManager::GetAsset<Audio>("Connect.mp3")->handle_;
    const auto p = AssetManager::GetAsset<Texture>("PlayerT.png");
    sprite.texture = p->handle_;
    sprite.pivot = Vector2{0.5, 0.5};
    sprite2.texture = p->handle_;
    auto& t = entity.GetComponent<TransformComponent>();
    auto& t2 = entity2.GetComponent<component::TransformComponent>();

    auto& script = entity.AddComponent<component::ScriptComponent>();
    auto asset = AssetManager::GetAsset<Script>("Demo.DemoBehavior.Player");
    script.script_class_handle = asset->handle_;

    CSharpScriptEngine::GetInstance()->RuntimeInitializeScriptEntity(entity);
  }
  scene_->OnInit();
  return true;
}

void Game::Update() {
  BE_PROFILE_FUNC();

  editor_layer_->OnUpdate();

  scene_->OnUpdate(0.016f);
}

void Game::Shutdown()
{
	
}

void Game::Render() {
  BE_PROFILE_SCOPE("GameRender");
  scene_->OnRender(0);
  editor_layer_->OnRender();
}
Game::~Game() {}

}  // namespace base_engine