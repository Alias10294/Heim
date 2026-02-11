#ifndef HEIM_ENTITY_HPP
#define HEIM_ENTITY_HPP

#include <cstdint>
#include <type_traits>
#include <utility>
#include "utility.hpp"

namespace heim
{
/*!
 * @brief An implementation of the entity type as described by the generic_entity-component-system
 *   pattern.
 *
 * @details Encapsulates an unsigned integral value and divides it into an index and a generation.
 *   This generation mechanism allows us to recycle index values and monitor the lifetime of
 *   entities themselves.
 *
 * @tparam UInt        The underlying value type.
 * @tparam IndexDigits The number of bits allocated to represent the index value.
 *
 * @note All remaining bits thar are not used for the index are used for its generation.
 */
template<
    typename UInt        = std::uintmax_t,
    int      IndexDigits = std::numeric_limits<UInt>::digits / 2>
class entity;

template<
    typename UInt,
    int      IndexDigits>
class entity
{
public:
  using underlying_type = UInt;

  static_assert(
      std::is_integral_v<underlying_type> && std::is_unsigned_v<underlying_type>,
      "heim::entity: underlying_type must be an unsigned integral type.");

  static constexpr int value_digits      = std::numeric_limits<underlying_type>::digits;
  static constexpr int index_digits      = IndexDigits;
  static constexpr int generation_digits = value_digits - index_digits;

  using value_type      = underlying_type;
  using index_type      = unsigned_integral_for_t<index_digits     >;
  using generation_type = unsigned_integral_for_t<generation_digits>;

private:
  static constexpr value_type null_value = std::numeric_limits<value_type>::max();

  static constexpr value_type index_mask      = static_cast<value_type>(null_value) >> generation_digits;
  static constexpr value_type generation_mask = static_cast<value_type>(null_value) << index_digits;

private:
  value_type m_value;

public:
  constexpr
  entity()
  noexcept;

  constexpr
  entity(entity const &)
  = default;

  constexpr
  entity(entity &&)
  = default;

  explicit constexpr
  entity(value_type const)
  noexcept;

  constexpr
  entity(index_type const, generation_type const)
  noexcept;

  constexpr
  ~entity()
  = default;

  constexpr
  entity &
  operator=(entity const &)
  = default;

  constexpr
  entity &
  operator=(entity &&)
  = default;

  constexpr
  entity &
  operator=(value_type const)
  noexcept;

  constexpr
  entity &
  operator=(std::pair<index_type, generation_type> const &)
  noexcept;


  [[nodiscard]] constexpr
  value_type
  value() const
  noexcept;

  [[nodiscard]] constexpr
  index_type
  index() const
  noexcept;

  [[nodiscard]] constexpr
  generation_type
  generation() const
  noexcept;


  [[nodiscard]] friend constexpr
  bool
  operator==(entity const &, entity const &)
  noexcept
  = default;
};



template<
    typename UInt,
    int      IndexDigits>
constexpr
entity<UInt, IndexDigits>
    ::entity()
noexcept
  : m_value(null_value)
{ }

template<
    typename UInt,
    int      IndexBits>
constexpr
entity<UInt, IndexBits>
    ::entity(value_type const val)
noexcept
  : m_value(val)
{ }

template<
    typename UInt,
    int      IndexBits>
constexpr
entity<UInt, IndexBits>
    ::entity(index_type const idx, generation_type const gen)
noexcept
  : entity(
        (static_cast<value_type>(gen) << index_digits)
      | (static_cast<value_type>(idx)  & index_mask))
{ }


template<
    typename UInt,
    int      IndexBits>
constexpr
entity<UInt, IndexBits> &
entity<UInt, IndexBits>
    ::operator=(value_type const val)
noexcept
{
  m_value = val;
  return *this;
}

template<
    typename UInt,
    int      IndexBits>
constexpr
entity<UInt, IndexBits> &
entity<UInt, IndexBits>
    ::operator=(std::pair<index_type, generation_type> const &pair)
noexcept
{
  m_value
  =   (static_cast<value_type>(pair.second) << index_digits)
    | (static_cast<value_type>(pair.first )  & index_mask);
  return *this;
}



template<
    typename UInt,
    int      IndexBits>
constexpr
typename entity<UInt, IndexBits>
    ::value_type
entity<UInt, IndexBits>
    ::value() const
noexcept
{
  return m_value;
}


template<
    typename UInt,
    int      IndexBits>
constexpr
typename entity<UInt, IndexBits>
    ::index_type
entity<UInt, IndexBits>
    ::index() const
noexcept
{
  return static_cast<index_type>(m_value & index_mask);
}

template<
    typename UInt,
    int      IndexBits>
constexpr
typename entity<UInt, IndexBits>
    ::generation_type
entity<UInt, IndexBits>
    ::generation() const
noexcept
{
  return static_cast<generation_type>(m_value >> index_digits);
}



/*!
 * @brief Determines whether the given type is a specialization of entity.
 *
 * @tparam T The type to determine for.
 */
template<typename T>
struct specializes_entity;

template<typename T>
inline constexpr
bool
specializes_entity_v
= specializes_entity<T>::value;

template<typename T>
struct specializes_entity
  : bool_constant<false>
{ };

template<
    typename UInt,
    int      IndexBits>
struct specializes_entity<
    entity<UInt, IndexBits>>
  : bool_constant<true>
{ };


}

#endif // HEIM_ENTITY_HPP