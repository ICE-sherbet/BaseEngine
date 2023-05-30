// @SignalHandle.h
// @brief リスナーへのイベント通知
// @author ICE
// @date 2023/03/07
//
// @details 発火されると事前に登録したメンバー関数ポインタを呼び出す

#pragma once
#include "Delegate.h"

namespace becs {
template <typename Type>
class Sink;

template <typename Type, typename Allocator>
class SignalHandler;

/**
 * \brief リスナーとの通知ハンドラー
 * \tparam Ret
 * \tparam Args
 * \tparam Allocator
 */
template <typename Ret, typename... Args, typename Allocator>
class SignalHandler<Ret(Args...), Allocator> {
  friend class Sink<SignalHandler<Ret(Args...), Allocator>>;

  using AllocTraits = std::allocator_traits<Allocator>;
  using ContainerType = std::vector<
      delegate<Ret(Args...)>,
      typename AllocTraits::template rebind_alloc<delegate<Ret(Args...)>>>;

 public:
  using AllocatorType = Allocator;
  using SizeType = std::size_t;
  using SinkType = Sink<SignalHandler<Ret(Args...), Allocator>>;

  SignalHandler() noexcept(
      std::is_nothrow_default_constructible_v<AllocatorType> &&
          std::is_nothrow_constructible_v<ContainerType, const AllocatorType &>)
      : SignalHandler{AllocatorType{}} {}

  explicit SignalHandler(const AllocatorType &allocator) noexcept(
      std::is_nothrow_constructible_v<ContainerType, const AllocatorType &>)
      : calls{allocator} {}

  SignalHandler(const SignalHandler &other) noexcept(
      std::is_nothrow_copy_constructible_v<ContainerType>)
      : calls{other.calls} {}

  SignalHandler(
      const SignalHandler &other,
      const AllocatorType
          &allocator) noexcept(std::
                                   is_nothrow_constructible_v<
                                       ContainerType, const ContainerType &,
                                       const AllocatorType &>)
      : calls{other.calls, allocator} {}

  SignalHandler(SignalHandler &&other) noexcept(
      std::is_nothrow_move_constructible_v<ContainerType>)
      : calls{std::move(other.calls)} {}

  SignalHandler(SignalHandler &&other, const AllocatorType &allocator) noexcept(
      std::is_nothrow_constructible_v<ContainerType, ContainerType &&,
                                      const AllocatorType &>)
      : calls{std::move(other.calls), allocator} {}

  SignalHandler &operator=(const SignalHandler &other) noexcept(
      std::is_nothrow_copy_assignable_v<ContainerType>) {
    calls = other.calls;
    return *this;
  }

  SignalHandler &operator=(SignalHandler &&other) noexcept(
      std::is_nothrow_move_assignable_v<ContainerType>) {
    calls = std::move(other.calls);
    return *this;
  }

  void swap(SignalHandler &other) noexcept(
      std::is_nothrow_swappable_v<ContainerType>) {
    using std::swap;
    swap(calls, other.calls);
  }

  [[nodiscard]] constexpr AllocatorType get_allocator() const noexcept {
    return calls.get_allocator();
  }

  /**
   * \brief 接続されているリスナー数を返す
   * \return 現在の接続されているリスナー数
   */
  [[nodiscard]] SizeType size() const noexcept { return calls.size(); }

  /**
   * @brief 1つでもリスナーに接続されているかどうか
   * @return 接続されていない False / 接続されている True
   */
  [[nodiscard]] bool empty() const noexcept { return calls.empty(); }

  /**
   * @brief 通知を送るトリガー
   *
   * すべてのリスナーに通知を送る.
   *
   * @param args リスナーの引数
   */
  void publish(Args... args) const {
    for (auto &&call : std::as_const(calls)) {
      call(args...);
    }
  }

 private:
  ContainerType calls;
};

/**
 * \brief シンクの接続部クラス
 * クライアントが自身で解放を行える
 */
class Connection {
  template <typename>
  friend class sink;

  Connection(delegate<void(void *)> fn, void *ref)
      : disconnect_{fn}, signal_{ref} {}

 public:
  Connection() : disconnect_{}, signal_{} {}

  [[nodiscard]] explicit operator bool() const noexcept {
    return static_cast<bool>(disconnect_);
  }

  void release() {
    if (disconnect_) {
      disconnect_(signal_);
      disconnect_.reset();
    }
  }

