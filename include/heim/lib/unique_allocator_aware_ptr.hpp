#ifndef HEIM_LIB_UNIQUE_ALLOCATOR_AWARE_PTR_HPP
#define HEIM_LIB_UNIQUE_ALLOCATOR_AWARE_PTR_HPP

#include <cstddef>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>
#include "utility.hpp"

namespace heim
{
/*!
 * \brief
 *   A function object that uses an allocator to deallocate and destroy a single object.
 *
 * \details
 *   Is designed to be used as an allocator-aware alternative to \c std::default_delete ,
 *   the default deletion policy of \c std::unique_ptr .
 */
template<
    typename T,
    typename Allocator>
requires allocator_for<Allocator, T>
class allocator_aware_deleter
{
public:
  using value_type     = T;
  using allocator_type = Allocator;

  using pointer
  = typename std::allocator_traits<allocator_type>::pointer;

private:
  [[no_unique_address]]
  allocator_type m_allocator;

private:
  static constexpr
  bool
  s_noexcept_default_construct()
  noexcept
  { return std::is_nothrow_default_constructible_v<allocator_type>; }

public:
  constexpr
  allocator_aware_deleter()
  noexcept(s_noexcept_default_construct())
  requires(std::default_initializable<allocator_type>)
  = default;

  constexpr
  allocator_aware_deleter(allocator_aware_deleter const &)
  = default;

  constexpr
  allocator_aware_deleter(allocator_aware_deleter &&)
  = default;

  explicit constexpr
  allocator_aware_deleter(allocator_type const &alloc)
  noexcept
    : m_allocator{alloc}
  { }

  explicit constexpr
  allocator_aware_deleter(allocator_type &&alloc)
  noexcept
    : m_allocator{std::move(alloc)}
  { }

  template<typename Alloc>
  requires (std::constructible_from<allocator_type, Alloc &&>)
  explicit constexpr
  allocator_aware_deleter(Alloc &&alloc)
  noexcept
    : m_allocator{std::forward<Alloc>(alloc)}
  { }

  constexpr
  ~allocator_aware_deleter()
  = default;

  constexpr
  allocator_aware_deleter &
  operator=(allocator_aware_deleter const &)
  = default;

  constexpr
  allocator_aware_deleter &
  operator=(allocator_aware_deleter &&)
  = default;

  constexpr
  void
  operator()(pointer ptr)
  noexcept
  {
    if (ptr == pointer{})
      return;

    std::allocator_traits<allocator_type>::destroy   (m_allocator, std::to_address(ptr));
    std::allocator_traits<allocator_type>::deallocate(m_allocator, ptr, 1);
  }
};

/*!
 * \brief
 *   A function object that uses an allocator to deallocate and destroy an array type object.
 *
 * \details
 *   Is designed to be used as an allocator-aware alternative to \c std::default_delete ,
 *   the default deletion policy of \c std::unique_ptr .
 */
template<
    typename T,
    typename Allocator>
requires (
   !std::is_array_v<T>
 && allocator_for<Allocator, T>)
class allocator_aware_deleter<T[], Allocator>
{
public:
  using value_type     = T;
  using element_type   = T;
  using allocator_type = Allocator;

  using pointer
  = typename std::allocator_traits<allocator_type>::pointer;

private:
  [[no_unique_address]]
  allocator_type m_allocator;
  std::size_t    m_size;

private:
  static constexpr
  bool
  s_noexcept_default_construct()
  noexcept
  { return std::is_nothrow_default_constructible_v<allocator_type>; }

public:
  constexpr
  allocator_aware_deleter()
  noexcept(s_noexcept_default_construct())
  requires(std::default_initializable<allocator_type>)
    : m_allocator{}
    , m_size     {}
  { }

  constexpr
  allocator_aware_deleter(allocator_aware_deleter const &)
  = default;

  constexpr
  allocator_aware_deleter(allocator_aware_deleter &&)
  = default;

  explicit constexpr
  allocator_aware_deleter(allocator_type const &alloc, std::size_t const size = 0)
  noexcept
    : m_allocator{alloc}
    , m_size     {size}
  { }

  explicit constexpr
  allocator_aware_deleter(allocator_type &&alloc, std::size_t const size = 0)
  noexcept
    : m_allocator{std::move(alloc)}
    , m_size     {size}
  { }

  template<typename Alloc>
  requires(std::constructible_from<allocator_type, Alloc &&>)
  explicit constexpr
  allocator_aware_deleter(Alloc &&alloc, std::size_t const size = 0)
  noexcept
    : m_allocator{std::forward<Alloc>(alloc)}
    , m_size     {size}
  { }

  constexpr
  ~allocator_aware_deleter()
  = default;

  constexpr
  allocator_aware_deleter &
  operator=(allocator_aware_deleter const &)
  = default;

  constexpr
  allocator_aware_deleter &
  operator=(allocator_aware_deleter &&)
  = default;

