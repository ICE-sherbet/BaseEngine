// @IFieldStorage.h
// @brief
// @author ICE
// @date 2023/05/26
//
// @details

#pragma once
#include <span>

#include "MonoClassTypeInfo.h"
#include "MonoGCHandle.h"
#include "Ref.h"

namespace base_engine {
class IFieldStorage : public RefCounted {
 public:
  IFieldStorage(MonoFieldInfo* field_info) : field_info_(field_info) {}

  virtual void SetRuntimeInstance(GCHandle instance) = 0;

  virtual Variant GetValueVariant() const = 0;
  virtual void SetValueVariant(const Variant& buffer) = 0;

  const MonoFieldInfo* GetFieldInfo() const { return field_info_; }

 protected:
  MonoFieldInfo* field_info_ = nullptr;
};

class FieldStorage : public IFieldStorage {
 public:
  explicit FieldStorage(MonoFieldInfo* field_info);

  GCHandle GetInstance() const;

  template <typename T>
  T GetValue() {
    if (instance_ != nullptr) {
      if (const bool success = GetValueRuntime(&data_); !success) return T();

      T value = T();
      return value;
    }
    if (!data_) return T();

    return static_cast<T>(data_);
  }
  template <typename T>
  void SetValue(const T& value) {
    if (instance_ != nullptr) {
      const auto data = Variant{value};
      SetValueRuntime(&data);
    } else {
      data_ = Variant{value};
    }
  }
  void SetRuntimeInstance(const GCHandle instance) override;

  Variant GetValueVariant() const override;

  void SetValueVariant(const Variant& buffer) override;

 private:
  void SetValueRuntime(const Variant* in_buffer) const;
  bool GetValueRuntime(Variant* out_buffer) const;

 private:
  Variant data_{VariantType::kNil};
  GCHandle instance_ = nullptr;
};
}  // namespace base_engine
