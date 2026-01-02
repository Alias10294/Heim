#ifndef HEIM_ALLOCATOR_HPP
#define HEIM_ALLOCATOR_HPP

#include <memory>
#include "utility.hpp"

namespace heim
{
/*!
 * @brief Determines whether T should pass as a type of allocator.
 *
 * @tparam T The type to determine for.
 */
template<
    typename T>
struct is_allocator
  : bool_constant<
        requires(
            T                                           &t,
            typename std::allocator_traits<T>::pointer   ptr,
            typename std::allocator_traits<T>::size_type n)
        {
          typename std::allocator_traits<T>::value_type;
          { std::allocator_traits<T>::allocate  (t, n)      };
          { std::allocator_traits<T>::deallocate(t, ptr, n) };
        }>
{ };

template<
    typename T>
inline constexpr
bool
is_allocator_v
= is_allocator<T>::value();


/*!
 * @brief The interface of properties of allocator types.
 *
 * @tparam Allocator The allocator type.
 */
template<
    typename Allocator>
struct allocator_traits
  : std::allocator_traits<Allocator>
{
  static_assert(is_allocator_v<Allocator>);
};


//! @brief The default allocator type.
template<
    typename T>
using default_allocator
= std::allocator<T>;


} // namespace heim

#endif // HEIM_ALLOCATOR_HPP
