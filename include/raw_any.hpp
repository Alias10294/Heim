#ifndef HEIM_RAW_ANY_HPP
#define HEIM_RAW_ANY_HPP

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <new>
#include <type_traits>
#include <utility>

namespace heim
{
template<
    std::size_t BufferSize  = sizeof (void *),
    std::size_t BufferAlign = alignof(std::max_align_t)>
class generic_raw_any
{
private:
  static_assert(
      BufferSize >= sizeof(void *),
      "heim::generic_raw_any<BufferSize, BufferAlign>: "
          "BufferSize >= sizeof(void *).");
  static_assert(
      BufferAlign >= alignof(void*),
      "heim::generic_raw_any<BufferSize, BufferAlign>: "
          "BufferAlign >= alignof(void*).");
  static_assert(
      (BufferAlign & (BufferAlign - 1)) == 0,
      "heim::generic_raw_any<BufferSize, BufferAlign>: "
          "(BufferAlign & BufferAlign - 1) == 0.");

public:
  constexpr static std::size_t buffer_size  = BufferSize;
  constexpr static std::size_t buffer_align = BufferAlign;

private:
  using buffer_t = std::byte[buffer_size];

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

    void                          *value;
    alignas(buffer_align) buffer_t buffer;

  };


  enum class operation : std::uint8_t
  {
    cast,
    copy,
    destroy,
    move
  };

  using manager_t
  = const void *(*)(
      operation const,
      generic_raw_any const *,
      generic_raw_any *);

private:
  storage_t m_storage;
  manager_t m_manager;

private:
  template<typename T>
  constexpr static bool
  to_buffer_v
  =   sizeof (T) <= buffer_size
   && alignof(T) <= buffer_align
   && std::is_nothrow_move_constructible_v<T>;


  template<typename T>
  constexpr static const void *
  s_manage(operation const op, generic_raw_any const *any, generic_raw_any *arg)
  requires (std::is_same_v<T, std::decay_t<T>>)
  {
    T const *ptr = nullptr;
    if constexpr (to_buffer_v<T>)
      ptr = std::launder(reinterpret_cast<T const *>(&any->m_storage.buffer));
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
          ::new(&arg->m_storage.buffer) T{*ptr};
          arg->m_manager = any->m_manager;
        }
        else
        {
          arg->m_storage.value = new T{*ptr};
          arg->m_manager       = any->m_manager;
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
        ::new(&arg->m_storage.buffer) T{std::move(*const_cast<T *>(ptr))};
        arg->m_manager = any->m_manager;

        ptr->~T();
        const_cast<generic_raw_any *>(any)->m_manager = nullptr;
      }
      else
      {
        arg->m_storage.value = any->m_storage.value;
        arg->m_manager       = any->m_manager;

        const_cast<generic_raw_any *>(any)->m_storage.value = nullptr;
        const_cast<generic_raw_any *>(any)->m_manager       = nullptr;
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
  generic_raw_any()
    : m_manager{nullptr}
  { }

  constexpr
  generic_raw_any(generic_raw_any const &other)
    : generic_raw_any{}
  {
    if (!other.has_value())
      return;

    other.m_manager(operation::copy, &other, this);
  }

  constexpr
  generic_raw_any(generic_raw_any &&other)
  noexcept
    : generic_raw_any{}
  {
    if (!other.has_value())
      return;

    other.m_manager(operation::move, &other, this);
  }

  template<typename T, typename ...Args>
  explicit constexpr
  generic_raw_any(std::in_place_type_t<T>, Args &&...args)
  requires (
      std::is_same_v<T, std::decay_t<T>>
   && std::constructible_from<T, Args...>)
    : generic_raw_any{}
  {
    m_emplace<T>(std::forward<Args>(args)...);
  }

  template<typename T, typename U, typename ...Args>
  constexpr
  generic_raw_any(
      std::in_place_type_t<T>,
      std::initializer_list<U> ilist,
      Args                &&...args)
  requires (
      std::is_same_v<T, std::decay_t<T>>
   && std::constructible_from<T, std::initializer_list<U>, Args...>)
    : generic_raw_any{}
  {
    m_emplace<T>(ilist, std::forward<Args>(args)...);
  }

  template<typename T>
  explicit constexpr
  generic_raw_any(T &&value)
  requires (
      std::is_same_v<T, std::decay_t<T>>
   && std::is_move_constructible_v<T>)
    : generic_raw_any{std::in_place_type<T>, std::forward<T>(value)}
  { }


  constexpr
  ~generic_raw_any()
  noexcept
  {
    reset();
  }


  constexpr generic_raw_any &
  operator=(generic_raw_any const &other)
  {
    if (this == &other)
      return *this;

    *this = generic_raw_any{other};
    return *this;
  }

  constexpr generic_raw_any &
  operator=(generic_raw_any &&other)
  noexcept
  {
    if (this == &other)
      return *this;

    reset();
    if (!other.has_value())
      return *this;

    other.m_manager(operation::move, &other, this);

    return *this;
  }

  template<typename T>
  constexpr generic_raw_any &
  operator=(T &&value)
  requires (
      std::is_same_v<T, std::decay_t<T>>
   && std::is_move_constructible_v<T>)
  {
    *this = generic_raw_any{std::forward<T>(value)};
    return *this;
  }


  constexpr void
  swap(generic_raw_any &other)
  noexcept
  {
    if (this == &other)
      return;

    if (!has_value() && !other.has_value())
      return;

    if (has_value() && other.has_value())
    {
      generic_raw_any tmp;

      other.m_manager(operation::move, &other, &tmp);
      m_manager      (operation::move, this  , &other);
      tmp.m_manager  (operation::move, &tmp  , this);
    }
    else
    {
      generic_raw_any       *empty = !has_value() ? this : &other;
      generic_raw_any const *full  = !has_value() ? &other : this;

      full->m_manager(operation::move, full, empty);
    }
  }

  friend constexpr void
  swap(generic_raw_any &lhs, generic_raw_any &rhs)
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
    return *m_cast<T>();
  }

