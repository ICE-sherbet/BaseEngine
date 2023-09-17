#pragma once
#include <functional>
#include <memory>
#include <string_view>
#include <vector>

#include "BaseEngineCore.h"
#include "ILayer.h"
#include "GameManager.h"

class ResourceContainer;

namespace base_engine {
class GameScene;
class Game {
 public:
  ~Game();
  bool Initialize();
  void Update();
  void Render();
  void Shutdown();

  void SetScene(const Ref<Scene>& scene) { scene_ = scene; }
 private:

  uint64_t actor_id_max_ = 0;

  BaseEngineCore engine_;
  std::unique_ptr<GameManager> game_manager_;
  std::shared_ptr<ResourceContainer> resource_container_;
  std::vector<std::shared_ptr<GameScene>> scenes_;
  std::vector<std::function<void()>> next_frame_event_;
  std::unique_ptr<ILayer> editor_layer_;
  Ref<Scene> scene_;
};
}  // namespace base_engine