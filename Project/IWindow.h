// @IWindow.h
// @brief
// @author ICE
// @date 2023/10/17
//
// @details

#pragma once
#include "Ref.h"

namespace base_engine {
class IWindow : RefCounted {
 public:
	virtual ~IWindow() = default;
	static IWindow* Create();
  virtual void Init() = 0;
  virtual void Update() = 0;
  virtual bool IsShow() = 0;

  virtual uint32_t GetWidth() const = 0;
  virtual uint32_t GetHeight() const = 0;

  virtual void* GetNativeWindow() = 0;
};
}  // namespace base_engine