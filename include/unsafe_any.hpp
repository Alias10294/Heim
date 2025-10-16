#ifndef HEIM_UNSAFE_ANY_HPP
#define HEIM_UNSAFE_ANY_HPP

#include <concepts>
#include <type_traits>
#include <utility>

namespace heim
{
class unsafe_any
{
private:
  using destroy_func_t = void  (void *);
  using copy_func_t    = void *(void const *);

private:
  void *m_value;

  destroy_func_t *m_destroy_f;
  copy_func_t    *m_copy_f;

private:
  template<typename T>
  constexpr static destroy_func_t *
  s_make_destroy_func()
  noexcept
  requires (std::is_same_v<T, std::remove_cvref_t<T>>)
  {
    return
        [](void *ptr)
        -> void
        {
          if constexpr (std::is_same_v<T, std::nullptr_t>)
            return;

          if constexpr (std::is_array_v<T>)
            delete[] static_cast<T *>(ptr);
          else
            delete   static_cast<T *>(ptr);
        };
  }


  template<typename T>
  constexpr static copy_func_t *
  s_make_copy_func()
  noexcept
  requires (std::is_same_v<T, std::remove_cvref_t<T>>)
  {
    return
        [](void const * const ptr)
        -> void *
        {
          if constexpr (
              std::is_same_v<T, std::nullptr_t>
          || !std::is_copy_constructible_v<T>)
            return nullptr;
          else
            return static_cast<void *>(new T{*static_cast<T const *>(ptr)});
        };
  }


  constexpr void
  m_copy(unsafe_any const &other)
  noexcept(noexcept(other.m_copy_f(other.m_value)))
  {
    void *value = other.m_copy_f(other.m_value);

    m_value     = value;

    if (m_value)
    {
      m_destroy_f = other.m_destroy_f;
      m_copy_f    = other.m_copy_f;
    }
    else
    {
      m_destroy_f = s_make_destroy_func<std::nullptr_t>();
      m_copy_f    = s_make_copy_func   <std::nullptr_t>();
    }
  }

public:
  constexpr
  unsafe_any()
    : m_value    {nullptr},
      m_destroy_f{s_make_destroy_func<std::nullptr_t>()},
      m_copy_f   {s_make_copy_func   <std::nullptr_t>()}
  { }

  constexpr
  unsafe_any(unsafe_any const &other)
    : unsafe_any{}
  {
    m_copy(other);
  }

  constexpr
  unsafe_any(unsafe_any &&other)
  noexcept
    : m_value    {other.m_value},
      m_destroy_f{other.m_destroy_f},
      m_copy_f   {other.m_copy_f}
  {
    other.m_value     = nullptr;
    other.m_destroy_f = s_make_destroy_func<std::nullptr_t>();
    other.m_copy_f    = s_make_copy_func   <std::nullptr_t>();
  }

  template<typename T, typename ...Args>
  constexpr
  unsafe_any(std::in_place_type_t<T>, Args &&...args)
  requires (
      std::is_same_v<T, std::remove_cvref_t<T>>
   && std::constructible_from<T, Args...>)
    : m_value    {new T{std::forward<Args>(args)...}},
      m_destroy_f{s_make_destroy_func<T>()},
      m_copy_f   {s_make_copy_func   <T>()}
  { }

  template<typename T>
  constexpr
  unsafe_any(T &&value)
  requires (
      std::is_same_v<T, std::remove_cvref_t<T>>
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

    unsafe_any copy{other};
    swap(copy);

    return *this;
  }

  constexpr unsafe_any &
  operator=(unsafe_any &&other)
  noexcept
  {
    if (this == &other)
      return *this;

    reset();
    m_value     = other.m_value;
    m_destroy_f = other.m_destroy_f;
    m_copy_f    = other.m_copy_f;

    other.m_value     = nullptr;
    other.m_destroy_f = s_make_destroy_func<std::nullptr_t>();
    other.m_copy_f    = s_make_copy_func   <std::nullptr_t>();

    return *this;
  }

  template<typename T>
  constexpr unsafe_any &
  operator=(T &&value)
  requires (
      std::is_same_v<T, std::remove_cvref_t<T>>
   && std::is_move_constructible_v<T>)
  {
    reset();
    m_value     = new T{std::forward<T>(value)};
    m_destroy_f = s_make_destroy_func<T>();
    m_copy_f    = s_make_copy_func   <T>();

    return *this;
  }


  constexpr void
  swap(unsafe_any &other)
  noexcept
  {
    using std::swap;

    swap(m_value    , other.m_value);
    swap(m_destroy_f, other.m_destroy_f);
    swap(m_copy_f   , other.m_copy_f);
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
    return m_value != nullptr;
  }


  template<typename T>
  [[nodiscard]]
  constexpr T &
  get()
  noexcept
  {
    return *static_cast<T *>(m_value);
  }

  template<typename T>
  [[nodiscard]]
  constexpr T const &
  get() const
  noexcept
  {
    return *static_cast<T const *>(m_value);
  }



  constexpr void
  reset()
  noexcept
  {
    if (has_value())
      m_destroy_f(m_value);

    m_value     = nullptr;
    m_destroy_f = s_make_destroy_func<std::nullptr_t>();
    m_copy_f    = s_make_copy_func   <std::nullptr_t>();
  }


  template<typename T, typename ...Args>
  constexpr T &
  emplace(std::in_place_type_t<T>, Args &&...args)
  requires (
      std::is_same_v<T, std::remove_cvref_t<T>>
   && std::constructible_from<T, Args...>)
  {
    reset();
    m_value     = new T{std::forward<Args>(args)...};
    m_destroy_f = s_make_destroy_func<T>();
    m_copy_f    = s_make_copy_func   <T>();

    return get<T>();
  }

};

template<typename T, typename ...Args>
[[nodiscard]]
constexpr unsafe_any
make_any(Args &&...args)
{
  return unsafe_any{std::in_place_type<T>, std::forward<Args>(args)...};
}


}


#endif // HEIM_UNSAFE_ANY_HPP
