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
 * @note This type only adheres to what the std::allocator_traits type demands of an allocator
 *   type. Proper complete allocator types should try to adhere to the allocator requirements
 *   formulated on cppreference.com.
 */
template<typename T>
struct is_an_allocator;

template<typename T>
inline constexpr
bool
is_an_allocator_v
= is_an_allocator<T>::value;



/*!
 * @brief Determines whether a type passes as an allocator for a certain value type.
 *
 * @tparam A The type to determine the allocator characteristics of.
 * @tparam T The value type to determine for.
 */
template<
    typename A,
    typename T>
struct is_an_allocator_for;

template<
    typename A,
    typename T>
inline constexpr
bool
is_an_allocator_for_v
= is_an_allocator_for<A, T>::value;



template<typename T>
struct is_an_allocator
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
    typename A,
    typename T>
struct is_an_allocator_for
  : bool_constant<
        is_an_allocator_v<A>
     && std::is_same_v<typename A::value_type, T>>
{ };


}

#endif // HEIM_ALLOCATOR_HPP