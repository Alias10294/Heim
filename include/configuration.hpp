#ifndef HEIM_CONFIGURATION_HPP
#define HEIM_CONFIGURATION_HPP

#include <cstddef>
#include <memory>
#include "lib/utility.hpp"

namespace heim
{
template<typename = redefine_tag>
struct default_allocator
{
  template<typename T>
  using type_for = std::allocator<T>;

};

/*
Exemple de redéfinition:

template<>
struct default_allocator<redefine_tag>
{
  template<typename T>
  using type_for = std::pmr::polymorphic_allocator<T>;

};
*/

template<typename T>
struct allocator_for
{
  using type
  = default_allocator<>
      ::template type_for<T>;

};

template<typename T>
using allocator_for_t
= allocator_for<T>::type;


/*!
 * @brief The default page size for each internal page of the component type
 *   @code C@endcode 's container.
 *
 * @tparam C The component whose default page size to get.
 */
template<typename C>
struct default_page_size
{
  static constexpr std::size_t
  value = 4096;

};

template<typename C>
inline constexpr std::size_t
default_page_size_v = default_page_size<C>::value;


/*!
 * @brief The properties in Heim for the component type @code C@endcode.
 *
 * @tparam C The component type whose traits to get.
 */
template<typename C>
struct component_traits
{
  //! @brief The component type.
  using component
  = C;

  //! @brief The page size for the component type's container.
  static constexpr std::size_t
  page_size = default_page_size_v<C>;

  //! @brief The allocator type for the component type.
  using allocator
  = allocator_for_t<C>;

};


}


#endif // HEIM_CONFIGURATION_HPP
