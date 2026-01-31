#ifndef HEIM_SPARSE_SET_BASED_STORAGE_HPP
#define HEIM_SPARSE_SET_BASED_STORAGE_HPP

#include <cstddef>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>
#include "allocator.hpp"
#include "entity.hpp"
#include "pool.hpp"
#include "type_sequence.hpp"
#include "utility.hpp"

namespace heim::sparse_set_based
{
/*!
 * @brief The main container of components, specialized for usage in the entity-component-system
 *   pattern.
 *
 * @details Manages single-component optimized pools to hold components of each type in their
 *   separate container. This allows for fast addition and removal of components on entities, and
 *   optimal iteration speed on small sets of component types.
 *
 * @tparam Entity           The entity type.
 * @tparam Allocator        The allocator type.
 * @tparam ComponentInfoSeq The component information sequence.
 *
 * @note The component information sequence should not be specialized manually.
 */
template<
    typename Entity           = entity<>,
    typename Allocator        = std::allocator<Entity>,
    typename ComponentInfoSeq = type_sequence<>>
class storage;

template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
class storage
{
public:
  using size_type       = std::size_t;
  using difference_type = std::ptrdiff_t;


  using entity_type             = Entity;
  using allocator_type          = Allocator;
  using component_info_sequence = ComponentInfoSeq;

  static_assert(
      specializes_entity_v<entity_type>,
      "entity_type must be a specialization of entity.");
  static_assert(
      is_an_allocator_for_v<allocator_type, entity_type>,
      "allocator_type must pass as an allocator of entity_type.");

private:
  struct component_info_sequence_traits
  {
  private:
    template<typename ComponentInfo>
    struct to_component
    {
      using type
      = typename ComponentInfo::template get<0>;
    };

    template<typename ComponentInfo>
    struct to_pool
    {
      using type
      = pool<
          typename ComponentInfo::template get<0>,
          entity_type,
          allocator_type,
          ComponentInfo::template get<1>::value,
          ComponentInfo::template get<2>::value>;
    };

    // makes the last component info index expression dependent, forcing its evaluation to be done
    // in the second phase of the name lookup, avoiding some obscure errors on ::paged & ::tagged.
    template<auto>
    static constexpr
    size_type
    s_dependent_last_index
    = component_info_sequence::size - 1;


    using component_sequence = typename component_info_sequence::template map<to_component>;
    using pool_sequence      = typename component_info_sequence::template map<to_pool>;

  public:
    using pool_tuple = typename pool_sequence::tuple;


    template<typename Component>
    using component
    = typename component_info_sequence
        ::template append<type_sequence<Component, size_constant<1024>, std::is_empty<Component>>>;

    template<std::size_t PageSize>
    requires(component_info_sequence::size > 0)
    using paged
    = typename component_info_sequence
        ::template set<
            s_dependent_last_index<PageSize>,
            typename component_info_sequence
                ::template get<s_dependent_last_index<PageSize>>
                ::template set<1, size_constant<PageSize>>>;

    template<bool TagValue>
    requires(component_info_sequence::size > 0)
    using tagged
    = typename component_info_sequence
        ::template set<
            s_dependent_last_index<TagValue>,
            typename component_info_sequence
                ::template get<s_dependent_last_index<TagValue>>
                ::template set<2, bool_constant<TagValue>>>;


    template<typename Component>
    requires(component_sequence::template contains<Component>)
    static constexpr
    size_type
    index
    = component_sequence::template index<Component>;

    static_assert(
        component_sequence::is_unique,
        "Component types can only be declared once in the storage.");
  };

private:
  using pool_tuple = typename component_info_sequence_traits::pool_tuple;


  template<typename Component>
  requires(requires { component_info_sequence_traits::template index<Component>; })
  static constexpr
  size_type
  s_component_index
  = component_info_sequence_traits::template index<Component>;

public:
  template<typename Component>
  using component
  = storage<
      entity_type,
      allocator_type,
      typename component_info_sequence_traits::template component<Component>>;

