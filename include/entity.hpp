#ifndef HEIM_ENTITY_HPP
#define HEIM_ENTITY_HPP

#include <climits>
#include <cstddef>
#include <limits>
#include <type_traits>

namespace heim
{
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
  static constexpr entity_type
  index_mask      = std::numeric_limits<entity_type>::max() >> generation_bits;

  static constexpr entity_type
  generation_mask = std::numeric_limits<entity_type>::max() << index_bits;


  static constexpr entity_type
  max_index      = index_mask;

  static constexpr entity_type
  max_generation = generation_mask >> index_bits;

public:
  static constexpr entity_type
  index(entity_type const e)
  noexcept
  {
    return e & index_mask;
  }


  static constexpr entity_type
  generation(entity_type const e)
  noexcept
  {
    return e >> index_bits;
  }


  static constexpr entity_type
  from(
      entity_type const generation,
      entity_type const index)
  noexcept
  {
    return (generation << index_bits) | (index & index_mask);
  }



};


}


#endif // HEIM_ENTITY_HPP
