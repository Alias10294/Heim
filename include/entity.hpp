#ifndef HEIM_ENTITY_HPP
#define HEIM_ENTITY_HPP

#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>
#include "utility.hpp"

namespace heim
{
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


  static constexpr size_type value_bits = std::numeric_limits<value_type>::digits;
  static constexpr size_type index_bits = IndexBits;

  static_assert(0 < index_bits && index_bits < value_bits);

  static constexpr size_type generation_bits = value_bits - index_bits;


  using index_type      = unsigned_integral_for_t<index_bits     >;
  using generation_type = unsigned_integral_for_t<generation_bits>;


  static constexpr value_type      null_value      = std::numeric_limits<value_type>::max();
  static constexpr index_type      null_index      = static_cast<index_type>(null_value >> generation_bits);
  static constexpr generation_type null_generation = static_cast<generation_type>(null_value >> index_bits);

  static constexpr value_type index_mask      = static_cast<value_type>(null_index     );
  static constexpr value_type generation_mask = static_cast<value_type>(generation_bits);

private:
  value_type m_value;

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
  { return static_cast<index_type>(m_value & index_mask); }

  [[nodiscard]] constexpr
  generation_type
  generation() const
  noexcept
  { return static_cast<generation_type>(m_value >> index_bits); }


  [[nodiscard]] constexpr
  bool
  is_null() const
  noexcept
  { return m_value == null_value; }



  constexpr
  entity()
  noexcept
    : m_value{null_value}
  { }

  constexpr
  entity(entity const &)
  noexcept
  = default;

  constexpr
  entity(entity &&)
  noexcept
  = default;

  template<typename Val>
  explicit constexpr
  entity(Val const value)
  noexcept
  requires(std::is_same_v<Val, value_type>)
    : m_value(value)
  { }

  template<
      typename Idx,
      typename Gen>
  constexpr
  entity(
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

  template<typename Val>
  constexpr
  entity &
  operator=(Val const value)
  noexcept
  requires(std::is_same_v<Val, value_type>)
  {
    m_value = value;
    return *this;
  }
};


template<typename T>
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

template<typename T>
inline constexpr bool is_entity_v = is_entity<T>::value;



using default_entity = entity<>;


} // namespace heim

#endif // HEIM_ENTITY_HPP