  template<std::size_t PageSize>
  requires(component_info_sequence::size > 0)
  using paged
  = storage<
      entity_type,
      allocator_type,
      typename component_info_sequence_traits::template paged<PageSize>>;

  template<bool TagValue>
  requires(component_info_sequence::size > 0)
  using tagged
  = storage<
      entity_type,
      allocator_type,
      typename component_info_sequence_traits::template tagged<TagValue>>;

private:
  pool_tuple
  m_pools;

private:
  static constexpr
  bool
  s_noexcept_default_construct_false()
  noexcept;

  static constexpr
  bool
  s_noexcept_default_construct()
  noexcept;

  static constexpr
  bool
  s_noexcept_move_alloc_construct()
  noexcept;

  template<std::size_t ...Is>
  static constexpr
  bool
  s_noexcept_erase_entity(std::index_sequence<Is ...>)
  noexcept;

  static constexpr
  bool
  s_noexcept_swap()
  noexcept;


  explicit constexpr
  storage(bool_constant<true>)
  noexcept;

  explicit constexpr
  storage(bool_constant<false>)
  noexcept(s_noexcept_default_construct_false());


  template<typename Component>
  [[nodiscard]] constexpr
  auto &
  m_pool()
  noexcept;

  template<typename Component>
  [[nodiscard]] constexpr
  auto const &
  m_pool() const
  noexcept;

public:
  explicit constexpr
  storage(allocator_type const &)
  noexcept;

  constexpr
  storage()
  noexcept(s_noexcept_default_construct());

  constexpr
  storage(storage const &, allocator_type const &);

  constexpr
  storage(storage const &)
  = default;

  constexpr
  storage(storage &&, allocator_type const &)
  noexcept(s_noexcept_move_alloc_construct());

  constexpr
  storage(storage &&)
  = default;

  constexpr
  ~storage()
  = default;

  constexpr storage &
  operator=(storage const &)
  = default;

  constexpr storage &
  operator=(storage &&)
  = default;

  [[nodiscard]] constexpr
  allocator_type
  get_allocator() const
  noexcept;


  constexpr
  void
  erase_entity(entity_type const)
  noexcept(s_noexcept_erase_entity(std::make_index_sequence<std::tuple_size_v<pool_tuple>>()));


  template<
      typename    Component,
      typename ...Args>
  constexpr
  auto
  emplace(entity_type const, Args &&...);


  constexpr
  void
  swap(storage &)
  noexcept(s_noexcept_swap());


  friend constexpr
  void
  swap(storage &lhs, storage &rhs)
  noexcept(s_noexcept_swap())
  {
    lhs.swap(rhs);
  }

