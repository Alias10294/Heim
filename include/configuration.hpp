#ifndef HEIM_CONFIGURATION_HPP
#define HEIM_CONFIGURATION_HPP

#include <cstddef>
#include <memory>

namespace heim
{
/*!
 * @brief The default allocator to use in Heim for @code T@endcode.
 *
 * @tparam T The type whose default allocator to get.
 */
template<typename T>
struct default_allocator
{
  using type
  = std::allocator<T>;

};

template<typename T>
using default_allocator_t
= typename default_allocator<T>::type;


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
  using component_type
  = C;

  //! @brief The page size for the component type's container.
  static constexpr std::size_t
  page_size = default_page_size_v<C>;

  //! @brief The allocator type for the component type.
  using allocator_type
  = default_allocator_t<C>;

};


}


#endif // HEIM_CONFIGURATION_HPP
