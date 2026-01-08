#ifndef HEIM_ALLOCATOR_HPP
#define HEIM_ALLOCATOR_HPP

#include <memory>
#include <type_traits>
#include "utility.hpp"

namespace heim
{
/*!
 * @brief Determines if a type passes as an allocator type.
 *
 * @tparam T The type to determine for.
 *
 * @note This type only adheres to what the std::allocator_traits type demands
 *   of an allocator type. Proper complete allocator types should try to adhere
 *   to the allocator requirements formulated on cppreference.com.
 */
template<typename T>
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

template<typename T>
inline constexpr
bool
is_allocator_v
= is_allocator<T>::value;

/*!
 * @brief Determines whether a type passes as an allocator for a certain value type.
 *
 * @tparam T The type to determine the allocator characteristics of.
 * @tparam U The value type to determine for.
 */
template<
    typename T,
    typename U>
struct is_allocator_for
  : bool_constant<
        is_allocator_v<T>
     && std::is_same_v<typename T::value_type, U>>
{ };

template<
    typename Allocator,
    typename T>
inline constexpr
bool
is_allocator_for_v
= is_allocator_for<Allocator, T>::value;



/*!
 * @brief An extension of std::allocator_traits that explicitly checks if the given type is an allocator.
 *
 * @tparam Allocator The allocator type.
 */
template<
    typename Allocator>
struct allocator_traits
  : std::allocator_traits<Allocator>
{
  static_assert(
      is_allocator_v<Allocator>,
      "The type must pass as an valid allocator type.");
};



/*!
 * @brief An alias to simplify and standardize the used allocator across the library.
 *
 * @tparam T The value type.
 */
template<typename T>
using allocator
= std::allocator<T>;


} // namespace heim

#endif // HEIM_ALLOCATOR_HPP
