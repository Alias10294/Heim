#ifndef HEIM_LIB_UTILITY_HPP
#define HEIM_LIB_UTILITY_HPP

#include <limits>
#include <memory>
#include <type_traits>

namespace heim
{
/*!
 * \brief
 *   Determines whether the specializing type is qualified.
 *
 * \details
 *   A qualified type is a type that carries a qualifier, that is \code const\endcode, \code volatile\endcode,
 *   \code &\endcode or \code &&\endcode.
 */
template<typename T>
struct is_qualified
  : std::bool_constant<
        std::is_const_v    <T>
     || std::is_volatile_v <T>
     || std::is_reference_v<T>>
{ };

template<typename T>
inline constexpr
bool
is_qualified_v
= is_qualified<T>::value;

template<typename T>
concept qualified
= is_qualified_v<T>;


/*!
 * \brief
 *   Determines whether the specializing type is an allocator.
 *
 * \details
 *   An allocator is a type that fulfills the minimal requirements for \code std::allocator_traits\endcode
 *   to be well-formed.
 */
template<typename T>
struct is_allocator
  : std::bool_constant<
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

template<typename T>
concept allocator
= is_allocator_v<T>;


/*!
 * \brief
 *   Determines whether the first specializing type is an allocator type for the second specializing
 *   type.
 */
template<
    typename T,
    typename U>
struct is_allocator_for
  : std::bool_constant<
        is_allocator_v<T>
     && std::is_same_v<
            typename T::value_type,
            U>>
{ };

template<
    typename T,
    typename U>
inline constexpr
bool
is_allocator_for_v
= is_allocator_for<T, U>::value;

template<
    typename T,
    typename U>
concept allocator_for
= is_allocator_for_v<T, U>;


/*!
 * \brief
 *   Determines the unsigned integral type with at least as many digits as the specializing integer
 *   value.
 */
template<int Digits>
requires(
    Digits >  0
 && Digits <= std::numeric_limits<unsigned long long>::digits)
struct unsigned_integral_for
  : std::type_identity<
        std::conditional_t<
            Digits <= std::numeric_limits<unsigned char>::digits,
            unsigned char,
            std::conditional_t<
                Digits <= std::numeric_limits<unsigned short>::digits,
                unsigned short,
                std::conditional_t<
                    Digits <= std::numeric_limits<unsigned int>::digits,
                    unsigned int,
                    std::conditional_t<
                        Digits <= std::numeric_limits<unsigned long>::digits,
                        unsigned long,
                        unsigned long long>>>>>
{ };

template<int Digits>
using unsigned_integral_for_t
= typename unsigned_integral_for<Digits>::type;

} // namespace heim

#endif // HEIM_LIB_UTILITY_HPP
