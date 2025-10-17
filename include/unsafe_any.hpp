#ifndef HEIM_UNSAFE_ANY_HPP
#define HEIM_UNSAFE_ANY_HPP

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <new>
#include <type_traits>
#include <utility>

namespace heim
{
class unsafe_any
{
private:
  using small_buffer_t = std::byte[sizeof(void *)];

  union storage_t
  {
    constexpr
    storage_t()
      : value{nullptr}
    { }

    constexpr
    storage_t(storage_t const &other)
    = delete;

    constexpr storage_t &
    operator=(storage_t const &other)
    = delete;

    void          *value;
    small_buffer_t buffer;

  };


  enum class operation : std::uint8_t
  {
    cast,
    copy,
    destroy,
    move
  };

  union manager_arg_t
  {
    void       *arg;
    unsafe_any *any;

  };

  using manager_t
  = const void *(*)(operation const, unsafe_any const *, manager_arg_t *);

private:
  storage_t m_storage;
  manager_t m_manager;

private:
  template<typename T>
  constexpr static bool
  to_buffer_v
  =   sizeof (T) <= sizeof (storage_t)
   && alignof(T) <= alignof(storage_t)
   && std::is_nothrow_move_constructible_v<T>;


  template<typename T>
  constexpr static const void *
  s_manage(operation const op, unsafe_any const *any, manager_arg_t *arg)
  requires (std::is_same_v<T, std::decay_t<T>>)
  {
    T const *ptr = nullptr;
    if constexpr (to_buffer_v<T>)
      ptr = reinterpret_cast<T const *>(&any->m_storage.buffer);
    else
      ptr = static_cast<T const *>(any->m_storage.value);

    switch (op)
    {
    case operation::cast:
      return static_cast<void const *>(ptr);
    case operation::copy:
      if constexpr (std::is_copy_constructible_v<T>)
      {
        if constexpr (to_buffer_v<T>)
        {
          ::new(&arg->any->m_storage.buffer) T{*ptr};
          arg->any->m_manager = any->m_manager;
        }
        else
        {
          arg->any->m_storage.value = new T{*ptr};
          arg->any->m_manager       = any->m_manager;
        }
      }
      break;
    case operation::destroy:
      if constexpr (to_buffer_v<T>)
        ptr->~T();
      else
        delete ptr;
      break;
    case operation::move:
      if constexpr (to_buffer_v<T>)
      {
        ::new(&arg->any->m_storage.buffer) T{std::move(*const_cast<T *>(ptr))};
        arg->any->m_manager = any->m_manager;

        ptr->~T();
        const_cast<unsafe_any *>(any)->m_manager = nullptr;
      }
      else
      {
        arg->any->m_storage.value = any->m_storage.value;
        arg->any->m_manager       = any->m_manager;

        const_cast<unsafe_any *>(any)->m_manager = nullptr;
      }
      break;
    }
    return nullptr;
  }


  template<typename T, typename ...Args>
  constexpr void
  m_emplace(Args &&...args)
  requires (
      std::is_same_v<T, std::decay_t<T>>
   && std::constructible_from<T, Args &&...>)
  {
    if constexpr (to_buffer_v<T>)
      ::new(&m_storage.buffer) T{std::forward<Args>(args)...};
    else
      m_storage.value = new T{std::forward<Args>(args)...};

    m_manager = &s_manage<T>;
  }


  template<typename T>
  [[nodiscard]]
  constexpr T *
  m_cast()
  noexcept
  requires (std::is_same_v<T, std::decay_t<T>>)
  {
    return static_cast<T *>(const_cast<void *>(
        s_manage<T>(operation::cast, this, nullptr)));
  }

  template<typename T>
  [[nodiscard]]
  constexpr T const *
  m_cast() const
  noexcept
  requires (std::is_same_v<T, std::decay_t<T>>)
  {
    return static_cast<T const *>(s_manage<T>(operation::cast, this, nullptr));
  }

public:
  constexpr
  unsafe_any()
    : m_manager{nullptr}
  { }

  constexpr
  unsafe_any(unsafe_any const &other)
  {
    if (!other.has_value())
      m_manager = nullptr;
    else
    {
      manager_arg_t arg;
      arg.any = this;
      other.m_manager(operation::copy, &other, &arg);
    }
  }

  constexpr
  unsafe_any(unsafe_any &&other)
  noexcept
  {
    if (!other.has_value())
      m_manager = nullptr;
    else
    {
      manager_arg_t arg;
      arg.any = this;
      other.m_manager(operation::move, &other, &arg);
    }
  }

  template<typename T, typename ...Args>
  explicit constexpr
  unsafe_any(std::in_place_type_t<T>, Args &&...args)
  requires (
      std::is_same_v<T, std::decay_t<T>>
   && std::constructible_from<T, Args...>)
    : m_manager{nullptr}
  {
    m_emplace<T>(std::forward<Args>(args)...);
  }

