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
 * @brief The library's entity type, that acts as the identifier for a represented object.
 *
 * @tparam Value The underlying unsigned integral type.
 * @tparam IndexBits The number of bits representing the index value of the entity.
 *
 * @details This type uses a mechanism of index and generation to differentiate active and inactive entities,
 *   and to allow for the reuse of index values.
 */
template<
    typename    Value     = std::uintmax_t,
    std::size_t IndexBits = std::numeric_limits<Value>::digits / 2>
class generic_entity
{
public:
  //! @brief The size type.
  using size_type  = std::size_t;
  //! @brief The underlying unsigned integral value type.
  using value_type = Value;

  static_assert(
      std::is_integral_v<value_type> && std::is_unsigned_v<value_type>,
      "The value type must be an unsigned integral type.");


  //! @brief The number of bits of the value_type.
  static constexpr size_type value_bits = std::numeric_limits<value_type>::digits;
  //! @brief The number of bits of the index value.
  static constexpr size_type index_bits = IndexBits;

  static_assert(
      0 < index_bits && index_bits < value_bits,
      "The number of index bits must be valid and within the value type's number of bits");

  //! @brief The number of bits of the generation value.
  static constexpr size_type generation_bits = value_bits - index_bits;


  //! @brief The optimal type to represent the index value.
  using index_type      = unsigned_integral_for_t<index_bits     >;
  //! @brief The optimal type to represent the generation value.
  using generation_type = unsigned_integral_for_t<generation_bits>;


  //! @brief The value for a null or invalid entity value.
  static constexpr value_type      null_value      = std::numeric_limits<value_type>::max();
  //! @brief The value for a null or invalid index value.
  static constexpr index_type      null_index      = static_cast<index_type>(null_value >> generation_bits);
  //! @brief The value for a null or invalid generation value.
  static constexpr generation_type null_generation = static_cast<generation_type>(null_value >> index_bits);

  //! @brief The value isolating the index value bits in the entity value.
  static constexpr value_type index_mask      = static_cast<value_type>(null_index     );
  //! @brief The value isolating the generation value bits in the entity value.
  static constexpr value_type generation_mask = static_cast<value_type>(null_generation) << index_bits;

private:
  //! @brief The entity value.
  value_type m_value;

public:
  [[nodiscard]] friend constexpr
  bool
  operator==(
      generic_entity const &lhs,
      generic_entity const &rhs)
  noexcept
  = default;



  //! @returns The underlying unsigned integral type value of the entity.
  [[nodiscard]] constexpr
  value_type
  value() const
  noexcept
  { return m_value; }

  //! @returns The index value of the entity.
  [[nodiscard]] constexpr
  index_type
  index() const
  noexcept
  { return static_cast<index_type>(m_value & index_mask); }

  //! @returns The generation value of the entity.
  [[nodiscard]] constexpr
  generation_type
  generation() const
  noexcept
  { return static_cast<generation_type>(m_value >> index_bits); }


  //! @returns @c true if the entity has the null value, @c false otherwise.
  [[nodiscard]] constexpr
  bool
  is_null() const
  noexcept
  { return m_value == null_value; }



  constexpr
  generic_entity()
  noexcept
    : m_value(null_value)
  { }

  constexpr
  generic_entity(generic_entity const &)
  noexcept
  = default;

  constexpr
  generic_entity(generic_entity &&)
  noexcept
  = default;

  template<typename Val>
  explicit constexpr
  generic_entity(Val const value)
  noexcept
  requires(std::is_same_v<Val, value_type>)
    : m_value(value)
  { }

  template<
      typename Idx,
      typename Gen>
  constexpr
  generic_entity(
      Idx const idx,
      Gen const gen)
  noexcept
  requires(
      std::is_same_v<Idx, index_type>
   && std::is_same_v<Gen, generation_type>)
    : m_value(
          (static_cast<value_type>(gen) << index_bits)
        | (static_cast<value_type>(idx)  & index_mask))
  { }

  constexpr
  ~generic_entity()
  noexcept
  = default;

  constexpr
  generic_entity &
  operator=(generic_entity const &)
  noexcept
  = default;

  constexpr
  generic_entity &
  operator=(generic_entity &&)
  noexcept
  = default;

  template<typename Val>
  constexpr
  generic_entity &
  operator=(Val const value)
  noexcept
  requires(std::is_same_v<Val, value_type>)
  {
    m_value = value;
    return *this;
  }
};


/*!
 * @brief Determines whether the given type is an entity (i.e. a specialization of generic_entity).
 *
 * @tparam T The type to determine for.
 */
template<typename T>
struct is_entity
  : bool_constant<false>
{ };

template<
    typename    Value,
    std::size_t IndexBits>
struct is_entity<
    generic_entity<Value, IndexBits>>
  : bool_constant<true>
{ };

template<typename T>
inline constexpr
bool
is_entity_v
= is_entity<T>::value;



//! @brief The default entity type across the library.
using entity
= generic_entity<>;


} // namespace heim

#endif // HEIM_ENTITY_HPP
