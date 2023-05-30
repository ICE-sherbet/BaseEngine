#include "MonoGlue.h"

#include <mono/jit/jit.h>
#include <mono/metadata/class.h>
#include <mono/metadata/exception.h>
#include <mono/metadata/loader.h>
#include <mono/metadata/object.h>
#include <mono/metadata/reflection.h>

#include <source_location>

#include "CSharpScriptEngine.h"
#include "ManagedComponentStorage.h"
#include "MonoGlueInternalCalls.h"
#include "ObjectEntity.h"
#include "TypeUtilities.h"

namespace base_engine {
class MonoGlue::Impl {
 public:
  void Clear();
  void RegisterComponentTypes();
  void RegisterInternalCalls();
  ~Impl();

 private:
  template <typename Glue>
  static void RegisterInternalCallGlues() {
    std::invoke(Glue{});
  }

  template <typename TComponent>
  void RegisterManagedComponent() const {
    std::string_view component_type_name =
        TypeUtilities::GetTypeName<TComponent, true>();
    std::string component_name =
        fmt::format("BaseEngine_ScriptCore.Components.{}", component_type_name);

    if (MonoType* managed_type = mono_reflection_type_from_name(
            component_name.data(),
            CSharpScriptEngine::GetInstance()->GetCoreImage())) {
      ManagedComponentStorage::GetInstance()
          ->GetInstance()
          ->RegisterComponent<TComponent>(managed_type);
    } else {
      // TODO [ログ][エラー処理] エラーログ出力
    }
  }
};

void MonoGlue::Impl::Clear() {
  ManagedComponentStorage::GetInstance()->Clear();
}

void MonoGlue::Impl::RegisterComponentTypes() {
  using namespace component;
  RegisterManagedComponent<TransformComponent>();
  RegisterManagedComponent<SpriteRendererComponent>();
  RegisterManagedComponent<AudioComponent>();
}

void MonoGlue::Impl::RegisterInternalCalls() {
  using namespace glue::internal_calls;
  RegisterInternalCallGlues<AudioGlue>();
  RegisterInternalCallGlues<DebugGlue>();
  RegisterInternalCallGlues<EntityGlue>();
  RegisterInternalCallGlues<SceneGlue>();
  RegisterInternalCallGlues<ScriptGlue>();
  RegisterInternalCallGlues<InputGlue>();
  RegisterInternalCallGlues<SpriteGlue>();
  RegisterInternalCallGlues<TextureGlue>();
  RegisterInternalCallGlues<TransformGlue>();
}

MonoGlue::Impl::~Impl() {}

MonoGlue::MonoGlue() { impl_ = std::make_unique<Impl>(); }

void MonoGlue::RegisterGlue() const {
  ManagedComponentStorage::GetInstance()->Init();
  impl_->RegisterComponentTypes();
  impl_->RegisterInternalCalls();
}

MonoGlue::~MonoGlue() = default;

}  // namespace base_engine
