#ifndef HEIM_COMPONENT_HPP
#define HEIM_COMPONENT_HPP

#include "utility.hpp"

namespace heim
{
/*!
 * @brief The default size for pages for containers.
 *
 * @note This type is meant, if need be, to be redefined by specializing
 *   default_container_page_size<redefine_tag>.
 */
template<
    typename = redefine_tag>
struct default_container_page_size
{
  /*!
   * @brief The size for pages for containers of component type Component.
   *
   * @tparam Component The component type.
   */
  template<
      typename Component>
  static consteval
  std::size_t
  value_for()
  noexcept
  {
    return std::size_t{1024};
  }
};

/*!
 * @brief The size for pages for containers of component type Component.
 *
 * @tparam Component The component type.
 *
 * @note This type is meant, if need be, to be redefined by specializing using
 *   the specific component type.
 */
template<
    typename Component>
struct container_page_size_for
  : size_constant<
        default_container_page_size<>::value_for<Component>()>
{
  //! @brief The component type.
  using component_type = Component;
};

template<
    typename Component>
inline constexpr
std::size_t
container_page_size_for_v
= container_page_size_for<Component>::value();



/*!
 * @brief The default value for component types to be considered tag or not.
 *
 * @note This type is meant, if need be, to be redefined by specializing
 *   default_is_tag<redefine_tag>.
 */
template<
    typename = redefine_tag>
struct default_is_tag
{
  template<
      typename Component>
  static consteval
  bool
  value_for()
  noexcept
  {
    return std::is_empty_v<Component>;
  }
};

/*!
 * @brief The value for Component to be considered tag or not.
 *
 * @tparam Component The component type.
 *
 * @note This type is meant, if need be, to be redefined by specializing using
 *   the specific component type.
 */
template<
    typename Component>
struct is_tag_for
  : bool_constant<default_is_tag<>::value_for<Component>()>
{
  //! @brief The component type.
  using component_type = Component;
};

template<
    typename Component>
inline constexpr
bool
is_tag_for_v
= is_tag_for<Component>::value();


} // namespace heim

#endif // HEIM_COMPONENT_HPP
