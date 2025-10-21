#ifndef HEIM_UNSAFE_ANY_HPP
#define HEIM_UNSAFE_ANY_HPP

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <new>
#include <type_traits>
#include <utility>

namespace heim
{
/*!
 * @brief A container for single values of any type.
 *
 * @tparam BufferSize  The size of the internal buffer for the small-buffer
 *   optimization.
 * @tparam BufferAlign The alignment of the internal buffer for the
 *   small-buffer optimization.
 *
 * @details Very similar to std::any, this container has the particularity of
 *   not being type-safe. It is intended to be used in situations where type
 *   identification does not come from the usual std::type_info.
 *   It also implements a small-buffer optimization, for which the buffer is
 *   customizable both in size and alignment.
 */
template<
    std::size_t BufferSize  = sizeof (void *),
    std::size_t BufferAlign = alignof(std::max_align_t)>
class generic_unsafe_any
{
private:
  static_assert(
      BufferSize >= sizeof(void *),
      "heim::generic_unsafe_any<BufferSize, BufferAlign>: "
          "BufferSize >= sizeof(void *).");
  static_assert(
      BufferAlign >= alignof(void *),
      "heim::generic_unsafe_any<BufferSize, BufferAlign>: "
          "BufferAlign >= alignof(void*).");
  static_assert(
      (BufferAlign & (BufferAlign - 1)) == 0,
      "heim::generic_unsafe_any<BufferSize, BufferAlign>: "
          "(BufferAlign & BufferAlign - 1) == 0.");

public:
  constexpr static std::size_t buffer_size  = BufferSize;
  constexpr static std::size_t buffer_align = BufferAlign;

private:
  //! @cond INTERNAL

  /*!
   * @brief The type used to contain the value, either internally or through
   *   allocation if needed.
   */
  union storage_t
  {
    using buffer_t = std::byte[buffer_size];


    /*!
     * @brief Default-constructs the storage.
     */
    constexpr
    storage_t()
      : value{nullptr}
    { }

    /*!
     * @brief Deleted copy constructor, preventing trivial copies of the type.
     */
    constexpr
    storage_t(storage_t const &other)
    = delete;

    /*!
     * @brief Deleted copy assignment operator, preventing trivial copies of
     *   the type.
     */
    constexpr storage_t &
    operator=(storage_t const &other)
    = delete;


    void                          *value;
    alignas(buffer_align) buffer_t buffer;

  };


  /*!
   * @brief The class used to express what operations the manager can perform
   *   on the contained value.
   */
  enum class operation : std::uint8_t
  {
    copy,
    destroy,
    move
  };

  using manager_t
  = void(*)(
      operation const,
      generic_unsafe_any const *,
      generic_unsafe_any *);

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


