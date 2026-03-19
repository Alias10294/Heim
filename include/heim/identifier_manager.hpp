#ifndef HEIM_IDENTIFIER_MANAGER_HPP
#define HEIM_IDENTIFIER_MANAGER_HPP

#include <cstddef>
#include <memory>
#include <ranges>
#include <type_traits>
#include <utility>
#include <vector>
#include "allocator.hpp"
#include "identifier.hpp"
#include "utility.hpp"

namespace heim
{
/*!
 * @brief An associative container specialized for the management of identifiers in the context of the
 *   entity-component-system pattern.
 *
 * @details Implements a specialized sparse set to separate the container into two sections of the
 *   active and inactive identifiers, whilst providing constant-time insertion, removal and access to
 *   those identifiers, as well as providing optimal iteration speed.
 *
 * @tparam Identifier The identifier type.
 * @tparam Allocator  The allocator  type.
 */
template<
    typename Identifier = identifier<>,
    typename Allocator  = std::allocator<Identifier>>
class identifier_manager;

template<
    typename Identifier,
    typename Allocator>
class identifier_manager
{
public:
  using identifier_type = Identifier;
  using allocator_type  = Allocator;

  static_assert(
      specializes_identifier_v<Identifier>,
      "heim::new_identifier_manager: identifier_type must be a specialization of identifier.");
  static_assert(
      is_an_allocator_for_v<allocator_type, identifier_type>,
      "heim::new_identifier_manager: allocator_type must pass as an allocator of identifier_type.");


  using size_type       = std::size_t;
  using difference_type = std::ptrdiff_t;

private:
  using container_type
  = std::vector<identifier_type, allocator_type>;

  using index_type      = typename identifier_type::index_type;
  using generation_type = typename identifier_type::generation_type;

private:
  container_type m_sparse;
  container_type m_dense;
  size_type      m_begin;

private:
  static constexpr bool s_noexcept_default_construct   () noexcept;
  static constexpr bool s_noexcept_move_alloc_construct() noexcept;
  static constexpr bool s_noexcept_swap() noexcept;

public:
  constexpr explicit
  identifier_manager(allocator_type const &)
  noexcept;

  constexpr
  identifier_manager()
  noexcept(s_noexcept_default_construct());

  constexpr identifier_manager(identifier_manager const &) = default;
  constexpr identifier_manager(identifier_manager &&)      = default;

  constexpr
  identifier_manager(identifier_manager const &, allocator_type const &);

  constexpr
  identifier_manager(identifier_manager &&, allocator_type const &)
  noexcept(s_noexcept_move_alloc_construct());

  constexpr
  ~identifier_manager()
  = default;

  constexpr identifier_manager &operator=(identifier_manager const &) = default;
  constexpr identifier_manager &operator=(identifier_manager &&)      = default;

  [[nodiscard]] constexpr
  allocator_type
  get_allocator() const
  noexcept;

  constexpr
  void
  swap(identifier_manager &)
  noexcept(s_noexcept_swap());


  [[nodiscard]] constexpr
  bool
  is_valid(identifier_type) const
  noexcept;

  [[nodiscard]] constexpr auto all()     const noexcept;
  [[nodiscard]] constexpr auto valid()   const noexcept;
  [[nodiscard]] constexpr auto invalid() const noexcept;


  [[nodiscard]] constexpr
  identifier_type
  summon();

  constexpr
  void
  banish(identifier_type)
  noexcept;

  constexpr
  void
  banish_all()
  noexcept;


  friend constexpr
  void
  swap(identifier_manager &lhs, identifier_manager &rhs)
  noexcept(s_noexcept_swap())
  {
    lhs.swap(rhs);
  }

