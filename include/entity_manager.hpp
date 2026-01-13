#ifndef HEIM_ENTITY_MANAGER_HPP
#define HEIM_ENTITY_MANAGER_HPP

#include <cstddef>
#include <ranges>
#include <type_traits>
#include <utility>
#include <vector>
#include "allocator.hpp"
#include "entity.hpp"

namespace heim
{
/*!
 * @brief An associative container specializing in the management of entities in the
 *   entity-component-system pattern.
 *
 * @details Implements a customized sparse set, that is each entity's position in the container is
 *   kept tracked by a complementary array. This structure allows for constant-time insertion,
 *   removal and access to entities, whilst containing the entities in contiguous memory for
 *   optimal iteration.
 *   Moreover, internally the dense array of entities of partitioned in two groups of valid and
 *   invalid entities. The invalid entities sit at the front of the vector and the valid ones at
 *   the back to accommodate for emplaced newly-generated entities.
 *
 * @tparam Entity    The entity type.
 * @tparam Allocator The allocator type.
 */
template<
    typename Entity    = entity,
    typename Allocator = allocator<Entity>>
class generic_entity_manager
{
public:
  using size_type       = std::size_t;
  using difference_type = std::ptrdiff_t;


  using entity_type    = Entity;
  using allocator_type = Allocator;

  static_assert(
      is_entity_v<entity_type>,
      "entity_type must be a specialization of generic_entity.");
  static_assert(
      is_an_allocator_for_v<allocator_type, entity_type>,
      "allocator_type must pass as an allocator of entity_type.");

private:
  using alloc_traits = allocator_traits<allocator_type>;

  using size_allocator   = typename alloc_traits::template rebind_alloc<size_type  >;
  using entity_allocator = typename alloc_traits::template rebind_alloc<entity_type>;

  using entity_vector   = std::vector<entity_type, entity_allocator>;
  using position_vector = std::vector<size_type  , size_allocator  >;


  using index_type      = typename entity_type::index_type;
  using generation_type = typename entity_type::generation_type;

public:
  using value_type = typename entity_vector::value_type;

  using reference       = typename entity_vector::reference;
  using const_reference = typename entity_vector::const_reference;

  using iterator       = typename entity_vector::iterator;
  using const_iterator = typename entity_vector::const_iterator;

  using reverse_iterator       = typename entity_vector::reverse_iterator;
  using const_reverse_iterator = typename entity_vector::const_reverse_iterator;

private:
  entity_vector   m_entities;
  position_vector m_positions;
  size_type       m_begin;

public:
  [[nodiscard]] constexpr
  auto
  valid()
  noexcept;

  [[nodiscard]] constexpr
  auto
  valid() const
  noexcept;

  [[nodiscard]] constexpr
  auto
  invalid()
  noexcept;

  [[nodiscard]] constexpr
  auto
  invalid() const
  noexcept;

  [[nodiscard]] constexpr
  auto
  all()
  noexcept;

  [[nodiscard]] constexpr
  auto
  all() const
  noexcept;


  [[nodiscard]] constexpr
  bool
  is_valid(entity_type const) const
  noexcept;


  [[nodiscard]] constexpr
  entity_type
  summon();

  constexpr
  void
  banish(entity_type const)
  noexcept;


  [[nodiscard]] constexpr
  allocator_type
  get_allocator() const
  noexcept;

  explicit constexpr
  generic_entity_manager(allocator_type const &)
  noexcept;

  constexpr
  generic_entity_manager()
  noexcept(std::is_nothrow_constructible_v<allocator_type>)
  requires(std::is_constructible_v        <allocator_type>);

  constexpr
  generic_entity_manager(
      generic_entity_manager const &,
      allocator_type const &);

  constexpr
  generic_entity_manager(generic_entity_manager const &)
  = default;

  constexpr
  generic_entity_manager(
      generic_entity_manager &&,
      allocator_type const &)
  noexcept(
      std::is_nothrow_constructible_v<
          position_vector,
          position_vector &&, size_allocator const &>
   && std::is_nothrow_constructible_v<
          entity_vector,
          entity_vector &&, entity_allocator const &>);

  constexpr
  generic_entity_manager(generic_entity_manager &&)
  = default;

  constexpr
  ~generic_entity_manager()
  = default;

  constexpr generic_entity_manager &
  operator=(generic_entity_manager const &)
  = default;

