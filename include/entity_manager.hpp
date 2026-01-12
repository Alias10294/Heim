#ifndef HEIM_ENTITY_MANAGER_HPP
#define HEIM_ENTITY_MANAGER_HPP

#include <cstddef>
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
 *   the back to accommodate for newly-generated entities.
 *
 * @tparam Entity    The entity type.
 * @tparam Allocator The allocator type.
 *
 * @note By default, the interval described by the begin and end iterators of the manager only
 *   includes the partition of valid entities.
 */
template<
    typename Entity    = entity,
    typename Allocator = allocator<Entity>>
class entity_manager
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
      is_allocator_for_v<allocator_type, entity_type>,
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
  difference_type m_begin;

public:
  [[nodiscard]] constexpr
  iterator
  begin()
  noexcept;

  [[nodiscard]] constexpr
  const_iterator
  begin() const
  noexcept;

  [[nodiscard]] constexpr
  iterator
  end()
  noexcept;

  [[nodiscard]] constexpr
  const_iterator
  end() const
  noexcept;

  [[nodiscard]] constexpr
  const_iterator
  cbegin() const
  noexcept;

  [[nodiscard]] constexpr
  const_iterator
  cend() const
  noexcept;

  [[nodiscard]] constexpr
  reverse_iterator
  rbegin()
  noexcept;

  [[nodiscard]] constexpr
  const_reverse_iterator
  rbegin() const
  noexcept;

  [[nodiscard]] constexpr
  reverse_iterator
  rend()
  noexcept;

  [[nodiscard]] constexpr
  const_reverse_iterator
  rend() const
  noexcept;

  [[nodiscard]] constexpr
  const_reverse_iterator
  crbegin() const
  noexcept;

  [[nodiscard]] constexpr
  const_reverse_iterator
  crend() const
  noexcept;


  [[nodiscard]] constexpr
  allocator_type
  get_allocator() const
  noexcept;

  explicit constexpr
  entity_manager(allocator_type const &)
  noexcept;

  constexpr
  entity_manager()
  noexcept(std::is_nothrow_constructible_v<allocator_type>)
  requires(std::is_constructible_v        <allocator_type>);

  constexpr
  entity_manager(
      entity_manager const &,
      allocator_type const &);

  constexpr
  entity_manager(entity_manager const &)
  = default;

  constexpr
  entity_manager(
      entity_manager &&,
      allocator_type const &)
  noexcept(
      std::is_nothrow_constructible_v<
          position_vector,
          position_vector &&, size_allocator const &>
   && std::is_nothrow_constructible_v<
          entity_vector,
          entity_vector &&, entity_allocator const &>);

  constexpr
  entity_manager(entity_manager &&)
  = default;

  constexpr
  ~entity_manager()
  = default;

  constexpr entity_manager &
  operator=(entity_manager const &)
  = default;

  constexpr entity_manager &
  operator=(entity_manager &&)
  = default;
};





template<
    typename Entity,
    typename Allocator>
constexpr
typename entity_manager<Entity, Allocator>
    ::iterator
entity_manager<Entity, Allocator>
    ::begin()
noexcept
{
  return m_entities.begin() + m_begin;
}

template<
    typename Entity,
    typename Allocator>
constexpr
typename entity_manager<Entity, Allocator>
    ::const_iterator
entity_manager<Entity, Allocator>
    ::begin() const
noexcept
{
  return m_entities.begin() + m_begin;
}


template<
    typename Entity,
    typename Allocator>
constexpr
typename entity_manager<Entity, Allocator>
    ::iterator
entity_manager<Entity, Allocator>
    ::end()
noexcept
{
  return m_entities.end();
}

template<
    typename Entity,
    typename Allocator>
constexpr
typename entity_manager<Entity, Allocator>
    ::const_iterator
entity_manager<Entity, Allocator>
    ::end() const
noexcept
{
  return m_entities.end();
}


template<
    typename Entity,
    typename Allocator>
constexpr
typename entity_manager<Entity, Allocator>
    ::const_iterator
entity_manager<Entity, Allocator>
    ::cbegin() const
noexcept
{
  return begin();
}


template<
    typename Entity,
    typename Allocator>
constexpr
typename entity_manager<Entity, Allocator>
    ::const_iterator
entity_manager<Entity, Allocator>
    ::cend() const
noexcept
{
  return end();
}


template<
    typename Entity,
    typename Allocator>
constexpr
typename entity_manager<Entity, Allocator>
    ::reverse_iterator
entity_manager<Entity, Allocator>
    ::rbegin()
noexcept
{
  return reverse_iterator(end());
}

template<
    typename Entity,
    typename Allocator>
constexpr
typename entity_manager<Entity, Allocator>
    ::const_reverse_iterator
entity_manager<Entity, Allocator>
    ::rbegin() const
noexcept
{
  return const_reverse_iterator(end());
}


template<
    typename Entity,
    typename Allocator>
constexpr
typename entity_manager<Entity, Allocator>
    ::reverse_iterator
entity_manager<Entity, Allocator>
    ::rend()
noexcept
{
  return reverse_iterator(begin());
}

template<
    typename Entity,
    typename Allocator>
constexpr
typename entity_manager<Entity, Allocator>
    ::const_reverse_iterator
entity_manager<Entity, Allocator>
    ::rend() const
noexcept
{
  return const_reverse_iterator(begin());
}


template<
    typename Entity,
    typename Allocator>
constexpr
typename entity_manager<Entity, Allocator>
    ::const_reverse_iterator
entity_manager<Entity, Allocator>
    ::crbegin() const
noexcept
{
  return rbegin();
}


template<
    typename Entity,
    typename Allocator>
constexpr
typename entity_manager<Entity, Allocator>
    ::const_reverse_iterator
entity_manager<Entity, Allocator>
    ::crend() const
noexcept
{
  return rend();
}



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
noexcept(std::is_nothrow_constructible_v<allocator_type>)
requires(std::is_constructible_v        <allocator_type>)
  : entity_manager(allocator_type())
{ }

template<
    typename Entity,
    typename Allocator>
constexpr
entity_manager<Entity, Allocator>
    ::entity_manager(
        entity_manager const &other,
        allocator_type const &alloc)
  : m_entities (other.m_entities , entity_allocator(alloc)),
    m_positions(other.m_positions, size_allocator  (alloc)),
    m_begin    (other.m_begin)
{ }

template<
    typename Entity,
    typename Allocator>
constexpr
entity_manager<Entity, Allocator>
    ::entity_manager(
        entity_manager &&     other,
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
