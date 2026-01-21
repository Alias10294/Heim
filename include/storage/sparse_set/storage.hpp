#ifndef HEIM_STORAGE_HPP
#define HEIM_STORAGE_HPP

#include <cstddef>
#include <memory>
#include <tuple>
#include <type_traits>
#include "fwd.hpp"
#include "pool.hpp"
#include "type_sequence.hpp"

namespace heim::sparse_set_based
{
struct empty_storage_tag
{ };



template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
class storage
{
public:
  using entity_type             = Entity;
  using allocator_type          = Allocator;
  using component_info_sequence = ComponentInfoSeq;

  using size_type       = std::size_t;
  using difference_type = std::ptrdiff_t;

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

  public:
    using component_sequence = typename component_info_sequence::template map<to_component>;
    using pool_sequence      = typename component_info_sequence::template map<to_pool>;

    using pool_tuple = typename pool_sequence::tuple;


    template<typename Component>
    static constexpr
    size_type
    component_index
    = component_sequence::template index<Component>;

    template<typename Component>
    using pool_type_for
    = std::tuple_element_t<component_index<Component>, pool_tuple>;

  public:
    template<typename Component>
    using component
    = std::conditional_t<
        std::is_same_v<
            component_info_sequence,
            empty_storage_component_info_sequence>,
        type_sequence<
            type_sequence<Component, size_constant<1024>, std::is_empty<Component>>>,
        typename component_info_sequence
            ::template append<
                type_sequence<Component, size_constant<1024>, std::is_empty<Component>>>>;

    template<std::size_t PageSize>
    using paged
    = typename component_info_sequence
        ::template set<
            component_info_sequence::size - 1,
            typename component_info_sequence
                ::template get<component_info_sequence::size - 1>
                ::template set<1, size_constant<PageSize>>>;

    template<bool TagValue>
    using tagged
    = typename component_info_sequence
        ::template set<
            component_info_sequence::size - 1,
            typename component_info_sequence
                ::template get<component_info_sequence::size - 1>
                ::template set<2, bool_constant<TagValue>>>;
  };


  using pool_tuple
  = typename component_info_sequence_traits::pool_tuple;

  template<typename Component>
  static constexpr
  size_type
  s_component_index
  = component_info_sequence_traits::template component_index<Component>;

public:
  template<typename Component>
  using pool_type_for
  = typename component_info_sequence_traits::template pool_type_for<Component>;


  template<typename Component>
  using component
  = storage<
      entity_type,
      allocator_type,
      typename component_info_sequence_traits::template component<Component>>;

  template<size_type PageSize>
  using paged
  = storage<
      entity_type,
      allocator_type,
      typename component_info_sequence_traits::template paged<PageSize>>;

  using unpaged
  = storage<
      entity_type,
      allocator_type,
      typename component_info_sequence_traits::template paged<0>>;

  using tagged
  = storage<
      entity_type,
      allocator_type,
      typename component_info_sequence_traits::template tagged<true>>;

  using untagged
  = storage<
      entity_type,
      allocator_type,
      typename component_info_sequence_traits::template tagged<false>>;

private:
  pool_tuple m_pools;

private:
  template<typename Component>
  [[nodiscard]] constexpr
  pool_type_for<Component> &
  m_pool()
  noexcept;

  template<typename Component>
  [[nodiscard]] constexpr
  pool_type_for<Component> const &
  m_pool() const
  noexcept;


  template<std::size_t ...Is>
  static consteval
  bool
  s_clear_noexcept_cond(std::index_sequence<Is ...>)
  noexcept;

  template<std::size_t ...Is>
  static consteval
  bool
  s_erase_noexcept_cond(std::index_sequence<Is ...>)
  noexcept;

public:
  explicit constexpr
  storage(allocator_type const &)
  noexcept;

  constexpr
  storage()
  noexcept(std::is_nothrow_default_constructible_v<allocator_type>);

  constexpr
  storage(storage const &, allocator_type const &);

  constexpr
  storage(storage const &)
  = default;

  constexpr
  storage(storage &&, allocator_type const &)
  noexcept(std::is_nothrow_constructible_v<pool_tuple, std::allocator_arg_t, allocator_type const &, pool_tuple &&>);

  constexpr
  storage(storage &&)
  = default;

  constexpr
  ~storage()
  = default;

  constexpr
  storage &
  operator=(storage const &)
  = default;

  constexpr
  storage &
  operator=(storage &&)
  = default;

  [[nodiscard]] constexpr
  allocator_type
  get_allocator() const
  noexcept;



  [[nodiscard]] constexpr
  bool
  empty() const
  noexcept;

  template<typename Component>
  [[nodiscard]] constexpr
  bool
  contains(entity_type const) const
  noexcept;

  template<typename Component>
  [[nodiscard]] constexpr
  typename pool_type_for<Component>::iterator
  find(entity_type const)
  noexcept;

  template<typename Component>
  [[nodiscard]] constexpr
  typename pool_type_for<Component>::const_iterator
  find(entity_type const) const
  noexcept;


  constexpr
  void
  clear()
  noexcept(s_clear_noexcept_cond(std::make_index_sequence<std::tuple_size_v<pool_tuple>>{}));

