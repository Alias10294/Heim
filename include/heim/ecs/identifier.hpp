#ifndef HEIM_ECS_IDENTIFIER_HPP
#define HEIM_ECS_IDENTIFIER_HPP

#include <cstdint>
#include <limits>
#include <type_traits>
#include "heim/lib/utility.hpp"

namespace heim
{
/*!
 * \brief
 *   Determines whether the specializing type is an identifier type.
 *
 * \details
 *   An identifier type, in the context of the entity-component-system (ECS) pattern, is a type able
 *   to uniquely identify an entity.
 */
template<typename T>
struct is_identifier
  : std::bool_constant<std::is_unsigned_v<T> && std::is_integral_v<T>>
{ };

template<typename T>
inline constexpr
bool
is_identifier_v
= is_identifier<T>::value;

template<typename T>
concept identifier
= is_identifier_v<T>;


template<typename>
struct identifier_traits
{ };

template<typename Identifier>
requires identifier<Identifier>
struct identifier_traits<Identifier>
{
  using identifier_type = Identifier;

  static_assert(
      is_identifier_v<identifier_type>,
      "heim::identifier_traits: identifier_type must be an identifier type.");


  static constexpr int index_digits      = std::numeric_limits<identifier_type>::digits / 2;
  static constexpr int generation_digits = std::numeric_limits<identifier_type>::digits / 2;

  using index_type      = unsigned_integral_for_t<index_digits>;
  using generation_type = unsigned_integral_for_t<generation_digits>;

  static constexpr identifier_type index_mask      = std::numeric_limits<identifier_type>::max() >> generation_digits;
  static constexpr identifier_type generation_mask = std::numeric_limits<identifier_type>::max() << index_digits;

  static constexpr identifier_type null
  = std::numeric_limits<identifier_type>::max();

  static constexpr
  index_type
  index(identifier_type const id)
  noexcept
  { return id & index_mask; }

  static constexpr
  generation_type
  generation(identifier_type const id)
  noexcept
  { return id >> index_digits; }

  static constexpr
  identifier_type
  from(
      index_type      const idx,
      generation_type const gen)
  noexcept
  {
    return (identifier_type{gen} << index_digits)
         | (identifier_type{idx} &  index_mask);
  }

  static constexpr
  identifier_type
  next(identifier_type const id)
  noexcept
  { return id + (identifier_type{1} << index_digits); }
};


template<typename = void>
struct default_identifier
  : std::type_identity<std::uint64_t>
{ };

template<typename = void>
using default_identifier_t
= typename default_identifier<>::type;

} // namespace heim

#endif // HEIM_ECS_IDENTIFIER_HPP
