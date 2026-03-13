#ifndef HEIM_SPARSE_SET_BASED_NEW_STORAGE_HPP
#define HEIM_SPARSE_SET_BASED_NEW_STORAGE_HPP

#include <cstddef>
#include <memory>
#include "heim/allocator.hpp"
#include "heim/identifier.hpp"
#include "heim/type_sequence.hpp"
#include "pool.hpp"
#include "sparse_set.hpp"

namespace heim::sparse_set_based
{

template<
    typename    Component,
    std::size_t PageSize,
    bool        TagValue>
using component_info
= type_sequence<Component, size_constant<PageSize>, bool_constant<TagValue>>;

template<typename ...Components>
using group_info
= type_sequence<Components ...>;

template<typename T>
struct is_group_info;

template<typename T>
struct is_group_info
  : bool_constant<false>
{ };

template<
    typename    First,
    typename    Second,
    typename ...Rest>
struct is_group_info<
    type_sequence<First, Second, Rest ...>>
  : bool_constant<true>
{ };




/*!
 * @brief The main container of components, specialized for usage in the entity-component-system
 *   pattern.
 *
 * @details Manages single-component optimized pools to hold components of each type in their
 *   separate container. This allows for fast addition and removal of components on entities, and
 *   optimal iteration speed on small sets of component types.
 *
 * @tparam Identifier       The identifier type.
 * @tparam Allocator        The allocator  type.
 * @tparam ComponentInfoSeq The component information sequence.
 * @tparam GroupInfoSeq     The group     information sequence.
 *
 * @note The component and group information sequences should not be specialized manually.
 */
template<
    typename Identifier       = identifier<>,
    typename Allocator        = std::allocator<Identifier>,
    typename ComponentInfoSeq = type_sequence<>,
    typename GroupInfoSeq     = type_sequence<>>
class new_storage;

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
class new_storage
{
public:
  using identifier_type         = Identifier;
  using allocator_type          = Allocator;
  using component_info_sequence = ComponentInfoSeq;
  using group_info_sequence     = GroupInfoSeq;

  static_assert(
      specializes_identifier_v<identifier_type>,
      "heim::sparse_set_based::new_storage: identifier_type must be a specialization of identifier.");
  static_assert(
      is_an_allocator_for_v<allocator_type, identifier_type>,
      "heim::sparse_set_based::new_storage: allocator_type must pass as an allocator of identifier_type.");
  static_assert(
      specializes_type_sequence_v<component_info_sequence>,
      "heim::sparse_set_based::new_storage: component_info_sequence must be a specialization of type_sequence.");
  static_assert(
      specializes_type_sequence_v<group_info_sequence>,
      "heim::sparse_set_based::new_storage: group_info_sequence must be a specialization of type_sequence.");

  using size_type       = std::size_t;
  using difference_type = std::ptrdiff_t;

private:
  template<auto>
  static constexpr
  size_type
  s_last_component_index
  = component_info_sequence::size - 1;


  template<typename ComponentInfo>
  struct to_component
  {
    using type
    = typename ComponentInfo::template get<0>;
  };

  template<typename ComponentInfo>
  struct to_container
  {
    using type
    = std::conditional_t<
        ComponentInfo::template get<2>::value,
        sparse_set<
            identifier_type,
            ComponentInfo::template get<1>::value,
            allocator_type>,
        pool<
            typename to_component<ComponentInfo>::type,
            identifier_type,
            ComponentInfo::template get<1>::value,
            allocator_type>>;
  };

  template<typename GroupInfo>
  struct to_group
  {
    using type
    = size_type;
  };


  using component_sequence = typename component_info_sequence::template map<to_component>;
  using container_sequence = typename component_info_sequence::template map<to_container>;
  using group_sequence     = typename group_info_sequence    ::template map<to_group    >;