  [[nodiscard]] friend constexpr
  bool
  operator==(identifier_manager const &lhs, identifier_manager const &rhs)
  noexcept
  {
    if (lhs.m_dense.size() != rhs.m_dense.size())
      return false;
    if (lhs.m_begin != rhs.m_begin)
      return false;

    for (identifier_type const id : lhs.m_dense)
    {
      auto const lhs_pos = static_cast<size_type>(lhs.m_sparse[static_cast<size_type>(id.index())].index());
      auto const rhs_pos = static_cast<size_type>(rhs.m_sparse[static_cast<size_type>(id.index())].index());

      bool const lhs_valid = lhs_pos >= lhs.m_begin;
      bool const rhs_valid = rhs_pos >= rhs.m_begin && rhs.m_dense[rhs_pos] == id;

      if (lhs_valid != rhs_valid)
        return false;
    }

    return true;
  }
};


template<
    typename Identifier,
    typename Allocator>
constexpr
bool
identifier_manager<Identifier, Allocator>
    ::s_noexcept_default_construct()
noexcept
{
  return std::is_nothrow_default_constructible_v<allocator_type>;
}

template<
    typename Identifier,
    typename Allocator>
constexpr
bool
identifier_manager<Identifier, Allocator>
    ::s_noexcept_move_alloc_construct()
noexcept
{
  return std::is_nothrow_constructible_v<
      container_type,
      container_type &&, allocator_type const &>;
}

template<
    typename Identifier,
    typename Allocator>
constexpr
bool
identifier_manager<Identifier, Allocator>
    ::s_noexcept_swap()
noexcept
{
  return std::is_nothrow_swappable_v<container_type>;
}

template<
    typename Identifier,
    typename Allocator>
constexpr
identifier_manager<Identifier, Allocator>
    ::identifier_manager(allocator_type const &alloc)
noexcept
  : m_sparse(alloc),
    m_dense (alloc),
    m_begin {0}
{ }

template<
    typename Identifier,
    typename Allocator>
constexpr
identifier_manager<Identifier, Allocator>
    ::identifier_manager()
noexcept(s_noexcept_default_construct())
  : identifier_manager(allocator_type{})
{ }

template<
    typename Identifier,
    typename Allocator>
constexpr
identifier_manager<Identifier, Allocator>
    ::identifier_manager(identifier_manager const &other, allocator_type const &alloc)
  : m_sparse(other.m_sparse, alloc),
    m_dense (other.m_dense , alloc),
    m_begin (other.m_begin)
{ }

template<
    typename Identifier,
    typename Allocator>
constexpr
identifier_manager<Identifier, Allocator>
    ::identifier_manager(identifier_manager &&other, allocator_type const &alloc)
noexcept(s_noexcept_move_alloc_construct())
  : m_sparse(std::move(other.m_sparse), alloc),
    m_dense (std::move(other.m_dense ), alloc),
    m_begin {other.m_begin}
{ }

template<
    typename Identifier,
    typename Allocator>
constexpr
typename identifier_manager<Identifier, Allocator>
    ::allocator_type
identifier_manager<Identifier, Allocator>
    ::get_allocator() const
noexcept
{
  return m_dense.get_allocator();
}

template<
    typename Identifier,
    typename Allocator>
constexpr
void
identifier_manager<Identifier, Allocator>
    ::swap(identifier_manager &other)
noexcept(s_noexcept_swap())
{
  using std::swap;

  swap(m_sparse, other.m_sparse);
  swap(m_dense , other.m_dense );
  swap(m_begin , other.m_begin );
}

template<
    typename Identifier,
    typename Allocator>
constexpr
bool
identifier_manager<Identifier, Allocator>
    ::is_valid(identifier_type const id) const
noexcept
{
  auto const
  idx
  = static_cast<size_type>(id.index());

  if (idx >= m_sparse.size())
    return false;

  identifier_type const
  pos
  = m_sparse[idx];

  return pos.index()      >= m_begin
      && pos.generation() == id.generation();
}

template<
    typename Identifier,
    typename Allocator>
constexpr
auto
identifier_manager<Identifier, Allocator>
    ::all() const
noexcept
{
  return std::ranges::subrange(m_dense.begin(), m_dense.end());
}

template<
    typename Identifier,
    typename Allocator>
constexpr
auto
identifier_manager<Identifier, Allocator>
    ::valid() const
noexcept
{
  return std::ranges::subrange(m_dense.begin() + m_begin, m_dense.end());
}

template<
    typename Identifier,
    typename Allocator>
constexpr
auto
identifier_manager<Identifier, Allocator>
    ::invalid() const
noexcept
{
  return std::ranges::subrange(m_dense.begin(), m_dense.begin() + m_begin);
}

template<
    typename Identifier,
    typename Allocator>
constexpr
typename identifier_manager<Identifier, Allocator>
    ::identifier_type
identifier_manager<Identifier, Allocator>
    ::summon()
{
  if (m_begin != 0)
    return m_dense[--m_begin];

  auto const
  id
  = identifier_type(static_cast<index_type>(m_dense.size()), generation_type{0});

  m_dense.emplace_back(id);
  // strong exception safety guarantee
  try
  { m_sparse.emplace_back(id); }
  catch (...)
  { m_dense.pop_back(); throw; }

  return id;
}

template<
    typename Identifier,
    typename Allocator>
constexpr
void
identifier_manager<Identifier, Allocator>
    ::banish(identifier_type const id)
noexcept
{
  if (!is_valid(id))
    return;

  auto const idx       = static_cast<size_type>(id.index());
  auto const pos       = static_cast<size_type>(m_sparse[idx]    .index());
  auto const idx_begin = static_cast<size_type>(m_dense [m_begin].index());

  if (pos >= m_begin)
  {
    using std::swap;

    swap(m_dense [pos], m_dense [m_begin  ]);
    swap(m_sparse[idx], m_sparse[idx_begin]);
  }

  identifier_type &
  banned
  = m_dense[m_begin];

  banned = identifier_type(banned.index(), banned.generation() + 1);
  ++m_begin;
}

template<
    typename Identifier,
    typename Allocator>
constexpr
void
identifier_manager<Identifier, Allocator>
    ::banish_all()
noexcept
{
  // we shortcut the individual banish method to avoid unnecessary swap attempts
  for (identifier_type &id : valid())
    id = identifier_type(id.index(), id.generation() + 1);

  m_begin = m_dense.size();
}


/*!
 * @brief Determines whether the given type is a specialization of identifier_manager.
 *
 * @tparam T The type to determine for.
 */
template<typename T>
struct specializes_identifier_manager;

template<typename T>
struct specializes_identifier_manager
  : bool_constant<false>
{ };

template<
    typename Identifier,
    typename Allocator>
struct specializes_identifier_manager<
    identifier_manager<Identifier, Allocator>>
  : bool_constant<true>
{ };

template<typename T>
inline constexpr
bool
specializes_identifier_manager_v
= specializes_identifier_manager<T>::value;


}

#endif // HEIM_IDENTIFIER_MANAGER_HPP