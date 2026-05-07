#ifndef HEIM_REGISTRY_HPP
#define HEIM_REGISTRY_HPP

#include <type_traits>

namespace heim
{
/*!
 * \brief
 *   Determines whether the specializing is a valid registry type.
 *
 * \note
 *   This type a customization point object, that is supposed to be partially specialized to accomodate
 *   for registry implementations.
 */
template<typename>
struct is_registry
  : std::false_type
{ };

template<typename T>
inline constexpr
bool
is_registry_v
= is_registry<T>::value;

template<typename T>
concept registry
= is_registry<T>::value;

} // namespace heim

#endif // HEIM_REGISTRY_HPP
