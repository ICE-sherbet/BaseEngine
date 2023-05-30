// @CSharpInstance.h
// @brief
// @author ICE
// @date 2023/04/06
//
// @details

#pragma once
#include <mono/metadata/object-forward.h>
#include <mono/metadata/object.h>

#include <memory>
#include <string>

class CSharpInstance {
 public:
  template <typename... TArgs>
  void CallMethod(const std::string& methodName, TArgs&&... args) {}

 private:
  void CallMethod(MonoMethod* managedMethod,
                  const void** parameters) {
    MonoObject* exception = NULL;
    mono_runtime_invoke(managedMethod, &mono_instance_,
                        const_cast<void**>(parameters), &exception);
  }

  std::shared_ptr<MonoObject> mono_instance_;
};