  static_assert(
      std::is_same_v<
          component_sequence,
          typename component_sequence::template map<std::remove_cvref>>,
      "heim::sparse_set_based::new_storage: component types must each neither be const nor contain references.");
  static_assert(
      component_sequence::is_unique,
      "heim::sparse_set_based::new_storage: component types must each be unique.");

  static_assert(
      std::is_same_v<
          group_info_sequence,
          typename group_info_sequence::template filter<specializes_type_sequence>>,
      "heim::sparse_set_based::new_storage: group_info_sequence must only contain specializations of type_sequence.");
  static_assert(
      group_info_sequence::flatten::template difference<component_sequence>::size == 0,
      "heim::sparse_set_based::new_storage: each component type in group_info types must be unique and "
          "already managed by the storage.");
  static_assert(
      std::is_same_v<
          group_info_sequence,
          typename group_info_sequence::template filter<is_group_info>>,
      "heim::sparse_set_based::new_storage: group_info types in group_info_sequence must contain at least 2 component types.");

  using container_tuple = typename component_info_sequence::template map<to_container>::tuple;
  using group_tuple     = typename group_info_sequence    ::template map<to_group    >::tuple;


  template<typename Component>
  static constexpr
  size_type
  component_index
  = component_sequence::template index<Component>;

  template<typename Component>
  struct meta
  {
    template<typename ComponentInfo>
    struct contains
    {
      using type
      = std::conditional_t<
          ComponentInfo::template contains<Component>,
          bool_constant<true >,
          bool_constant<false>>;
    };
  };

  template<typename Component>
  static constexpr
  bool
  is_grouped
  = group_info_sequence
      ::template map     <meta<Component>::template contains>
      ::template contains<bool_constant<true>>;

  template<typename Component>
  static constexpr
  size_type
  group_index
  = group_info_sequence
      ::template map  <meta<Component>::template contains>
      ::template index<bool_constant<true>>;


  template<typename Component>
  using container_of
  = typename container_sequence::template get<component_index<Component>>;

  template<typename Component>
  using group_info_of
  = typename group_info_sequence::template get<group_index<Component>>;

public:
  template<
      typename  Component,
      size_type PageSize  = default_container_page_size<>::value,
      bool      TagValue  = std::is_empty_v<Component>>
  using component
  = new_storage<
      identifier_type,
      allocator_type,
      typename component_info_sequence
          ::template append<component_info<Component, PageSize, TagValue>>,
      group_info_sequence>;

  template<size_type PageSize>
  using paged
  = new_storage<
      identifier_type,
      allocator_type,
      typename component_info_sequence
          ::template set<
              s_last_component_index<PageSize>,
              typename component_info_sequence
                  ::template get<s_last_component_index<PageSize>>
                  ::template set<1, size_constant<PageSize>>>,
      group_info_sequence>;

  template<bool TagValue>
  using tagged
  = new_storage<
      identifier_type,
      allocator_type,
      typename component_info_sequence
          ::template set<
              s_last_component_index<TagValue>,
              typename component_info_sequence
                  ::template get<s_last_component_index<TagValue>>
                  ::template set<2, bool_constant<TagValue>>>,
      group_info_sequence>;

  template<typename ...Components>
  using group
  = new_storage<
      identifier_type,
      allocator_type,
      component_info_sequence,
      typename group_info_sequence
          ::template append<group_info<Components ...>>>;

private:
  container_tuple m_containers;
  group_tuple     m_groups;

private:
  template<typename Component>
  static constexpr
  bool
  s_noexcept_m_group_swap()
  noexcept;

  template<typename ...Components> static constexpr bool s_noexcept_m_include(group_info<Components ...>) noexcept;
  template<typename ...Components> static constexpr bool s_noexcept_m_exclude(group_info<Components ...>) noexcept;

  template<typename ...Components>
  static constexpr
  bool
  s_noexcept_m_erase(type_sequence<Components ...>)
  noexcept;

