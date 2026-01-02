#ifndef HEIM_ENTITY_HPP
#define HEIM_ENTITY_HPP

#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>
#include "utility.hpp"

namespace heim
{
/*!
 * @brief The entity type for the objects in Heim.
 *
 * @tparam Value     The underlying unsigned integral value type.
 * @tparam IndexBits The number of bits dedicated to the index section of the
 *   entity.
 *
 * @details The entity are divided in two sections: the index and the
 *   generation.
 *   The index section is the value used as identification in containers.
 *   The generation section is the value used to differentiate inactive
 *   entities from active entities with the same index.
 */
template<
    typename    Value     = std::uintmax_t,
    std::size_t IndexBits = std::numeric_limits<Value>::digits / 2>
class entity
{
public:
  using size_type  = std::size_t;
  using value_type = Value;

  static_assert(
      std::is_integral_v<value_type>
   && std::is_unsigned_v<value_type>);


  static constexpr
  size_type
  value_bits()
  noexcept
  { return std::numeric_limits<Value>::digits; }
  static constexpr
  size_type
  index_bits()
  noexcept
  { return IndexBits; }

  static constexpr size_type value_bits_v = value_bits();
  static constexpr size_type index_bits_v = index_bits();

  static_assert(0 < index_bits_v && index_bits_v < value_bits_v);

  static constexpr
  size_type
  generation_bits()
  noexcept
  { return value_bits_v - index_bits_v; }

  static constexpr size_type generation_bits_v = generation_bits();


  using index_type      = unsigned_integral_for_t<index_bits_v     >;
  using generation_type = unsigned_integral_for_t<generation_bits_v>;


  static constexpr
  value_type
  null_value()
  noexcept
  { return std::numeric_limits<value_type>::max(); }

  static constexpr
  index_type
  null_index()
  noexcept
  { return static_cast<index_type>(null_value() >> generation_bits()); }

  static constexpr
  generation_type
  null_generation()
  noexcept
  { return static_cast<generation_type>(null_value() >> index_bits()); }

  static constexpr value_type      null_value_v      = null_value();
  static constexpr index_type      null_index_v      = null_index();
  static constexpr generation_type null_generation_v = null_generation();


  static constexpr
  value_type
  index_mask()
  noexcept
  { return static_cast<value_type>(null_index()); }

  static constexpr
  value_type
  generation_mask()
  noexcept
  { return static_cast<value_type>(null_generation()) << index_bits(); }

  static constexpr value_type index_mask_v      = index_mask();
  static constexpr value_type generation_mask_v = generation_mask();

private:
  value_type
  m_value;

public:
  [[nodiscard]] friend constexpr
  bool
  operator==(
      entity const &lhs,
      entity const &rhs)
  noexcept
  = default;



  [[nodiscard]] constexpr
  value_type
  value() const
  noexcept
  { return m_value; }

  [[nodiscard]] constexpr
  index_type
  index() const
  noexcept
  { return static_cast<index_type>(m_value & index_mask_v); }

  [[nodiscard]] constexpr
  generation_type
  generation() const
  noexcept
  { return static_cast<generation_type>(m_value >> index_bits_v); }


  [[nodiscard]] constexpr
  bool
  is_null() const
  noexcept
  { return m_value == null_value_v; }



  constexpr
  entity()
  noexcept
    : m_value{null_value_v}
  { }

  constexpr
  entity(entity const &)
  noexcept
  = default;

  constexpr
  entity(entity &&)
  noexcept
  = default;

  template<
      typename Val>
  requires(
      std::is_same_v<Val, value_type>)
  explicit constexpr
  entity(Val const value)
  noexcept
    : m_value{value}
  { }

  template<
      typename Idx,
      typename Gen>
  requires(
      std::is_same_v<Idx, index_type>
   && std::is_same_v<Gen, generation_type>)
  constexpr
  entity(
      Idx const idx,
      Gen const gen)
  noexcept
    : m_value{
          (static_cast<value_type>(gen) << index_bits_v)
        | (static_cast<value_type>(idx)  & index_mask_v)}
  { }

  constexpr
  ~entity()
  noexcept
  = default;

  constexpr
  entity &
  operator=(entity const &)
  noexcept
  = default;

  constexpr
  entity &
  operator=(entity &&)
  noexcept
  = default;

  template<
      typename Val>
  requires(
      std::is_same_v<Val, value_type>)
  constexpr
  entity &
  operator=(Val const value)
  noexcept
  {
    m_value = value;
    return *this;
  }
};


/*!
 * @brief Determines whether T is a specialization of entity.
 *
 * @tparam T The type to determine for.
 */
template<
    typename T>
struct is_entity
  : bool_constant<false>
{ };

template<
    typename    Value,
    std::size_t IndexBits>
struct is_entity<
    entity<Value, IndexBits>>
  : bool_constant<true>
{ };

template<
    typename T>
inline constexpr
bool
is_entity_v
= is_entity<T>::value();


//! @brief The default entity type.
using default_entity
= entity<>;


} // namespace heim


#endif // HEIM_ENTITY_HPP
