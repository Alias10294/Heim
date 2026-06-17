#ifndef HEIM_ECS_IDENTIFIER_HPP
#define HEIM_ECS_IDENTIFIER_HPP

#include <cstdint>
#include <limits>
#include <type_traits>
#include "heim/lib/utility.hpp"

namespace heim::ecs
{
template<typename>
struct identifier_traits
{ };

template<typename Id>
requires std::unsigned_integral<Id>
struct identifier_traits<Id>
{
  using identifier_type = Id;

  static constexpr int index_digits      = std::numeric_limits<Id>::digits / 2;
  static constexpr int generation_digits = std::numeric_limits<Id>::digits / 2;

  using index_type      = unsigned_integral_for_t<index_digits>;
  using generation_type = unsigned_integral_for_t<generation_digits>;

  static constexpr identifier_type index_mask      = std::numeric_limits<Id>::max() >> generation_digits;
  static constexpr identifier_type generation_mask = std::numeric_limits<Id>::max() << index_digits;

  static constexpr identifier_type null
  = std::numeric_limits<Id>::max();

  static constexpr
  index_type
  index(identifier_type const id)
  noexcept
  { return static_cast<index_type>(id & index_mask); }

  static constexpr
  generation_type
  generation(identifier_type const id)
  noexcept
  { return static_cast<generation_type>(id >> index_digits); }

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

using default_identifier_t
= typename default_identifier<>::type;


} // namespace heim::ecs

#endif // HEIM_ECS_IDENTIFIER_HPP
