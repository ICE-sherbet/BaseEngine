// @Ref.h
// @brief スマートポインタクラス
// @author ICE
// @date 2023/03/07
//
// @details

#pragma once
#include <atomic>
#include <cstdint>

namespace base_engine {
/**
 * \brief スマートポインタとして扱うオブジェクトの基底クラス
 * 継承させることで、Ref クラスからカウントを行う
 */
class RefCounted {
 public:
  void IncRefCount() const { ++ref_count_; }
  void DecRefCount() const { --ref_count_; }

  uint32_t GetRefCount() const { return ref_count_.load(); }

 private:
  mutable std::atomic<uint32_t> ref_count_ = 0;
};

namespace detail {
void AddToLiveReferences(void* instance);
void RemoveFromLiveReferences(void* instance);
bool IsLive(const void* instance);
}  // namespace detail

/**
 * \brief スマートポインタクラス
 * \tparam T RefCountedを継承したクラス
 */
template <typename T>
class Ref {
 public:
  Ref() : instance_(nullptr) {}

  Ref(std::nullptr_t n) : instance_(nullptr) {}

  Ref(T* instance) : instance_(instance) {
    static_assert(std::is_base_of_v<RefCounted, T>, "Class is not RefCounted!");

    IncRef();
  }

  template <typename T2>
  Ref(const Ref<T2>& other) {
    instance_ = (T*)other.instance_;
    IncRef();
  }

  template <typename T2>
  Ref(Ref<T2>&& other) {
    instance_ = (T*)other.instance_;
    other.instance_ = nullptr;
  }

  static Ref<T> CopyWithoutIncrement(const Ref<T>& other) {
    Ref<T> result = nullptr;
    result->instance_ = other.instance_;
    return result;
  }

  ~Ref() { DecRef(); }

  Ref(const Ref<T>& other) : instance_(other.instance_) { IncRef(); }

  Ref& operator=(std::nullptr_t) {
    DecRef();
    instance_ = nullptr;
    return *this;
  }

  Ref& operator=(const Ref<T>& other) {
    other.IncRef();
    DecRef();

    instance_ = other.instance_;
    return *this;
  }

  template <typename T2>
  Ref& operator=(const Ref<T2>& other) {
    other.IncRef();
    DecRef();

    instance_ = other.instance_;
    return *this;
  }

  template <typename T2>
  Ref& operator=(Ref<T2>&& other) {
    DecRef();

    instance_ = other.instance_;
    other.instance_ = nullptr;
    return *this;
  }

  operator bool() { return instance_ != nullptr; }
  operator bool() const { return instance_ != nullptr; }

  T* operator->() { return instance_; }
  const T* operator->() const { return instance_; }

  T& operator*() { return *instance_; }
  const T& operator*() const { return *instance_; }

  T* Raw() { return instance_; }
  const T* Raw() const { return instance_; }

  void Reset(T* instance = nullptr) {
    DecRef();
    instance_ = instance;
  }

  template <typename T2>
  Ref<T2> As() const {
    return Ref<T2>(*this);
  }

  template <typename... Args>
  static Ref<T> Create(Args&&... args) {
    // TODO newをオーバーロードし、生成のログなどを取れるようにする
    return Ref<T>(new T(std::forward<Args>(args)...));
  }

  bool operator==(const Ref<T>& other) const {
    return instance_ == other.instance_;
  }

  bool operator!=(const Ref<T>& other) const { return !(*this == other); }

  bool EqualsObject(const Ref<T>& other) {
    if (!instance_ || !other.instance_) return false;

    return *instance_ == *other.instance_;
  }

 private:
  void IncRef() const {
    if (instance_) {
      instance_->IncRefCount();
      detail::AddToLiveReferences((void*)instance_);
    }
  }

  void DecRef() const {
    if (instance_) {
      instance_->DecRefCount();
      if (instance_->GetRefCount() == 0) {
        delete instance_;
        detail::RemoveFromLiveReferences((void*)instance_);
        instance_ = nullptr;
      }
    }
  }

  template <class T2>
	friend class Ref;
  mutable T* instance_;
};

/**
 * \brief 弱参照スマートポインタクラス
 * \tparam T RefCountedを継承したクラス
 */
template <typename T>
class WeakRef {
 public:
  WeakRef() = default;

  WeakRef(Ref<T> ref) { instance_ = ref.Raw(); }

  WeakRef(T* instance) { instance_ = instance; }

  T* operator->() { return instance_; }
  const T* operator->() const { return instance_; }

  T& operator*() { return *instance_; }
  const T& operator*() const { return *instance_; }

  bool IsValid() const { return instance_ ? detail::IsLive(instance_) : false; }
  operator bool() const { return IsValid(); }

 private:
  T* instance_ = nullptr;
};

namespace detail {}

}  // namespace base_engine