  template<typename Component>
  constexpr
  bool
  erase(entity_type const)
  noexcept(noexcept(m_pool<Component>().erase(std::declval<entity_type const>())));

  constexpr
  bool
  erase(entity_type const)
  noexcept(s_erase_noexcept_cond(std::make_index_sequence<std::tuple_size_v<pool_tuple>>{}));

  template<
      typename    Component,
      typename ...Args>
  constexpr
  std::pair<typename pool_type_for<Component>::iterator, bool>
  emplace(entity_type const, Args &&...);

  constexpr
  void
  swap(storage &)
  noexcept(std::is_nothrow_swappable_v<pool_tuple>);


  friend constexpr
  void
  swap(storage &lhs, storage &rhs)
  noexcept(noexcept(lhs.swap(rhs)))
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
template<typename Component>
constexpr
typename storage<Entity, Allocator, ComponentInfoSeq>
    ::template pool_type_for<Component> &
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
typename storage<Entity, Allocator, ComponentInfoSeq>
    ::template pool_type_for<Component> const &
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
template<std::size_t ...Is>
consteval
bool
storage<Entity, Allocator, ComponentInfoSeq>
    ::s_clear_noexcept_cond(std::index_sequence<Is...>)
noexcept
{
  return (noexcept(std::declval<std::tuple_element_t<Is, pool_tuple> &>().clear()) && ...);
}


template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
template<std::size_t... Is>
consteval
bool
storage<Entity, Allocator, ComponentInfoSeq>
    ::s_erase_noexcept_cond(std::index_sequence<Is...>)
noexcept
{
  return (noexcept(std::declval<std::tuple_element_t<Is, pool_tuple> &>().erase(std::declval<entity_type const>())) || ...);
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
{ }

template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
constexpr
storage<Entity, Allocator, ComponentInfoSeq>
    ::storage()
noexcept(std::is_nothrow_default_constructible_v<allocator_type>)
  : storage(allocator_type())
{ }

template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
constexpr storage<Entity, Allocator, ComponentInfoSeq>
    ::storage(storage const &other, allocator_type const &alloc)
  : m_pools(std::allocator_arg, alloc, other.m_pools)
{ }

template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
constexpr storage<Entity, Allocator, ComponentInfoSeq>
    ::storage(storage &&other, allocator_type const &alloc)
noexcept(std::is_nothrow_constructible_v<pool_tuple, std::allocator_arg_t, allocator_type const &, pool_tuple &&>)
  : m_pools(std::allocator_arg, alloc, std::move(other.m_pools))
{ }



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
  return std::get<0>(m_pools).get_allocator();
}



template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
constexpr
bool
storage<Entity, Allocator, ComponentInfoSeq>
    ::empty() const
noexcept
{
  return std::apply(
      [](auto &...pools)
      {
        return (pools.empty() && ...);
      },
      m_pools);
}


template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
template<typename Component>
constexpr
bool
storage<Entity, Allocator, ComponentInfoSeq>
    ::contains(entity_type const e) const
noexcept
{
  return m_pool<Component>().contains(e);
}

template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
template<typename Component>
constexpr
typename storage<Entity, Allocator, ComponentInfoSeq>
    ::template pool_type_for<Component>
    ::iterator
storage<Entity, Allocator, ComponentInfoSeq>
    ::find(entity_type const e)
noexcept
{
  return m_pool<Component>().find(e);
}

template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
template<typename Component>
constexpr
typename storage<Entity, Allocator, ComponentInfoSeq>
    ::template pool_type_for<Component>
    ::const_iterator
storage<Entity, Allocator, ComponentInfoSeq>
    ::find(entity_type const e) const
noexcept
{
  return m_pool<Component>().find(e);
}

template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
constexpr
void
storage<Entity, Allocator, ComponentInfoSeq>
    ::clear()
noexcept(s_clear_noexcept_cond(std::make_index_sequence<std::tuple_size_v<pool_tuple>>{}))
{
  std::apply(
      [](auto &...pools)
      {
        (pools.clear(), ...);
      },
      m_pools);
}

template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
template<typename Component>
constexpr
bool
storage<Entity, Allocator, ComponentInfoSeq>
    ::erase(entity_type const e)
noexcept(noexcept(m_pool<Component>().erase(std::declval<entity_type const>())))
{
  return m_pool<Component>().erase(e);
}

template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
constexpr
bool
storage<Entity, Allocator, ComponentInfoSeq>
    ::erase(entity_type const e)
noexcept(s_erase_noexcept_cond(std::make_index_sequence<std::tuple_size_v<pool_tuple>>{}))
{
  return std::apply(
      [e](auto &...pools)
      {
        return (pools.erase(e) || ...);
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
std::pair<
    typename storage<Entity, Allocator, ComponentInfoSeq>
        ::template pool_type_for<Component>
        ::iterator,
    bool>
storage<Entity, Allocator, ComponentInfoSeq>
    ::emplace(entity_type const e, Args &&...args)
{
  return m_pool<Component>().emplace(e, std::forward<Args>(args)...);
}

template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
constexpr
void
storage<Entity, Allocator, ComponentInfoSeq>
    ::swap(storage &other)
noexcept(std::is_nothrow_swappable_v<pool_tuple>)
{
  m_pools.swap(other.m_pools);
}



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


}

#endif // HEIM_STORAGE_HPP