  constexpr
  void
  operator()(pointer ptr)
  noexcept
  {
    if (ptr == pointer{})
      return;

    auto * const first = std::to_address(ptr);
    auto        *last  = first + m_size;

    while (first != last)
      std::allocator_traits<allocator_type>::destroy(m_allocator, --last);

    std::allocator_traits<allocator_type>::deallocate(m_allocator, ptr, m_size);
  }
};


template<
    typename T,
    typename Allocator>
using unique_allocator_aware_ptr
= std::unique_ptr<T, allocator_aware_deleter<T, Allocator>>;


/*!
 * \brief
 *   Allocates and constructs a single object of the specializing type and wraps it in a \c unique_allocator_aware_ptr .
 */
template<
    typename    T,
    typename    Allocator,
    typename ...Args>
requires(!std::is_array_v<T>)
constexpr
unique_allocator_aware_ptr<T, Allocator>
make_unique_allocator_aware(Allocator const &a, Args &&...args)
{
  using allocator_type = Allocator;
  using alloc_traits   = std::allocator_traits<allocator_type>;
  using pointer        = typename alloc_traits::pointer;

  auto    alloc = allocator_type{a};
  pointer ptr   = alloc_traits::allocate(alloc, 1);

  try
  { alloc_traits::construct(alloc, std::to_address(ptr), std::forward<Args>(args)...); }
  catch (...)
  { alloc_traits::deallocate(alloc, ptr, 1); throw; }

  return unique_allocator_aware_ptr<T, Allocator>(
      ptr,
      allocator_aware_deleter<T, Allocator>(std::move(alloc)));
}

/*!
 * \brief
 *   Allocates and constructs an array type object of the specializing type and wraps it in a \c unique_allocator_aware_ptr .
 */
template<
    typename    T,
    typename    Allocator>
requires(std::is_unbounded_array_v<T>)
constexpr
unique_allocator_aware_ptr<T, Allocator>
make_unique_allocator_aware(Allocator const &a, std::size_t size)
{
  using allocator_type = Allocator;
  using alloc_traits   = std::allocator_traits<allocator_type>;
  using pointer        = typename alloc_traits::pointer;

  auto alloc = allocator_type{a};

  if (size == 0)
  {
    return unique_allocator_aware_ptr<T, Allocator>(
        pointer{},
        allocator_aware_deleter<T, Allocator>(std::move(alloc), 0));
  }

  pointer      ptr   = alloc_traits::allocate(alloc, size);
  auto * const begin = std::to_address(ptr);
  auto * const end   = begin + size;
  auto        *curr  = begin;

  try
  {
    while (curr != end)
    { alloc_traits::construct(alloc, curr); ++curr; }
  }
  catch (...)
  {
    while (curr != begin)
      alloc_traits::destroy(alloc, --curr);

    alloc_traits::deallocate(alloc, ptr, size);
    throw;
  }

  return unique_allocator_aware_ptr<T, Allocator>(
      ptr,
      allocator_aware_deleter<T, Allocator>(std::move(alloc), size));
}

template<
    typename    T,
    typename    Allocator,
    typename ...Args>
requires(std::is_bounded_array_v<T>)
unique_allocator_aware_ptr<T, Allocator>
make_unique_allocator_aware(Allocator const &, Args &&...)
= delete;

/*!
 * \brief
 *   Allocates and default-initializes a single object of the specializing type and wraps it in a
 *   \c unique_allocator_aware_ptr .
 */
template<
    typename T,
    typename Allocator>
requires(!std::is_array_v<T>)
constexpr
unique_allocator_aware_ptr<T, Allocator>
make_unique_allocator_aware_for_overwrite(Allocator const &a)
{
  using allocator_type = Allocator;
  using alloc_traits   = std::allocator_traits<allocator_type>;
  using pointer        = typename alloc_traits::pointer;

  auto    alloc = allocator_type{a};
  pointer ptr   = alloc_traits::allocate(alloc, 1);

  try
  { ::new(static_cast<void *>(std::to_address(ptr))) T; }
  catch (...)
  { alloc_traits::deallocate(alloc, ptr, 1); throw; }

  return unique_allocator_aware_ptr<T, Allocator>(
      ptr,
      allocator_aware_deleter<T, Allocator>(std::move(alloc)));
}

/*!
 * \brief
 *   Allocates and default-initializes an array type object of the specializing type and wraps it in
 *   a \c unique_allocator_aware_ptr .
 */
template<
    typename T,
    typename Allocator>
requires(std::is_unbounded_array_v<T>)
constexpr
unique_allocator_aware_ptr<T, Allocator>
make_unique_allocator_aware_for_overwrite(Allocator const &a, std::size_t size)
{
  using allocator_type = Allocator;
  using alloc_traits   = std::allocator_traits<allocator_type>;
  using pointer        = typename alloc_traits::pointer;

  auto alloc = allocator_type{a};

  if (size == 0)
  {
    return unique_allocator_aware_ptr<T, Allocator>(
        pointer{},
        allocator_aware_deleter<T, Allocator>(std::move(alloc), 0));
  }

  pointer      ptr   = alloc_traits::allocate(alloc, size);
  auto * const begin = std::to_address(ptr);
  auto * const end   = begin + size;
  auto        *curr  = begin;

  try
  {
    while (curr != end)
    { ::new(static_cast<void *>(curr)) std::remove_extent_t<T>; ++curr; }
  }
  catch (...)
  {
    while (curr != begin)
      alloc_traits::destroy(alloc, --curr);

    alloc_traits::deallocate(alloc, ptr, size);
    throw;
  }

  return unique_allocator_aware_ptr<T, Allocator>(
      ptr,
      allocator_aware_deleter<T, Allocator>(std::move(alloc), size));
}

template<
    typename    T,
    typename    Allocator,
    typename ...Args>
requires(std::is_bounded_array_v<T>)
unique_allocator_aware_ptr<T, Allocator>
make_unique_allocator_aware_for_overwrite(Allocator const &, Args &&...)
= delete;


} // namespace heim

#endif // HEIM_LIB_UNIQUE_ALLOCATOR_AWARE_PTR_HPP