  template<typename T, typename U, typename ...Args>
  constexpr T &
  emplace(std::initializer_list<U> ilist, Args &&...args)
  requires (
      std::is_same_v<T, std::decay_t<T>>
   && std::constructible_from<T, std::initializer_list<U>, Args...>)
  {
    reset();
    m_emplace<T>(ilist, std::forward<Args>(args)...);
    return *m_cast<T>();
  }



  template<typename T, std::size_t Size, std::size_t Align>
  friend constexpr T const *
  raw_any_cast(generic_raw_any<Size, Align> const &any)
  noexcept
  requires (std::is_same_v<T, std::decay_t<T>>);

  template<typename T, std::size_t Size, std::size_t Align>
  friend constexpr T *
  raw_any_cast(generic_raw_any<Size, Align> &any)
  noexcept
  requires (std::is_same_v<T, std::decay_t<T>>);

};


using raw_any = generic_raw_any<>;


template<
    typename    T,
    std::size_t BufferSize,
    std::size_t BufferAlign,
    typename ...Args>
[[nodiscard]]
constexpr generic_raw_any<BufferSize, BufferAlign>
make_raw_any(Args &&...args)
requires (
      std::is_same_v<T, std::decay_t<T>>
   && std::constructible_from<T, Args...>)
{
  return generic_raw_any<BufferSize, BufferAlign>{
      std::in_place_type<T>,
      std::forward<Args>(args)...};
}

template<typename T, std::size_t BufferSize, std::size_t BufferAlign>
constexpr T const *
raw_any_cast(generic_raw_any<BufferSize, BufferAlign> const &any)
noexcept
requires (std::is_same_v<T, std::decay_t<T>>)
{
  return any.template m_cast<T>();
}

template<typename T, std::size_t BufferSize, std::size_t BufferAlign>
constexpr T *
raw_any_cast(generic_raw_any<BufferSize, BufferAlign> &any)
noexcept
requires (std::is_same_v<T, std::decay_t<T>>)
{
  return any.template m_cast<T>();
}


}


#endif // HEIM_RAW_ANY_HPP