  static constexpr bool s_noexcept_default_construct_false() noexcept;
  static constexpr bool s_noexcept_default_construct      () noexcept;
  static constexpr bool s_noexcept_move_alloc_construct   () noexcept;
  static constexpr bool s_noexcept_swap() noexcept;

  template<typename Component> static constexpr bool s_noexcept_erase    () noexcept;
  template<typename Component> static constexpr bool s_noexcept_try_erase() noexcept;

  static constexpr
  bool
  s_noexcept_erase()
  noexcept;


  constexpr explicit new_storage(bool_constant<true >) noexcept;
  constexpr explicit new_storage(bool_constant<false>) noexcept(s_noexcept_default_construct_false());


  template<typename Component> [[nodiscard]] constexpr auto       &m_container()       noexcept;
  template<typename Component> [[nodiscard]] constexpr auto const &m_container() const noexcept;

  template<typename Component> [[nodiscard]] constexpr size_type &m_group()       noexcept;
  template<typename Component> [[nodiscard]] constexpr size_type  m_group() const noexcept;


  template<typename ...Components>
  [[nodiscard]] constexpr
  bool
  m_has(group_info<Components ...>, identifier_type) const
  noexcept;

  template<typename Component>
  constexpr
  void
  m_group_swap(identifier_type, difference_type)
  noexcept(s_noexcept_m_group_swap<Component>());

  template<typename ...Components>
  constexpr
  void
  m_include(group_info<Components ...>, identifier_type)
  noexcept(s_noexcept_m_include(group_info<Components ...>{}));

  template<typename ...Components>
  constexpr
  void
  m_exclude(group_info<Components ...>, identifier_type)
  noexcept(s_noexcept_m_exclude(group_info<Components ...>{}));


  template<typename ...Components>
  constexpr
  void
  m_erase(type_sequence<Components ...>, identifier_type)
  noexcept(s_noexcept_m_erase(type_sequence<Components ...>{}));

public:
  constexpr explicit
  new_storage(allocator_type const &)
  noexcept;

  constexpr
  new_storage()
  noexcept(s_noexcept_default_construct());

  constexpr
  new_storage(new_storage const &, allocator_type const &);

  constexpr
  new_storage(new_storage const &)
  = default;

  constexpr
  new_storage(new_storage &&, allocator_type const &)
  noexcept(s_noexcept_move_alloc_construct());

  constexpr
  new_storage(new_storage &&)
  = default;

  constexpr
  ~new_storage()
  = default;

  constexpr new_storage &operator=(new_storage const &) = default;
  constexpr new_storage &operator=(new_storage &&)      = default;

  [[nodiscard]] constexpr
  allocator_type
  get_allocator() const
  noexcept;

  constexpr
  void
  swap(new_storage &)
  noexcept(s_noexcept_swap());


  template<typename Component>
  [[nodiscard]] constexpr
  bool
  has(identifier_type) const
  noexcept;

  template<typename Component> [[nodiscard]] constexpr Component       &get(identifier_type)       noexcept;
  template<typename Component> [[nodiscard]] constexpr Component const &get(identifier_type) const noexcept;

  template<typename Expression> [[nodiscard]] constexpr auto query()       noexcept;
  template<typename Expression> [[nodiscard]] constexpr auto query() const noexcept;


  template<typename Component, typename ...Args> constexpr void emplace    (identifier_type, Args &&...);
  template<typename Component, typename ...Args> constexpr bool try_emplace(identifier_type, Args &&...);

  template<typename Component> constexpr bool insert(identifier_type, Component const &);
  template<typename Component> constexpr bool insert(identifier_type, Component &&     );

  template<typename Component> constexpr bool insert_or_assign(identifier_type, Component const &);
  template<typename Component> constexpr bool insert_or_assign(identifier_type, Component &&     );

  template<typename Component> constexpr void erase    (identifier_type) noexcept(s_noexcept_erase    <Component>());
  template<typename Component> constexpr bool try_erase(identifier_type) noexcept(s_noexcept_try_erase<Component>());

