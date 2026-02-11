#ifndef HEIM_ENTITY_MANAGER_HPP
#define HEIM_ENTITY_MANAGER_HPP

#include <cstddef>
#include <memory>
#include <ranges>
#include <type_traits>
#include <utility>
#include <vector>
#include "allocator.hpp"
#include "entity.hpp"
#include "utility.hpp"

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
    typename Entity,
    typename Allocator>
class entity_manager;

template<
    typename Entity,
    typename Allocator>
class entity_manager
{
public:
  using entity_type    = Entity;
  using allocator_type = Allocator;

  static_assert(
      specializes_entity_v<entity_type>,
      "heim::entity_manager: entity_type must specialize entity.");
  static_assert(
      is_an_allocator_for_v<allocator_type, entity_type>,
      "heim::entity_manager: allocator_type must pass as an allocator of entity_type.");

  using size_type       = std::size_t;
  using difference_type = std::ptrdiff_t;

private:
  using entity_allocator = typename std::allocator_traits<allocator_type>::template rebind_alloc<entity_type>;
  using size_allocator   = typename std::allocator_traits<allocator_type>::template rebind_alloc<size_type>;

  using entity_vector   = std::vector<entity_type, entity_allocator>;
  using position_vector = std::vector<size_type  , size_allocator>;

  using index_type      = typename entity_type::index_type;
  using generation_type = typename entity_type::generation_type;

private:
  entity_vector   m_entities;
  position_vector m_positions;
  size_type       m_begin;

private:
  static constexpr
  bool
  s_noexcept_default_construct()
  noexcept;

  static constexpr
  bool
  s_noexcept_move_alloc_construct()
  noexcept;

  static constexpr
  bool
  s_noexcept_swap()
  noexcept;

public:
  explicit constexpr
  entity_manager(allocator_type const &)
  noexcept;

  constexpr
  entity_manager()
  noexcept(s_noexcept_default_construct());

  constexpr
  entity_manager(entity_manager const &)
  = default;

  constexpr
  entity_manager(entity_manager &&)
  = default;

  constexpr
  entity_manager(entity_manager const &, allocator_type const &);

  constexpr
  entity_manager(entity_manager &&, allocator_type const &)
  noexcept(s_noexcept_move_alloc_construct());

  constexpr
  ~entity_manager()
  = default;

  constexpr
  entity_manager &
  operator=(entity_manager const &)
  = default;

  constexpr
  entity_manager &
  operator=(entity_manager &&)
  = default;

  [[nodiscard]] constexpr
  allocator_type
  get_allocator() const
  noexcept;


  [[nodiscard]] constexpr
  auto
  all() const
  noexcept;

  [[nodiscard]] constexpr
  auto
  valid() const
  noexcept;

  [[nodiscard]] constexpr
  auto
  invalid() const
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

  constexpr
  void
  banish_all()
  noexcept;


  constexpr
  void
  swap(entity_manager &)
  noexcept(s_noexcept_swap());


  friend constexpr
  void
  swap(entity_manager &lhs, entity_manager &rhs)
  noexcept(noexcept(lhs.swap(rhs)))
  {
    lhs.swap(rhs);
  }

  [[nodiscard]] friend constexpr
  bool
  operator==(entity_manager const &lhs, entity_manager const &rhs)
  noexcept
  {
    if (lhs.m_entities.size() != rhs.m_entities.size())
      return false;
    if (lhs.m_begin != rhs.m_begin)
      return false;

    for (entity_type const e : lhs.m_entities)
    {
      size_type const rhs_pos_e = rhs.m_positions[static_cast<size_type>(e.index())];
      size_type const lhs_pos_e = lhs.m_positions[static_cast<size_type>(e.index())];

      bool const rhs_is_valid = rhs_pos_e >= rhs.m_begin && rhs.m_entities[rhs_pos_e] == e;
      bool const lhs_is_valid = lhs_pos_e >= lhs.m_begin;

      if (lhs_is_valid != rhs_is_valid)
        return false;
    }

    return true;
  }
}; // class entity_manager



template<
    typename Entity,
    typename Allocator>
constexpr
bool
entity_manager<Entity, Allocator>
    ::s_noexcept_default_construct()
noexcept
{
  return std::is_nothrow_default_constructible_v<allocator_type>;
}


template<
    typename Entity,
    typename Allocator>
constexpr
bool
entity_manager<Entity, Allocator>
    ::s_noexcept_move_alloc_construct()
noexcept
{
  return
      std::is_nothrow_constructible_v<
          entity_vector,
          entity_vector &&, entity_allocator const &>
   && std::is_nothrow_constructible_v<
          position_vector,
          position_vector &&, size_allocator const &>;
}

template<
    typename Entity,
    typename Allocator>