  [[nodiscard]] friend constexpr
  bool
  operator==(storage const &, storage const &)
  = default;
};



template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
constexpr
bool
storage<Entity, Allocator, ComponentInfoSeq>
    ::s_noexcept_default_construct_false()
noexcept
{
  return std::is_nothrow_default_constructible_v<allocator_type>;
}


template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
constexpr
bool
storage<Entity, Allocator, ComponentInfoSeq>
    ::s_noexcept_default_construct()
noexcept
{
  return component_info_sequence::size > 0
      || std::is_nothrow_default_constructible_v<allocator_type>;
}


template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
constexpr
bool
storage<Entity, Allocator, ComponentInfoSeq>
    ::s_noexcept_move_alloc_construct()
noexcept
{
  return std::is_nothrow_constructible_v<
      pool_tuple,
      pool_tuple &&, allocator_type const &>;
}


template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
template<std::size_t ...Is>
constexpr
bool
storage<Entity, Allocator, ComponentInfoSeq>
    ::s_noexcept_erase_entity(std::index_sequence<Is...>)
noexcept
{
  return
     (noexcept(std::get<Is>(std::declval<pool_tuple &>()).erase(std::declval<entity_type const>()))
   && ...);
}

template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
constexpr
bool
storage<Entity, Allocator, ComponentInfoSeq>
    ::s_noexcept_swap()
noexcept
{
  return std::is_nothrow_swappable_v<pool_tuple>;
}



template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
constexpr
storage<Entity, Allocator, ComponentInfoSeq>
    ::storage(bool_constant<true>)
noexcept
  : m_pools()
{ }

template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
constexpr
storage<Entity, Allocator, ComponentInfoSeq>
    ::storage(bool_constant<false>)
noexcept(s_noexcept_default_construct_false())
  : storage(allocator_type())
{ }



template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
template<typename Component>
constexpr
auto &
storage<Entity, Allocator, ComponentInfoSeq>
    ::m_pool()
noexcept
{
  return std::get<s_component_index<Component>>(m_pools);
}

template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
template<typename Component>
constexpr
auto const &
storage<Entity, Allocator, ComponentInfoSeq>
    ::m_pool() const
noexcept
{
  return std::get<s_component_index<Component>>(m_pools);
}



template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
constexpr
storage<Entity, Allocator, ComponentInfoSeq>
    ::storage(allocator_type const &alloc)
noexcept
  : m_pools(std::allocator_arg, alloc)
{
  static_assert(
      component_info_sequence::size > 0,
      "A storage with no component types does not hold any allocator.");
}

template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
constexpr
storage<Entity, Allocator, ComponentInfoSeq>
    ::storage()
noexcept(s_noexcept_default_construct())
  : storage(bool_constant<(component_info_sequence::size > 0)>())
{ }

template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
constexpr
storage<Entity, Allocator, ComponentInfoSeq>
    ::storage(storage const &other, allocator_type const &alloc)
  : m_pools(std::allocator_arg, alloc, other.m_pools)
{
  static_assert(
      component_info_sequence::size > 0,
      "A storage with no component types does not hold any allocator.");
}

template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
constexpr
storage<Entity, Allocator, ComponentInfoSeq>
    ::storage(storage &&other, allocator_type const &alloc)
noexcept(s_noexcept_move_alloc_construct())
  : m_pools(std::allocator_arg, alloc, std::move(other.m_pools))
{
  static_assert(
      component_info_sequence::size > 0,
      "A storage with no component types does not hold any allocator.");
}


template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
constexpr
typename storage<Entity, Allocator, ComponentInfoSeq>
    ::allocator_type
storage<Entity, Allocator, ComponentInfoSeq>
    ::get_allocator() const
noexcept
{
  static_assert(
      component_info_sequence::size > 0,
      "A storage with no component types does not hold any allocator.");

  return allocator_type(std::get<0>(m_pools).get_allocator());
}



template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
constexpr
void
storage<Entity, Allocator, ComponentInfoSeq>
    ::erase_entity(entity_type const e)
noexcept(s_noexcept_erase_entity(std::make_index_sequence<std::tuple_size_v<pool_tuple>>()))
{
  std::apply(
      [e](auto &...pools)
      {
        (pools.erase(e), ...);
      },
      m_pools);
}



template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
template<
    typename    Component,
    typename ...Args>
constexpr
auto
storage<Entity, Allocator, ComponentInfoSeq>
    ::emplace(entity_type const e, Args &&...args)
{
  return std::get<s_component_index<Component>>(m_pools).emplace(e, std::forward<Args>(args)...);
}



template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
constexpr
void
storage<Entity, Allocator, ComponentInfoSeq>
    ::swap(storage &other)
noexcept(s_noexcept_swap())
{
  std::swap(m_pools, other.m_pools);
}



/*!
 * @brief Determines whether the given type is a specialization of storage.
 *
 * @tparam T The type to determine for.
 */
template<typename T>
struct specializes_storage;

template<typename T>
inline constexpr
bool
specializes_storage_v
= specializes_storage<T>::value;

template<typename T>
struct specializes_storage
  : bool_constant<false>
{ };

template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
struct specializes_storage<
    storage<Entity, Allocator, ComponentInfoSeq>>
  : bool_constant<true>
{ };


} // namespace heim::sparse_set_based

#endif // HEIM_SPARSE_SET_BASED_STORAGE_HPP