  constexpr
  void
  erase(identifier_type)
  noexcept(s_noexcept_erase());

  constexpr
  void
  clear()
  noexcept;


  friend constexpr
  void
  swap(new_storage &lhs, new_storage &rhs)
  noexcept(s_noexcept_swap())
  {
    lhs.swap(rhs);
  }

  [[nodiscard]]
  friend constexpr
  bool
  operator==(new_storage const &, new_storage const &)
  = default;
};


template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<typename Component>
constexpr
bool
new_storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::s_noexcept_m_group_swap()
noexcept
{
  return noexcept(std::declval<container_of<Component> &>().swap(std::declval<identifier_type>(), std::declval<identifier_type>()));
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<typename ...Components>
constexpr
bool
new_storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::s_noexcept_m_include(group_info<Components ...>)
noexcept
{
  return (s_noexcept_m_group_swap<Components>() && ...);
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<typename ...Components>
constexpr
bool
new_storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::s_noexcept_m_exclude(group_info<Components ...>)
noexcept
{
  return (s_noexcept_m_group_swap<Components>() && ...);
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<typename ...Components>
constexpr
bool
new_storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::s_noexcept_m_erase(type_sequence<Components ...>)
noexcept
{
  return (s_noexcept_try_erase<Components>() && ...);
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
constexpr
bool
new_storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::s_noexcept_default_construct_false()
noexcept
{
  return std::is_nothrow_default_constructible_v<allocator_type>;
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
constexpr
bool
new_storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::s_noexcept_default_construct()
noexcept
{
  return component_info_sequence::size == 0
      || s_noexcept_default_construct_false();
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
constexpr
bool
new_storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::s_noexcept_move_alloc_construct()
noexcept
{
  return
      std::is_nothrow_constructible_v<
          container_tuple,
          std::allocator_arg_t, allocator_type const &, container_tuple &&>
   && std::is_nothrow_constructible_v<
          group_tuple,
          group_tuple &&>;
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
constexpr
bool
new_storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::s_noexcept_swap()
noexcept
{
  return std::is_nothrow_swappable_v<container_tuple>
      && std::is_nothrow_swappable_v<group_tuple>;
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<typename Component>
constexpr
bool
new_storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::s_noexcept_erase()
noexcept
{
  if constexpr (is_grouped<Component>)
  {
    return s_noexcept_m_exclude(group_info_of<Component>{})
        && noexcept(std::declval<container_of<Component> &>().erase(std::declval<identifier_type>()));
  }
  else
    return noexcept(std::declval<container_of<Component> &>().erase(std::declval<identifier_type>()));
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<typename Component>
constexpr
bool
new_storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::s_noexcept_try_erase()
noexcept
{
  if constexpr (is_grouped<Component>)
  {
    return s_noexcept_m_exclude(group_info_of<Component>{})
        && noexcept(std::declval<container_of<Component> &>().erase    (std::declval<identifier_type>()))
        && noexcept(std::declval<container_of<Component> &>().try_erase(std::declval<identifier_type>()));
  }
  else
    return noexcept(std::declval<container_of<Component> &>().try_erase(std::declval<identifier_type>()));
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
constexpr
bool
new_storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::s_noexcept_erase()
noexcept
{
  return s_noexcept_m_erase(component_sequence{});
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
constexpr
new_storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::new_storage(bool_constant<true>)
noexcept
  : m_containers(),
    m_groups    ()
{ }

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
constexpr
new_storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::new_storage(bool_constant<false>)
noexcept(s_noexcept_default_construct_false())
  : new_storage(allocator_type{})
{ }

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<typename Component>
constexpr
auto &
new_storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::m_container()
noexcept
{
  return std::get<component_index<Component>>(m_containers);
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<typename Component>
constexpr
auto const &
new_storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::m_container() const
noexcept
{
  return std::get<component_index<Component>>(m_containers);
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<typename Component>
constexpr
typename new_storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::size_type &
new_storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::m_group()
noexcept
{
  return std::get<group_index<Component>>(m_groups);
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<typename Component>
constexpr
typename new_storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::size_type
new_storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::m_group() const
noexcept
{
  return std::get<group_index<Component>>(m_groups);
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<typename ...Components>
constexpr
bool
new_storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::m_has(group_info<Components ...>, identifier_type const id) const
noexcept
{
  return (m_container<Components>().contains(id) && ...);
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<typename Component>
constexpr
void
new_storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::m_group_swap(identifier_type const id, difference_type const group)
noexcept(s_noexcept_m_group_swap<Component>())
{
  using sparse_set_type
  = sparse_set<identifier_type, container_of<Component>::page_size, allocator_type>;

  container_of<Component> &
  container
  = m_container<Component>();

  container.swap(id, *(static_cast<sparse_set_type &>(container).begin() + group));
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<typename ...Components>
constexpr
void
new_storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::m_include(group_info<Components ...>, identifier_type const id)
noexcept(s_noexcept_m_include(group_info<Components ...>{}))
{
  size_type &
  group
  = m_group<typename group_info<Components ...>::template get<0>>();

  (m_group_swap<Components>(id, static_cast<difference_type>(group)), ...);
  ++group;
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<typename ...Components>
constexpr
void
new_storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::m_exclude(group_info<Components ...>, identifier_type const id)
noexcept(s_noexcept_m_exclude(group_info<Components ...>{}))
{
  size_type &
  group
  = m_group<typename group_info<Components ...>::template get<0>>();

  (m_group_swap<Components>(id, static_cast<difference_type>(group) - 1), ...);
  --group;
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<typename ...Components>
constexpr
void
new_storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::m_erase(type_sequence<Components ...>, identifier_type const id)
noexcept(s_noexcept_m_erase(type_sequence<Components ...>{}))
{
  (try_erase<Components>(id), ...);
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
constexpr
new_storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::new_storage(allocator_type const &alloc)
noexcept
  : m_containers(std::allocator_arg, alloc),
    m_groups    ()
{
  static_assert(
      component_info_sequence::size > 0,
      "heim::sparse_set_based::storage::storage: A storage with no component type cannot hold any allocator.");
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
constexpr
new_storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::new_storage()
noexcept(s_noexcept_default_construct())
  : new_storage(bool_constant<component_info_sequence::size == 0>{})
{ }

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
constexpr
new_storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::new_storage(new_storage const &other, allocator_type const &alloc)
  : m_containers(std::allocator_arg, alloc, other.m_containers),
    m_groups    (other.m_groups)
{ }

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
constexpr
new_storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::new_storage(new_storage &&other, allocator_type const &alloc)
noexcept(s_noexcept_move_alloc_construct())
  : m_containers(std::allocator_arg, alloc, std::move(other.m_containers)),
    m_groups    (std::move(other.m_groups))
{ }

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
constexpr
typename new_storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::allocator_type
new_storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::get_allocator() const
noexcept
{
  static_assert(
      component_info_sequence::size > 0,
      "heim::sparse_set_based::storage::get_allocator: A storage with no component type cannot hold any allocator.");

  return std::get<0>(m_containers).get_allocator();
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
constexpr
void
new_storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::swap(new_storage &other)
noexcept(s_noexcept_swap())
{
  using std::swap;

  swap(m_containers, other.m_containers);
  swap(m_groups    , other.m_groups    );
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<typename Component>
constexpr
bool
new_storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::has(identifier_type const id) const
noexcept
{
  return m_container<Component>().contains(id);
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<typename Component>
constexpr
Component &
new_storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::get(identifier_type const id)
noexcept
{
  return m_container<Component>()[id];
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<typename Component>
constexpr
Component const &
new_storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::get(identifier_type const id) const
noexcept
{
  return m_container<Component>()[id];
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<typename Expression>
constexpr
auto
new_storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::query()
noexcept
{
  // TODO: implement
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<typename Expression>
constexpr
auto
new_storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::query() const
noexcept
{
  // TODO: implement
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<
    typename    Component,
    typename ...Args>
constexpr
void
new_storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::emplace(identifier_type const id, Args &&...args)
{
  m_container<Component>().emplace(id, std::forward<Args>(args)...);

  if constexpr (is_grouped<Component>)
  {
    if (m_has(group_info_of<Component>{}, id))
      m_include(group_info_of<Component>{}, id);
  }
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<
    typename    Component,
    typename ...Args>
constexpr
bool
new_storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::try_emplace(identifier_type const id, Args &&...args)
{
  bool const
  ret
  = m_container<Component>().try_emplace(id, std::forward<Args>(args)...).second;

  if constexpr (is_grouped<Component>)
  {
    if (ret && m_has(group_info_of<Component>{}, id))
      m_include(group_info_of<Component>{}, id);
  }

  return ret;
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<typename Component>
constexpr
bool
new_storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::insert(identifier_type const id, Component const &c)
{
  bool const
  ret
  = m_container<Component>().insert(id, c).second;

  if constexpr (is_grouped<Component>)
  {
    if (ret && m_has(group_info_of<Component>{}, id))
      m_include(group_info_of<Component>{}, id);
  }

  return ret;
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<typename Component>
constexpr
bool
new_storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::insert(identifier_type const id, Component &&c)
{
  bool const
  ret
  = m_container<Component>().insert(id, std::forward<Component>(c)).second;

  if constexpr (is_grouped<Component>)
  {
    if (ret && m_has(group_info_of<Component>{}, id))
      m_include(group_info_of<Component>{}, id);
  }

  return ret;
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<typename Component>
constexpr
bool
new_storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::insert_or_assign(identifier_type const id, Component const &c)
{
  bool const
  ret
  = m_container<Component>().insert_or_assign(id, c).second;

  if constexpr (is_grouped<Component>)
  {
    if (ret && m_has(group_info_of<Component>{}, id))
      m_include(group_info_of<Component>{}, id);
  }

  return ret;
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<typename Component>
constexpr
bool
new_storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::insert_or_assign(identifier_type const id, Component &&c)
{
  bool const
  ret
  = m_container<Component>().insert_or_assign(id, std::forward<Component>(c)).second;

  if constexpr (is_grouped<Component>)
  {
    if (ret && m_has(group_info_of<Component>{}, id))
      m_include(group_info_of<Component>{}, id);
  }

  return ret;
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<typename Component>
constexpr
void
new_storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::erase(identifier_type const id)
noexcept(s_noexcept_erase<Component>())
{
  if constexpr (is_grouped<Component>)
  {
    if (m_has(group_info_of<Component>{}, id))
      m_exclude(group_info_of<Component>{}, id);
  }

  m_container<Component>().erase(id);
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<typename Component>
constexpr
bool
new_storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::try_erase(identifier_type const id)
noexcept(s_noexcept_try_erase<Component>())
{
  if constexpr (is_grouped<Component>)
  {
    if (m_has(group_info_of<Component>{}, id))
    {
      m_exclude(group_info_of<Component>{}, id);
      m_container<Component>().erase(id);
      return true;
    }
  }

  return m_container<Component>().try_erase(id);
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
constexpr
void
new_storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::erase(identifier_type const id)
noexcept(s_noexcept_erase())
{
  m_erase(component_sequence{}, id);
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
constexpr
void
new_storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::clear()
noexcept
{
  std::apply([](auto &...containers) { (containers.clear(), ...); }, m_containers);
  std::apply([](auto &...groups    ) { ((groups = 0)      , ...); }, m_groups    );
}


}

#endif // HEIM_SPARSE_SET_BASED_NEW_STORAGE_HPP