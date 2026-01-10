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
  using position_vector = std::vector<size_type  , size_allocator>;


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
