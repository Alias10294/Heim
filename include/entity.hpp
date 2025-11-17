#ifndef HEIM_ENTITY_HPP
#define HEIM_ENTITY_HPP

#include <climits>
#include <cstddef>
#include <limits>
#include <type_traits>
#include "lib/utility.hpp"

namespace heim
{
/*!
 * @brief Whether @code T@endcode is a valid entity type or not.
 *
 * @tparam T The type to check for.
 */
template<typename T>
struct is_entity
{
  static constexpr bool
  value
  =   std::is_same_v<T, std::remove_cvref_t<T>>
   && std::is_integral_v<T>
   && std::is_unsigned_v<T>;

};

template<typename T>
inline constexpr bool
is_entity_v = is_entity<T>::value;

template<typename T>
concept entity
= is_entity_v<T>;


/*!
 * @brief The uniform interface to use entities.
 *
 * @tparam Entity The entity type.
 */
template<typename Entity>
struct entity_traits
{
  static_assert(
      is_entity_v<Entity>,
      "heim::entity_traits<Entity>: "
          "is_entity_v<Entity>;");

public:
  using entity_type = Entity;


  static constexpr std::size_t
  total_bits      = sizeof(entity_type) * CHAR_BIT;

  static constexpr std::size_t
  index_bits      = total_bits / 2;

  static constexpr std::size_t
  generation_bits = total_bits / 2;

private:
  static_assert(
      index_bits + generation_bits == total_bits,
      "heim::entity_traits<Entity>: "
          "index_bits + generation_bits == total_bits;");

public:
  using index_type      = unsigned_integral_for_t<index_bits>;
  using generation_type = unsigned_integral_for_t<generation_bits>;


  static constexpr entity_type
  index_mask      = std::numeric_limits<entity_type>::max() >> generation_bits;

  static constexpr entity_type
  generation_mask = std::numeric_limits<entity_type>::max() << index_bits;


  static constexpr index_type
  max_index      = static_cast<index_type>(index_mask);

  static constexpr generation_type
  max_generation = static_cast<generation_type>(generation_mask >> index_bits);

public:
  /*!
   * @brief The corresponding index of @code e@endcode.
   *
   * @param e The entity to get the index of.
   * @returns The index of @code e@endcode.
   */
  static constexpr index_type
  index(entity_type const e)
  noexcept
  {
    return static_cast<index_type>(e & index_mask);
  }


  /*!
   * @brief The corresponding generation of @code e@endcode.
   *
   * @param e The entity to get the generation of.
   * @returns The generation of @code e@endcode.
   */
  static constexpr generation_type
  generation(entity_type const e)
  noexcept
  {
    return static_cast<generation_type>(e >> index_bits);
  }


  /*!
   * @brief The entity formulated by combining the given generation and index.
   *
   * @param generation The generation to set the entity to.
   * @param index      The index      to set the entity to.
   * @returns The entity with the index @code index@endcode and generation
   *   @code generation@endcode.
   */
  static constexpr entity_type
  from(
      generation_type const generation,
      index_type      const index)
  noexcept
  {
    return (static_cast<entity_type>(generation) << index_bits)
         | (static_cast<entity_type>(index) & index_mask);
  }



};


}


#endif // HEIM_ENTITY_HPP
