#ifndef HEIM_ECS_COMPONENT_HPP
#define HEIM_ECS_COMPONENT_HPP

#include <type_traits>
#include "heim/lib/utility.hpp"

namespace heim
{
/*!
 * \brief
 *   Determines whether the specializing type is a component type in the context of the entity-component-system
 *   (ECS) pattern.
 */
template<typename T>
struct is_component
  : std::bool_constant<
         std ::is_object_v   <T>
     && !heim::is_qualified_v<T>>
{ };

template<typename T>
inline constexpr
bool
is_component_v
= is_component<T>::value;

template<typename T>
concept component
= is_component_v<T>;

} // namespace heim

#endif // HEIM_ECS_COMPONENT_HPP