 private:
  delegate<void(void *)> disconnect_;
  void *signal_;
};

/**
 * \brief リスナーをシグナルに接続する。
 * SignalHandler と Sink
 * を完全に分離することで、クライアントには発行機能を公開しない。
 * Sinkのライフタイムが参照するシグナルのライフタイムを超えると未定義動作に繋がります。
 * \tparam Ret
 * \tparam Args
 * \tparam Allocator
 */
template <typename Ret, typename... Args, typename Allocator>
class Sink<SignalHandler<Ret(Args...), Allocator>> {
  using SignalType = SignalHandler<Ret(Args...), Allocator>;
  using DifferenceType = typename SignalType::container_type::difference_type;

  template <auto Candidate, typename Type>
  static void release(Type value_or_instance, void *signal) {
    Sink{*static_cast<SignalType *>(signal)}.disconnect<Candidate>(
        value_or_instance);
  }

  template <auto Candidate>
  static void release(void *signal) {
    Sink{*static_cast<SignalType *>(signal)}.disconnect<Candidate>();
  }

  auto before(delegate<Ret(Args...)> call) {
    const auto &calls = signal->calls;
    const auto it = std::find(calls.cbegin(), calls.cend(), std::move(call));

    Sink other{*this};
    other.offset = calls.cend() - it;
    return other;
  }

 public:
  Sink(SignalHandler<Ret(Args...), Allocator> &ref) noexcept
      : offset{}, signal{&ref} {}

  [[nodiscard]] bool empty() const noexcept { return signal->calls.empty(); }

  template <auto Function>
  [[nodiscard]] Sink before() {
    delegate<Ret(Args...)> call{};
    call.template connect<Function>();
    return before(std::move(call));
  }

  template <auto Candidate, typename Type>
  [[nodiscard]] Sink before(Type &&value_or_instance) {
    delegate<Ret(Args...)> call{};
    call.template connect<Candidate>(value_or_instance);
    return before(std::move(call));
  }

  template <
      typename Type,
      typename = std::enable_if_t<
          !std::is_same_v<std::decay_t<std::remove_pointer_t<Type>>, void>,
          Sink>>
  [[nodiscard]] Sink before(Type &value_or_instance) {
    return before(&value_or_instance);
  }

  [[nodiscard]] Sink before(const void *value_or_instance) {
    Sink other{*this};

    if (value_or_instance) {
      const auto &calls = signal->calls;
      const auto it =
          std::find_if(calls.cbegin(), calls.cend(),
                       [value_or_instance](const auto &delegate) {
                         return delegate.data() == value_or_instance;
                       });

      other.offset = calls.cend() - it;
    }

    return other;
  }

  [[nodiscard]] Sink before() {
    Sink other{*this};
    other.offset = signal->calls.size();
    return other;
  }

  template <auto Candidate, typename... Type>
  Connection connect(Type &&...value_or_instance) {
    disconnect<Candidate>(value_or_instance...);

    delegate<Ret(Args...)> call{};
    call.template connect<Candidate>(value_or_instance...);
    signal->calls.insert(signal->calls.end() - offset, std::move(call));

    delegate<void(void *)> conn{};
    conn.template connect<&release<Candidate, Type...>>(value_or_instance...);
    return {std::move(conn), signal};
  }
  template <auto Candidate, typename... Type>
  void disconnect(Type &&...value_or_instance) {
    auto &calls = signal->calls;
    delegate<Ret(Args...)> call{};
    call.template connect<Candidate>(value_or_instance...);
    calls.erase(std::remove(calls.begin(), calls.end(), std::move(call)),
                calls.end());
  }

  template <typename Type,
            typename = std::enable_if_t<!std::is_same_v<
                std::decay_t<std::remove_pointer_t<Type>>, void>>>
  void disconnect(Type &value_or_instance) {
    disconnect(&value_or_instance);
  }

  void disconnect(const void *value_or_instance) {
    if (value_or_instance) {
      auto &calls = signal->calls;
      auto predicate = [value_or_instance](const auto &delegate) {
        return delegate.data() == value_or_instance;
      };
      calls.erase(
          std::remove_if(calls.begin(), calls.end(), std::move(predicate)),
          calls.end());
    }
  }

  void disconnect() { signal->calls.clear(); }

 private:
  DifferenceType offset;
  SignalType *signal;
};

}  // namespace becs