constexpr
bool
entity_manager<Entity, Allocator>
    ::s_noexcept_swap()
noexcept
{
  return std::is_nothrow_swappable_v<entity_vector  >
      && std::is_nothrow_swappable_v<position_vector>;
}



template<
    typename Entity,
    typename Allocator>
constexpr
entity_manager<Entity, Allocator>
    ::entity_manager(allocator_type const &alloc)
noexcept
  : m_entities (entity_allocator(alloc)),
    m_positions(size_allocator  (alloc)),
    m_begin    (0)
{ }

template<
    typename Entity,
    typename Allocator>
constexpr
entity_manager<Entity, Allocator>
    ::entity_manager()
noexcept(s_noexcept_default_construct())
  : entity_manager(allocator_type())
{ }

template<
    typename Entity,
    typename Allocator>
constexpr
entity_manager<Entity, Allocator>
    ::entity_manager(entity_manager const &other, allocator_type const &alloc)
  : m_entities (other.m_entities , entity_allocator(alloc)),
    m_positions(other.m_positions, size_allocator  (alloc)),
    m_begin    (other.m_begin)
{ }

template<
    typename Entity,
    typename Allocator>
constexpr
entity_manager<Entity, Allocator>
    ::entity_manager(entity_manager &&other, allocator_type const &alloc)
noexcept(s_noexcept_move_alloc_construct())
  : m_entities (std::move(other.m_entities ), entity_allocator(alloc)),
    m_positions(std::move(other.m_positions), size_allocator  (alloc)),
    m_begin    (std::move(other.m_begin))
{ }


template<
    typename Entity,
    typename Allocator>
constexpr
typename entity_manager<Entity, Allocator>
    ::allocator_type
entity_manager<Entity, Allocator>
    ::get_allocator() const
noexcept
{
  return allocator_type(m_entities.get_allocator());
}



template<
    typename Entity,
    typename Allocator>
constexpr
auto
entity_manager<Entity, Allocator>
    ::all() const
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
entity_manager<Entity, Allocator>
    ::valid() const
noexcept
{
  return std::ranges::subrange(
      m_entities.begin() + m_begin,
      m_entities.end  ());
}

template<
    typename Entity,
    typename Allocator>
constexpr
auto
entity_manager<Entity, Allocator>
    ::invalid() const
noexcept
{
  return std::ranges::subrange(
      m_entities.begin(),
      m_entities.begin() + m_begin);
}



template<
    typename Entity,
    typename Allocator>
constexpr
bool
entity_manager<Entity, Allocator>
    ::is_valid(entity_type const e) const
noexcept
{
  size_type const idx = e.index();
  if (idx >= m_positions.size())
    return false;

  size_type const pos = m_positions[idx];
  return pos >= m_begin
      && m_entities[pos] == e;
}



template<typename Entity, typename Allocator>
constexpr
typename entity_manager<Entity, Allocator>
    ::entity_type
entity_manager<Entity, Allocator>
    ::summon()
{
  if (m_begin != 0)
    return m_entities[--m_begin];

  size_type const pos = m_entities.size();

  m_entities.emplace_back(static_cast<index_type>(pos), generation_type(0));
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
entity_manager<Entity, Allocator>
    ::banish(entity_type const e)
noexcept
{
  if (!is_valid(e))
    return;

  size_type const idx_e     = e.index();
  size_type const pos_e     = m_positions[idx_e];
  size_type const pos_begin = m_begin;
  size_type const idx_begin = m_entities[pos_begin].index();

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
void
entity_manager<Entity, Allocator>
    ::banish_all()
noexcept
{
  // we shortcut the individual banish method to avoid unnecessary swap attempts
  for (entity_type &e : valid())
    e = entity_type(e.index(), e.generation() + 1);

  m_begin = m_entities.size();
}



template<
    typename Entity,
    typename Allocator>
constexpr
void
entity_manager<Entity, Allocator>
    ::swap(entity_manager &other)
noexcept(s_noexcept_swap())
{
  std::swap(m_entities , other.m_entities );
  std::swap(m_positions, other.m_positions);
  std::swap(m_begin    , other.m_begin    );
}



/*!
 * @brief Determines whether the given type is a specialization of entity_manager.
 *
 * @tparam T The type to determine for.
 */
template<typename T>
struct specializes_entity_manager;

template<typename T>
inline constexpr
bool
specializes_entity_manager_v
= specializes_entity_manager<T>::value;

template<typename T>
struct specializes_entity_manager
  : bool_constant<false>
{ };

template<
    typename Entity,
    typename Allocator>
struct specializes_entity_manager<
    entity_manager<Entity, Allocator>>
  : bool_constant<true>
{ };


} // namespace heim

#endif // HEIM_ENTITY_MANAGER_HPP