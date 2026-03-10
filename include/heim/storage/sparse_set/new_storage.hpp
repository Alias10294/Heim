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

  using size_type       = std::size_t;
  using difference_type = std::ptrdiff_t;

private:
  struct utility
  {
    struct component
    {
    private:
      template<auto>
      constexpr static
      size_type
      s_last_component_index
      = component_info_sequence::size - 1;

      template<typename CInfo> struct to_component_type { using type = typename CInfo::template get<0>; };
      template<typename CInfo> struct to_page_size_type { using type = typename CInfo::template get<1>; };
      template<typename CInfo> struct to_tag_value_type { using type = typename CInfo::template get<2>; };

      template<typename CInfo>
      struct to_container_type
      {
        using type
        = std::conditional_t<
            to_tag_value_type<CInfo>::type::value,
            sparse_set<
                identifier_type,
                to_page_size_type<CInfo>::type::value,
                allocator_type>,
            pool<
                to_component_type<CInfo>,
                identifier_type,
                to_page_size_type<CInfo>::type::value,
                allocator_type>>;
      };

    public:
      template<
          typename  Component,
          size_type PageSize  = default_container_page_size<>::value,
          bool      TagValue  = std::is_empty_v<Component>>
      using use_component
      = typename component_info_sequence
          ::template append<type_sequence<Component, size_constant<PageSize>, bool_constant<TagValue>>>;

      template<size_type PageSize>
      using use_paged
      = typename component_info_sequence
          ::template set<
              s_last_component_index<PageSize>,
              typename component_info_sequence
                  ::template get<s_last_component_index<PageSize>>
                  ::template set<1, size_constant<PageSize>>>;

      template<bool TagValue>
      using use_tagged
      = typename component_info_sequence
          ::template set<
              s_last_component_index<TagValue>,
              typename component_info_sequence
                  ::template get<s_last_component_index<TagValue>>
                  ::template set<2, bool_constant<TagValue>>>;


      using component_sequence = typename component_info_sequence::template map<to_component_type>;
      using page_size_sequence = typename component_info_sequence::template map<to_page_size_type>;
      using tag_value_sequence = typename component_info_sequence::template map<to_tag_value_type>;
      using container_sequence = typename component_info_sequence::template map<to_container_type>;

      template<typename Component>
      static constexpr
      size_type
      component_index
      = component_sequence::template index<Component>;

      template<typename Component> struct page_size_of : page_size_sequence::template get<component_index<Component>> { };
      template<typename Component> struct tag_value_of : tag_value_sequence::template get<component_index<Component>> { };

      template<typename Component>
      struct container_of
      {
        using type
        = typename container_sequence::template get<component_index<Component>>;
      };
    };

    struct group
    {
    private:
      template<typename GroupInfo>
      struct to_group
      {
        using type
        = size_type;
      };

    public:
      template<typename ...Components>
      using use_group
      = typename group_info_sequence
          ::template append<type_sequence<Components ...>>;


      using group_sequence
      = typename group_info_sequence::template map<to_group>;
    };
  };

public:
  template<
      typename  Component,
      size_type PageSize  = default_container_page_size<>::value,
      bool      TagValue  = std::is_empty_v<Component>>
  using component
  = new_storage<
      identifier_type,
      allocator_type,
      typename utility::component::template use_component<Component, PageSize, TagValue>,
      group_info_sequence>;

  template<size_type PageSize>
  using paged
  = new_storage<
      identifier_type,
      allocator_type,
      typename utility::component::template use_paged<PageSize>,
      group_info_sequence>;

  template<bool TagValue>
  using tagged
  = new_storage<
      identifier_type,
      allocator_type,
      typename utility::component::template use_tagged<TagValue>,
      group_info_sequence>;

  template<typename ...Components>
  using group
  = new_storage<
      identifier_type,
      allocator_type,
      component_info_sequence,
      typename utility::group::template use_group<Components ...>>;

private:
  using container_tuple = typename utility::component::container_sequence::tuple;
  using group_tuple     = typename utility::group    ::group_sequence    ::tuple;

private:
  container_tuple m_containers;
  group_tuple     m_groups;

private:
  static constexpr bool s_noexcept_default_construct_false() noexcept;
  static constexpr bool s_noexcept_default_construct      () noexcept;
  static constexpr bool s_noexcept_move_alloc_construct   () noexcept;
  static constexpr bool s_noexcept_swap() noexcept;

  template<typename Component> static constexpr bool s_noexcept_erase    () noexcept;
  template<typename Component> static constexpr bool s_noexcept_try_erase() noexcept;

  template<std::size_t ...Is>
  static constexpr
  bool
  s_noexcept_erase(std::index_sequence<Is ...>)
  noexcept;


  constexpr explicit new_storage(bool_constant<true >) noexcept;
  constexpr explicit new_storage(bool_constant<false>) noexcept(s_noexcept_default_construct_false());


  template<typename Component> [[nodiscard]] constexpr auto       &m_container()       noexcept;
  template<typename Component> [[nodiscard]] constexpr auto const &m_container() const noexcept;

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

  [[nodiscard]]
  constexpr
  allocator_type
  get_allocator() const
  noexcept;

  constexpr
  void
  swap(new_storage &)
  noexcept(s_noexcept_swap());


  template<typename Component>
  [[nodiscard]]
  constexpr
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
  noexcept(s_noexcept_erase(std::make_index_sequence<utility::component::container_sequence::size>()));

  constexpr
  void
  clear()
  noexcept;


  friend constexpr
  void
  swap(new_storage &lhs, new_storage const &rhs)
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
          std::allocator_arg_t, allocator_type const &, container_tuple &&>;
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
  return std::is_nothrow_swappable_v<container_tuple>;
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
  return noexcept(std::declval<typename utility::component::template container_of<Component>::type &>().erase(std::declval<identifier_type>()));
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
  return noexcept(std::declval<typename utility::component::template container_of<Component>::type &>().try_erase(std::declval<identifier_type>()));
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<std::size_t ...Is>
constexpr
bool
new_storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::s_noexcept_erase(std::index_sequence<Is...>)
noexcept
{
  return
     (noexcept(std::get<Is>(std::declval<container_tuple &>()).erase(std::declval<identifier_type>()))
   && ...);
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
  return std::get<utility::component::template component_index<Component>>(m_containers);
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
  return std::get<utility::component::template component_index<Component>>(m_containers);
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
    m_groups    (0)
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
{ }

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
{ }

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

  // TODO: add group logic
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

  // TODO: add group logic
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

  // TODO: add group logic
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

  // TODO: add group logic
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

  // TODO: add group logic
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

  // TODO: add group logic
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
  m_container<Component>().erase(id);

  // TODO: add group logic
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
  bool const
  ret
  = m_container<Component>().try_erase(id);

  // TODO: add group logic
  return ret;
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
noexcept(s_noexcept_erase(std::make_index_sequence<utility::component::container_sequence::size>()))
{
  // TODO: implement with group logic in mind
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