  template<typename T, typename U, typename ...Args>
  constexpr
  unsafe_any(
      std::in_place_type_t<T>,
      std::initializer_list<U> ilist,
      Args                &&...args)
  requires (
      std::is_same_v<T, std::decay_t<T>>
   && std::constructible_from<T, std::initializer_list<U>, Args...>)
    : m_manager{nullptr}
  {
    m_emplace<T>(ilist, std::forward<Args>(args)...);
  }

  template<typename T>
  explicit constexpr
  unsafe_any(T &&value)
  requires (
      std::is_same_v<T, std::decay_t<T>>
   && std::is_move_constructible_v<T>)
    : unsafe_any{std::in_place_type<T>, std::forward<T>(value)}
  { }


  constexpr
  ~unsafe_any()
  noexcept
  {
    reset();
  }


  constexpr unsafe_any &
  operator=(unsafe_any const &other)
  {
    if (this == &other)
      return *this;

    *this = unsafe_any{other};
    return *this;
  }

  constexpr unsafe_any &
  operator=(unsafe_any &&other)
  noexcept
  {
    if (this == &other)
      return *this;

    reset();
    if (!other.has_value())
      return *this;

    manager_arg_t arg;
    arg.any = this;
    other.m_manager(operation::move, &other, &arg);

    return *this;
  }

  template<typename T>
  constexpr unsafe_any &
  operator=(T &&value)
  requires (
      std::is_same_v<T, std::decay_t<T>>
   && std::is_move_constructible_v<T>)
  {
    *this = unsafe_any{std::forward<T>(value)};
    return *this;
  }


  constexpr void
  swap(unsafe_any &other)
  noexcept
  {
    if (this == &other)
      return;

    if (!has_value() && !other.has_value())
      return;

    if (has_value() && other.has_value())
    {
      unsafe_any tmp;
      manager_arg_t arg;

      arg.any = &tmp;
      other.m_manager(operation::move, &other, &arg);
      arg.any = &other;
      m_manager      (operation::move, this  , &arg);
      arg.any = this;
      tmp.m_manager  (operation::move, &tmp  , &arg);
    }
    else
    {
      unsafe_any       *empty = !has_value() ? this : &other;
      unsafe_any const *full  = !has_value() ? &other : this;
      manager_arg_t arg;

      arg.any = empty;
      full->m_manager(operation::move, full, &arg);
    }
  }

  friend constexpr void
  swap(unsafe_any &lhs, unsafe_any &rhs)
  noexcept(noexcept(lhs.swap(rhs)))
  {
    lhs.swap(rhs);
  }



  [[nodiscard]]
  constexpr bool
  has_value() const
  noexcept
  {
    return m_manager != nullptr;
  }



  constexpr void
  reset()
  noexcept
  {
    if (!has_value())
      return;

    m_manager(operation::destroy, this, nullptr);
    m_manager = nullptr;
  }


  template<typename T, typename ...Args>
  constexpr T &
  emplace(Args &&...args)
  requires (
      std::is_same_v<T, std::decay_t<T>>
   && std::constructible_from<T, Args...>)
  {
    reset();
    m_emplace<T>(std::forward<Args>(args)...);
    return m_cast<T>();
  }

  template<typename T, typename U, typename ...Args>
  constexpr T &
  emplace(std::initializer_list<U> ilist, Args &&...args)
  requires (
      std::is_same_v<T, std::decay_t<T>>
   && std::constructible_from<T, Args...>)
  {
    reset();
    m_emplace<T>(ilist, std::forward<Args>(args)...);
    return m_cast<T>();
  }



  template<typename T>
  friend constexpr T const *
  unsafe_any_cast(unsafe_any const &any)
  noexcept
  requires (std::is_same_v<T, std::decay_t<T>>);

};


template<typename T, typename ...Args>
[[nodiscard]]
constexpr unsafe_any
make_unsafe_any(Args &&...args)
requires (
      std::is_same_v<T, std::decay_t<T>>
   && std::constructible_from<T, Args...>)
{
  return unsafe_any{std::in_place_type<T>, std::forward<Args>(args)...};
}


template<typename T>
constexpr T const *
unsafe_any_cast(unsafe_any const &any)
noexcept
requires (std::is_same_v<T, std::decay_t<T>>)
{
  if (!any.has_value())
    return nullptr;

  if (&unsafe_any::s_manage<T> != any.m_manager)
    return nullptr;

  return any.m_cast<T>();
}

template<typename T>
constexpr T *
unsafe_any_cast(unsafe_any &any)
noexcept
requires (std::is_same_v<T, std::decay_t<T>>)
{
  return const_cast<T *>(unsafe_any_cast<T>(std::as_const(any)));
}




}


#endif // HEIM_UNSAFE_ANY_HPP