  constexpr generic_entity_manager &
  operator=(generic_entity_manager &&)
  = default;
};


/*!
 * @brief Determines whether the given type is a specialization of generic_entity_manager.
 *
 * @tparam T The type to determine for.
 */
template<typename T>
struct is_entity_manager
  : bool_constant<false>
{ };

template<typename T>
inline constexpr
bool
is_entity_manager_v
= is_entity_manager<T>::value;

template<
    typename Entity,
    typename Allocator>
struct is_entity_manager<
    generic_entity_manager<Entity, Allocator>>
  : bool_constant<true>
{ };


//! @brief The default specialization of generic_entity_manager.
using entity_manager
= generic_entity_manager<>;





template<
    typename Entity,
    typename Allocator>
constexpr
auto
generic_entity_manager<Entity, Allocator>
    ::valid()
noexcept
{
  return std::ranges::subrange(
      m_entities.begin() + static_cast<difference_type>(m_begin),
      m_entities.end  ());
}

template<
    typename Entity,
    typename Allocator>
constexpr
auto
generic_entity_manager<Entity, Allocator>
    ::valid() const
noexcept
{
  return std::ranges::subrange(
      m_entities.cbegin() + static_cast<difference_type>(m_begin),
      m_entities.cend  ());
}


template<
    typename Entity,
    typename Allocator>
constexpr
auto
generic_entity_manager<Entity, Allocator>
    ::invalid()
noexcept
{
  return std::ranges::subrange(
      m_entities.begin(),
      m_entities.begin() + static_cast<difference_type>(m_begin));
}

template<
    typename Entity,
    typename Allocator>
constexpr
auto
generic_entity_manager<Entity, Allocator>
    ::invalid() const
noexcept
{
  return std::ranges::subrange(
      m_entities.cbegin(),
      m_entities.cbegin() + static_cast<difference_type>(m_begin));
}


template<
    typename Entity,
    typename Allocator>
constexpr
auto
generic_entity_manager<Entity, Allocator>
    ::all()
noexcept
{
  return std::ranges::subrange(
      m_entities.begin(),
      m_entities.end  ());
}

template<
    typename Entity,
    typename Allocator>
constexpr
auto
generic_entity_manager<Entity, Allocator>
    ::all() const
noexcept
{
  return std::ranges::subrange(
      m_entities.cbegin(),
      m_entities.cend  ());
}



template<
    typename Entity,
    typename Allocator>
constexpr
bool
generic_entity_manager<Entity, Allocator>
    ::is_valid(entity_type const e) const
noexcept
{
  size_type const idx = static_cast<size_type>(e.index());
  if (idx >= m_positions.size())
    return false;

  size_type const pos = m_positions[idx];

  return pos >= m_begin
      && m_entities[pos] == e;
}


template<
    typename Entity,
    typename Allocator>
constexpr
typename generic_entity_manager<Entity, Allocator>
    ::entity_type
generic_entity_manager<Entity, Allocator>
    ::summon()
{
  if (m_begin != 0)
    return m_entities[--m_begin];

  size_type const pos = m_entities.size();

  m_entities.emplace_back(
      static_cast<index_type>(pos),
      generation_type(0));
  // strong exception safety guarantee
  try
  { m_positions.emplace_back(pos); }
  catch (...)
  { m_entities.pop_back(); throw; }

  return m_entities.back();
}


template<
    typename Entity,
    typename Allocator>
constexpr
void
generic_entity_manager<Entity, Allocator>
    ::banish(entity_type e)
noexcept
{
  if (!is_valid(e))
    return;

  size_type const idx_e     = static_cast<size_type>(e.index());
  size_type const pos_e     = m_positions[idx_e];
  size_type const pos_begin = m_begin;
  size_type const idx_begin = static_cast<size_type>(m_entities[pos_begin].index());

  if (pos_e != pos_begin)
  {
    using std::swap;
    swap(m_entities [pos_e], m_entities [pos_begin]);
    swap(m_positions[idx_e], m_positions[idx_begin]);
  }

  entity_type &banned = m_entities[pos_begin];
  banned = entity_type(banned.index(), banned.generation() + 1);

  ++m_begin;
}



template<
    typename Entity,
    typename Allocator>
constexpr
typename generic_entity_manager<Entity, Allocator>
    ::allocator_type
generic_entity_manager<Entity, Allocator>
    ::get_allocator() const
noexcept
{
  return allocator_type(m_entities.get_allocator());
}

template<
    typename Entity,
    typename Allocator>
constexpr
generic_entity_manager<Entity, Allocator>
    ::generic_entity_manager(allocator_type const &alloc)
noexcept
  : m_entities (entity_allocator(alloc)),
    m_positions(size_allocator  (alloc)),
    m_begin    (0)
{ }

template<
    typename Entity,
    typename Allocator>
constexpr
generic_entity_manager<Entity, Allocator>
    ::generic_entity_manager()
noexcept(std::is_nothrow_constructible_v<allocator_type>)
requires(std::is_constructible_v        <allocator_type>)
  : generic_entity_manager(allocator_type())
{ }

template<
    typename Entity,
    typename Allocator>
constexpr
generic_entity_manager<Entity, Allocator>
    ::generic_entity_manager(
        generic_entity_manager const &other,
        allocator_type const &alloc)
  : m_entities (other.m_entities , entity_allocator(alloc)),
    m_positions(other.m_positions, size_allocator  (alloc)),
    m_begin    (other.m_begin)
{ }

template<
    typename Entity,
    typename Allocator>
constexpr
generic_entity_manager<Entity, Allocator>
    ::generic_entity_manager(
        generic_entity_manager &&     other,
        allocator_type const &alloc)
noexcept(
    std::is_nothrow_constructible_v<
        position_vector,
        position_vector &&, size_allocator const &>
 && std::is_nothrow_constructible_v<
        entity_vector,
        entity_vector &&, entity_allocator const &>)
  : m_entities (std::move(other.m_entities ), entity_allocator(alloc)),
    m_positions(std::move(other.m_positions), size_allocator  (alloc)),
    m_begin    (std::move(other.m_begin))
{ }


} // namespace heim

#endif // HEIM_ENTITY_MANAGER_HPP