  /*!
   * @brief The internal method used to perform given operations on the
   *   contained value.
   *
   * @tparam T The type of the contained value.
   * @param op    The kind of operation to perform on the contained value.
   * @param self  The pointer to the container that holds the value.
   * @param other The pointer to the optional other container on which the
   *   operation has to be performed.
   * @pre @code std::is_same_v<T, std::decay_t<T>>@endcode.
   */
  template<typename T>
  constexpr static void
  s_manage(
      operation const           op,
      generic_unsafe_any const *self,
      generic_unsafe_any       *other)
  requires (std::is_same_v<T, std::decay_t<T>>)
  {
    T const *ptr = nullptr;
    if constexpr (to_buffer_v<T>)
      ptr = std::launder(reinterpret_cast<T const *>(&self->m_storage.buffer));
    else
      ptr = static_cast<T const *>(self->m_storage.value);

    switch (op)
    {
    case operation::copy:
      if constexpr (std::is_copy_constructible_v<T>)
      {
        if constexpr (to_buffer_v<T>)
        {
          ::new(&other->m_storage.buffer) T{*ptr};
          other->m_manager = self->m_manager;
        }
        else
        {
          other->m_storage.value = new T{*ptr};
          other->m_manager       = self->m_manager;
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
        ::new(&other->m_storage.buffer) T{std::move(*const_cast<T *>(ptr))};
        other->m_manager = self->m_manager;

        ptr->~T();
        const_cast<generic_unsafe_any *>(self)->m_manager = nullptr;
      }
      else
      {
        other->m_storage.value = self->m_storage.value;
        other->m_manager       = self->m_manager;

        const_cast<generic_unsafe_any *>(self)->m_storage.value = nullptr;
        const_cast<generic_unsafe_any *>(self)->m_manager       = nullptr;
      }
      break;
    }
  }


  /*!
   * @brief Constructs the value to be contained using @code args@endcode.
   *
   * @tparam T    The type of the constructed value.
   * @tparam Args The type of the arguments to construct the value.
   * @param args The arguments to construct the value.
   * @pre - @code std::is_same_v<T, std::decay_t<T>>@endcode.
   * @pre - @code std::constructible_from<T, Args &&...>@endcode.
   */
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


  /*!
   * @brief Casts the contained value to the given templated type, without any
   *   type-safety exception or guarantee.
   *
   * @tparam T The type to cast the contained value to.
   * @returns The cast pointer to the value.
   * @pre @code std::is_same_v<T, std::decay_t<T>>@endcode.
   */
  template<typename T>
  [[nodiscard]]
  constexpr T *
  m_cast()
  noexcept
  requires (std::is_same_v<T, std::decay_t<T>>)
  {
    if constexpr (to_buffer_v<T>)
      return reinterpret_cast<T *>(m_storage.buffer);
    else
      return static_cast<T *>(m_storage.value);
  }

  /*!
   * @brief Casts the const contained value to the given templated type,
   *   without any type-safety exception or guarantee.
   *
   * @tparam T The type to cast the contained value to.
   * @returns The cast pointer to the const contained value.
   * @pre @code std::is_same_v<T, std::decay_t<T>>@endcode.
   */
  template<typename T>
  [[nodiscard]]
  constexpr T const *
  m_cast() const
  noexcept
  requires (std::is_same_v<T, std::decay_t<T>>)
  {
    if constexpr (to_buffer_v<T>)
      return reinterpret_cast<T const *>(&m_storage.buffer);
    else
      return static_cast<T const *>(m_storage.value);
  }

  //! @endcond

public:
  /*!
   * @brief Default-constructs the container.
   */
  constexpr
  generic_unsafe_any()
    : m_manager{nullptr}
  { }

  /*!
   * @brief Constructs @c *this to be a copy of @code other@endcode.
   *
   * @param other The container to copy.
   */
  constexpr
  generic_unsafe_any(generic_unsafe_any const &other)
    : generic_unsafe_any{}
  {
    if (!other.has_value())
      return;

    other.m_manager(operation::copy, &other, this);
  }

  /*!
   * @brief Constructs @c *this to be the moved @code other@endcode.
   *
   * @param other The moved container.
   */
  constexpr
  generic_unsafe_any(generic_unsafe_any &&other)
  noexcept
    : generic_unsafe_any{}
  {
    if (!other.has_value())
      return;

    other.m_manager(operation::move, &other, this);
  }

  /*!
   * @brief Constructs @c *this along its value using @code args@endcode.
   *
   * @tparam T    The type of the object to contain.
   * @tparam Args The type of the arguments to construct the value.
   * @param args The arguments to construct the value.
   * @pre - @code std::is_same_v<T, std::decay_t<T>>@endcode.
   * @pre - @code std::constructible_from<T, Args...>@endcode.
   */
  template<typename T, typename ...Args>
  explicit constexpr
  generic_unsafe_any(std::in_place_type_t<T>, Args &&...args)
  requires (
      std::is_same_v<T, std::decay_t<T>>
   && std::constructible_from<T, Args...>)
    : generic_unsafe_any{}
  {
    m_emplace<T>(std::forward<Args>(args)...);
  }

  /*!
   * @brief Constructs @c *this along its contained value with
   *   @code ilist@endcode and @code args@endcode.
   *
   * @tparam T    The type of the object to contain.
   * @tparam U    The type of the elements in the initializer list to construct
   *   the value.
   * @tparam Args The type of the arguments to construct the value.
   * @param ilist The initializer list to construct the value.
   * @param args  The arguments to construct the value.
   * @pre - @code std::is_same_v<T, std::decay_t<T>>@endcode.
   * @pre - @code std::constructible_from<T, std::initializer_list<U>, Args...>
   *   @endcode.
   */
  template<typename T, typename U, typename ...Args>
  constexpr
  generic_unsafe_any(
      std::in_place_type_t<T>,
      std::initializer_list<U> ilist,
      Args                &&...args)
  requires (
      std::is_same_v<T, std::decay_t<T>>
   && std::constructible_from<T, std::initializer_list<U>, Args...>)
    : generic_unsafe_any{}
  {
    m_emplace<T>(ilist, std::forward<Args>(args)...);
  }

  /*!
   * @brief Constructs @c *this and moves @code value@endcode to be the
   *   contained value.
   *
   * @tparam T The type of the value to contain.
   * @param value The value to contain.
   * @pre - @code std::is_same_v<T, std::decay_t<T>>@endcode.
   * @pre - @code std::is_move_constructible_v<T>@endcode.
   */
  template<typename T>
  explicit constexpr
  generic_unsafe_any(T &&value)
  requires (
      std::is_same_v<T, std::decay_t<T>>
   && std::is_move_constructible_v<T>)
    : generic_unsafe_any{std::in_place_type<T>, std::forward<T>(value)}
  { }


  /*!
   * @brief Destroys the container.
   */
  constexpr
  ~generic_unsafe_any()
  noexcept
  {
    reset();
  }


  /*!
   * @brief Assigns @c *this to be a copy of @code other@endcode.
   *
   * @param other The container to copy.
   * @returns @c *this .
   */
  constexpr generic_unsafe_any &
  operator=(generic_unsafe_any const &other)
  {
    if (this == &other)
      return *this;

    *this = generic_unsafe_any{other};
    return *this;
  }

  /*!
   * @brief Assigns @c *this to be the moved @code other@endcode.
   *
   * @param other The moved container.
   * @returns @c *this .
   */
  constexpr generic_unsafe_any &
  operator=(generic_unsafe_any &&other)
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

  /*!
   * @brief Assigns @code value@endcode to be the contained value of @c *this .
   *
   * @tparam T The type of the value to contain.
   * @param value The value to contain.
   * @returns @c *this .
   * @pre - @code std::is_same_v<T, std::decay_t<T>>@endcode.
   * @pre - @code std::is_move_constructible_v<T>@endcode.
   */
  template<typename T>
  constexpr generic_unsafe_any &
  operator=(T &&value)
  requires (
      std::is_same_v<T, std::decay_t<T>>
   && std::is_move_constructible_v<T>)
  {
    *this = generic_unsafe_any{std::forward<T>(value)};
    return *this;
  }


  /*!
   * @brief Swaps the contents of @c *this and @code other@endcode.
   *
   * @param other The other container whose contents to swap.
   */
  constexpr void
  swap(generic_unsafe_any &other)
  noexcept
  {
    if (this == &other)
      return;

    if (!has_value() && !other.has_value())
      return;

    if (has_value() && other.has_value())
    {
      generic_unsafe_any tmp;

      other.m_manager(operation::move, &other, &tmp);
      m_manager      (operation::move, this  , &other);
      tmp.m_manager  (operation::move, &tmp  , this);
    }
    else
    {
      generic_unsafe_any       *empty = !has_value() ? this : &other;
      generic_unsafe_any const *full  = !has_value() ? &other : this;

      full->m_manager(operation::move, full, empty);
    }
  }

  /*!
   * @brief Swaps the contents of @code lhs@endcode and @code rhs@endcode.
   *
   * @param lhs The first  container whose contents to swap.
   * @param rhs The second container whose contents to swap.
   */
  friend constexpr void
  swap(generic_unsafe_any &lhs, generic_unsafe_any &rhs)
  noexcept(noexcept(lhs.swap(rhs)))
  {
    lhs.swap(rhs);
  }



  /*!
   * @brief Checks whether @c *this contains a value.
   *
   * @returns @ true if @c *this contains a value, @c false otherwise.
   */
  [[nodiscard]]
  constexpr bool
  has_value() const
  noexcept
  {
    return m_manager != nullptr;
  }



  /*!
   * @brief Destroys the contained value, leaving the container empty.
   */
  constexpr void
  reset()
  noexcept
  {
    if (!has_value())
      return;

    m_manager(operation::destroy, this, nullptr);
    m_manager = nullptr;
  }


  /*!
   * @brief Emplaces a new value, destroying the currently contained value if
   *   any.
   *
   * @tparam T    The type of the new value to contain.
   * @tparam Args The type of the arguments to construct the new value.
   * @param args The arguments to construct the new value.
   * @returns A reference to the new value.
   * @pre - @code std::is_same_v<T, std::decay_t<T>>@endcode.
   * @pre - @code std::constructible_from<T, Args &&...>@endcode.
   */
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

  /*!
   * @brief Emplaces a new value, destroying the currently contained value if
   *   any.
   *
   * @tparam T    The type of the new value to contain.
   * @tparam U    The type of the elements in the initializer list to construct
   *   the new value.
   * @tparam Args The type of the arguments to construct the new value.
   * @param ilist The initializer list to construct the new value.
   * @param args  The arguments to construct the new value.
   * @returns A reference to the new value.
   * @pre - @code std::is_same_v<T, std::decay_t<T>>@endcode.
   * @pre - @code std::constructible_from<T, std::initializer_list<U>, Args...>
   *   @endcode.
   */
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



  /*!
   * @brief Performs access to the contained value without any type-safety
   *   mechanism.
   *
   * @tparam T     The supposed type of the value to access.
   * @tparam Size  The size of the internal buffer of the unsafe_any for the
   *   small-buffer optimization.
   * @tparam Align The alignment of the internal buffer of the unsafe_any for
   *   the small-buffer optimization.
   * @param any The container to access the value from.
   * @returns The cast pointer to the contained value.
   * @pre @code std::is_same_v<T, std::decay_t<T>>@endcode.
   */
  template<typename T, std::size_t Size, std::size_t Align>
  friend constexpr T *
  unsafe_any_cast(generic_unsafe_any<Size, Align> &any)
  noexcept
  requires (std::is_same_v<T, std::decay_t<T>>);

  /*!
   * @brief Performs access to the const contained value without any
   *   type-safety mechanism.
   *
   * @tparam T     The supposed type of the const value to access.
   * @tparam Size  The size of the internal buffer of the unsafe_any for the
   *   small-buffer optimization.
   * @tparam Align The alignment of the internal buffer of the unsafe_any for
   *   the small-buffer optimization.
   * @param any The container to access the const value from.
   * @returns The cast pointer to the const contained value.
   * @pre @code std::is_same_v<T, std::decay_t<T>>@endcode.
   */
  template<typename T, std::size_t Size, std::size_t Align>
  friend constexpr T const *
  unsafe_any_cast(generic_unsafe_any<Size, Align> const &any)
  noexcept
  requires (std::is_same_v<T, std::decay_t<T>>);

};


/*!
 * @brief The default generic_unsafe_any specialization.
 */
using unsafe_any = generic_unsafe_any<>;


/*!
 * @brief Returns a new unsafe_any containing a value constructed using
 *   @code args@endcode.
 *
 * @tparam T           The type of the value for the unsafe_any to contain.
 * @tparam BufferSize  The size of the internal buffer of the unsafe_any
 *   for the small-buffer optimization.
 * @tparam BufferAlign The alignment of the internal buffer of the unsafe_any
 *   for the small-buffer optimization.
 * @tparam Args        The type of the arguments to construct the value for the
 *   object to contain.
 * @param args The arguments to construct the value for the object to contain.
 * @pre - @code std::is_same_v<T, std::decay_t<T>>@endcode.
 * @pre - @code std::constructible_from<T, Args...>@endcode.
 */
template<
    typename    T,
    std::size_t BufferSize,
    std::size_t BufferAlign,
    typename ...Args>
[[nodiscard]]
constexpr generic_unsafe_any<BufferSize, BufferAlign>
make_unsafe_any(Args &&...args)
requires (
      std::is_same_v<T, std::decay_t<T>>
   && std::constructible_from<T, Args...>)
{
  return generic_unsafe_any<BufferSize, BufferAlign>{
      std::in_place_type<T>,
      std::forward<Args>(args)...};
}

/*!
 * @brief Returns a new unsafe_any containing a value constructed using
 *   @code args@endcode.
 *
 * @tparam T           The type of the value for the unsafe_any to contain.
 * @tparam BufferSize  The size of the internal buffer of the unsafe_any
 *   for the small-buffer optimization.
 * @tparam BufferAlign The alignment of the internal buffer of the unsafe_any
 *   for the small-buffer optimization.
 * @tparam U           The type of the elements in the initializer list to
 *   construct the value for the object to contain.
 * @tparam Args        The type of the arguments to construct the value for the
 *   object to contain.
 * @param ilist The initializer list to construct the value for the object to
 *   contain.
 * @param args  The arguments to construct the value for the object to contain.
 * @pre - @code std::is_same_v<T, std::decay_t<T>>@endcode.
 * @pre - @code std::constructible_from<T, Args...>@endcode.
 */
template<
    typename    T,
    std::size_t BufferSize,
    std::size_t BufferAlign,
    typename    U,
    typename ...Args>
[[nodiscard]]
constexpr generic_unsafe_any<BufferSize, BufferAlign>
make_unsafe_any(std::initializer_list<U> ilist, Args &&...args)
requires (
      std::is_same_v<T, std::decay_t<T>>
   && std::constructible_from<T, std::initializer_list<U>, Args...>)
{
  return generic_unsafe_any<BufferSize, BufferAlign>{
      std::in_place_type<T>,
      ilist,
      std::forward<Args>(args)...};
}


template<typename T, std::size_t BufferSize, std::size_t BufferAlign>
constexpr T *
unsafe_any_cast(generic_unsafe_any<BufferSize, BufferAlign> &any)
noexcept
requires (std::is_same_v<T, std::decay_t<T>>)
{
  return any.template m_cast<T>();
}

template<typename T, std::size_t BufferSize, std::size_t BufferAlign>
constexpr T const *
unsafe_any_cast(generic_unsafe_any<BufferSize, BufferAlign> const &any)
noexcept
requires (std::is_same_v<T, std::decay_t<T>>)
{
  return any.template m_cast<T>();
}


}


#endif // HEIM_UNSAFE_ANY_HPP
