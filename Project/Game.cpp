#include "Game.h"

#include <Mof.h>
#include <Utilities/GraphicsUtilities.h>

#include <iostream>

#include "Actor.h"
#include "AssetManager.h"
#include "Audio.h"
#include "CSharpScriptEngine.h"
#include "ConnectableObject.h"
#include "DataComponents.h"
#include "EditorLayer.h"
#include "GameScene.h"
#include "IBaseEngineCollider.h"
#include "IBaseEngineRender.h"
#include "MethodBind.h"
#include "ObjectEntity.h"
#include "PhysicsObjectFactory.h"
#include "Profiler.h"
#include "RenderComponent.h"
#include "Scene.h"
#include "SceneSerializer.h"
#include "Script.h"
#include "Texture.h"
base_engine::IBaseEngineCollider* b_collision;

namespace base_engine {

bool Game::Initialize() {
  class Hoge {
    int ans = 0;

   public:
    void Sub(int n) { ans -= n; }
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
  scene_ = Ref<Scene>::Create("Sample");
  editor_layer_ = std::make_unique<editor::EditorLayer>(this);
  editor_layer_->Initialize(scene_);
  BASE_ENGINE(Render)->Initialize();
  BASE_ENGINE(AssetManager)->Initialize();

  actors_.reserve(1024);
  BASE_ENGINE(Collider)->SetCallBack(this);
  b_collision = BASE_ENGINE(Collider);
  // TODO [リファクタリング][Script] Script周りの初期化処理をリファクタリング
  {
    CSharpScriptEngine::GetInstance()->SetScene(scene_);
    CSharpScriptEngine::GetInstance()->InitMono();
  }

  if (false)
  // TODO デバッグ用の生成処理をクラス化する
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
  BE_PROFILE_FRAME("Main Thread");
  BE_PROFILE_FUNC("GameUpdate");

  editor_layer_->OnUpdate();

  CreateObjectRegister();
  ProcessInput();
  UpdateGame();
  b_collision->Collide();
  scene_->OnUpdate(Mof::CUtilities::GetFrameSecond());

  // if (g_pInput->IsKeyPush(MOFKEY_B)) {
  //   g_pGraphics->SetScreenMode(false);
  // }
  // if (g_pInput->IsKeyPush(MOFKEY_V)) {
  //   g_pGraphics->SetScreenMode(true);
  // }
  if (g_pInput->IsKeyPush(MOFKEY_P) && g_pInput->IsKeyHold(MOFKEY_LCONTROL)) {
    SceneSerializer serializer(scene_);
    serializer.Serialize("Test.bscene");
  }
  if (g_pInput->IsKeyPush(MOFKEY_L) && g_pInput->IsKeyHold(MOFKEY_LCONTROL)) {
    SceneSerializer serializer(scene_);
    serializer.Deserialize("Test.bscene");
  }
  
}

void Game::Shutdown() { Clear(); }

void Game::AddActor(Actor* actor) {
  AddActor(actor,
           scenes_.empty() ? std::weak_ptr<GameScene>{} : scenes_.front());
}

void Game::AddActor(Actor* actor, const std::weak_ptr<GameScene> scene) {
  actor_id_max_++;
  actor->id_.id = actor_id_max_;
  auto actor_ptr = ActorPtr{actor};
  actor_id_cash_.emplace_back(actor_ptr);
  pending_actors_.emplace_back(actor_ptr);
  if (!scene.expired()) scene.lock()->AddActor(actor_ptr);
  actor_ptr->SetScene(scene);
}

void Game::RemoveActor(Actor* actor) {
  if (const auto iter = std::ranges::find_if(
          actors_, [actor](const ActorPtr& n) { return n.get() == actor; });
      iter != actors_.end()) {
    const auto scene = actor->GetScene().lock();
    std::iter_swap(iter, actors_.end() - 1);

    actors_next_frame_delete_.emplace_back(actors_.back());

    actors_.pop_back();
    auto t = actors_next_frame_delete_.back().use_count();
    if (scene) scene->Sync();
  }
}

void Game::RemoveActor(Actor* actor, const std::weak_ptr<GameScene> scene) {
  if (scene.expired()) return;
}

ActorWeakPtr Game::GetActor(ActorId id) {
  if (actor_id_max_ < id.id) return ActorPtr{};
  return actor_id_cash_[id.id];
  return ActorPtr{};
}

ActorWeakPtr Game::FindTagActor(std::string_view tag) {
  if (const auto iter = std::ranges::find_if(
          actors_, [tag](const ActorPtr& n) { return n->GetTag() == tag; });
      iter != actors_.end()) {
    return *iter;
  }

  if (const auto iter = std::ranges::find_if(
          pending_actors_,
          [tag](const ActorPtr& n) { return n->GetTag() == tag; });
      iter != pending_actors_.end()) {
    return *iter;
  }
  return ActorPtr{};
}

void Game::AddSprite(RenderComponent* render_component) {
  const int my_draw_order = render_component->GetDrawOrder();
  auto iter = sprites_.begin();
  for (; iter != sprites_.end(); ++iter) {
    if (my_draw_order < (*iter)->GetDrawOrder()) {
      break;
    }
  }

  sprites_.insert(iter, render_component);
}

void Game::RemoveSprite(RenderComponent* render_component) {
  auto iter = std::ranges::find(sprites_, render_component);
  sprites_.erase(iter);
}

void Game::CreateObjectRegister() {
  for (int i = 0; i < next_frame_event_.size(); ++i) {
    next_frame_event_[i]();
  }
  next_frame_event_.clear();

  updating_actors_ = true;
  for (int i = 0; i < actors_next_frame_delete_.size(); ++i) {
    if (auto& actor = actors_next_frame_delete_[i]) actor.reset();
  }
  for (auto&& actor : actors_next_frame_delete_) {
    if (actor) actor.reset();
  }
  actors_next_frame_delete_.clear();
  for (int i = 0; i < pending_actors_.size(); ++i) {
    pending_actors_[i]->StartActor();
    actors_.emplace_back(pending_actors_[i]);
  }
  pending_actors_.clear();

  for (auto&& actor : actors_) {
    actor->AddComponent();
  }
  updating_actors_ = false;
}

void Game::ProcessInput() {
  updating_actors_ = true;
  for (const auto& actor : actors_) {
    actor->ProcessInput();
  }
  updating_actors_ = false;
}

void Game::UpdateGame() {
  updating_actors_ = true;
  for (int i = 0; i < actors_.size(); ++i) {
    auto actor = actors_[i];
    if (actors_.empty()) break;
    if (actor->Enable()) actor->UpdateActor();

    if (clear_wait_actors_) {
      clear_wait_actors_ = false;
      break;
    }
  }
  updating_actors_ = false;

  std::vector<ActorPtr> dead_actors;
  for (const auto& actor : actors_) {
    if (actor->GetState() == Actor::kDead) {
      dead_actors.emplace_back(actor);
    }
  }
  for (auto&& actor : dead_actors) {
    actor.reset();
  }
}

void Game::Clear() {
  pending_actors_.clear();
  actors_.clear();
  actors_next_frame_delete_.clear();
  debug_render_.clear();
  scenes_.clear();
}

void Game::AddScene(std::string_view name) {
  scenes_.emplace_back(std::make_shared<GameScene>(this, 0, name));
}
void Game::AddScene(std::string_view name, const size_t index) {
  scenes_.emplace_back(std::make_shared<GameScene>(this, scenes_.size(), name));
}

void Game::RemoveScene(GameScene* scene) {
  if (const auto iter = std::ranges::find_if(
          scenes_, [scene](auto& n) { return n.get() == scene; });
      iter != scenes_.end()) {
    std::iter_swap(iter, scenes_.end() - 1);
    scenes_.pop_back();
  }
}

void Game::SetNextFrameEvent(const std::function<void()>& event) {
  next_frame_event_.emplace_back(event);
}

void Game::Render(){
  BE_PROFILE_FUNC("GameRender");
  BASE_ENGINE(Render)->Begin();
  scene_->OnRender(0);
  BASE_ENGINE(Render)->Next();
  editor_layer_->OnRender();
  BASE_ENGINE(Render)->End();
}
Game::~Game() {}

}  // namespace base_engine