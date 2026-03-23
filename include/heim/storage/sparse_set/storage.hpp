#ifndef HEIM_SPARSE_SET_BASED_STORAGE_HPP
#define HEIM_SPARSE_SET_BASED_STORAGE_HPP

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>
#include "heim/allocator.hpp"
#include "heim/identifier.hpp"
#include "heim/query_expression.hpp"
#include "heim/type_sequence.hpp"
#include "heim/utility.hpp"
#include "pool.hpp"
#include "sparse_set.hpp"
#include "heim/component.hpp"

namespace heim::sparse_set_based
{
template<
    typename    Component,
    std::size_t PageSize>
using component_info
= type_sequence<Component, size_constant<PageSize>>;

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
 * @brief The main container of components, specialized for usage in the entity-component-system pattern.
 *
 * @details Manages single-component optimized pools to hold components of each type in their separate
 *   container. This allows for fast addition and removal of components on entities, and optimal iteration
 *   speed on small sets of component types.
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
class storage;

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
class storage
{
public:
  using identifier_type         = Identifier;
  using allocator_type          = Allocator;
  using component_info_sequence = ComponentInfoSeq;
  using group_info_sequence     = GroupInfoSeq;

  static_assert(
      specializes_identifier_v<identifier_type>,
      "heim::sparse_set_based::storage: identifier_type must be a specialization of identifier.");
  static_assert(
      is_an_allocator_for_v<allocator_type, identifier_type>,
      "heim::sparse_set_based::storage: allocator_type must pass as an allocator of identifier_type.");
  static_assert(
      specializes_type_sequence_v<component_info_sequence>,
      "heim::sparse_set_based::storage: component_info_sequence must be a specialization of type_sequence.");
  static_assert(
      specializes_type_sequence_v<group_info_sequence>,
      "heim::sparse_set_based::storage: group_info_sequence must be a specialization of type_sequence.");

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
        component_tag_value_v<typename ComponentInfo::template get<0>>,
        sparse_set<
            identifier_type,
            ComponentInfo::template get<1>::value,
            allocator_type>,
        pool<
            typename ComponentInfo::template get<0>,
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
      "heim::sparse_set_based::storage: component types must each neither be const nor contain references.");
  static_assert(
      component_sequence::is_unique,
      "heim::sparse_set_based::storage: component types must each be unique.");

  static_assert(
      group_info_sequence
          ::template filter  <specializes_type_sequence>
          ::template is_equal<group_info_sequence>,
      "heim::sparse_set_based::storage: group_info_sequence must only contain specializations of type_sequence.");
  static_assert(
      group_info_sequence::flatten::template difference<component_sequence>::size == 0,
      "heim::sparse_set_based::storage: each component type in group_info types must be unique and already "
          "managed by the storage.");
  static_assert(
      group_info_sequence
          ::template filter  <is_group_info>
          ::template is_equal<group_info_sequence>,
      "heim::sparse_set_based::storage: group_info types in group_info_sequence must contain at least "
          "2 component types.");

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


  template<
      bool     IsConstQuery,
      typename Expression>
  class generic_query
  {
  public:
    static constexpr
    bool
    is_const_query
    = IsConstQuery;

    using expression_type
    = Expression;

    static_assert(
        specializes_query_expression_v<expression_type>,
        "heim::sparse_set_based::storage::generic_query: expression_type must be a specialization of "
            "query_expression.");


    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;

  private:
    using storage_type
    = maybe_const_t<storage, is_const_query>;


    using include_sequence = typename expression_type::include_sequence;
    using exclude_sequence = typename expression_type::exclude_sequence;

    static_assert(
        include_sequence::size > 0,
        "heim::sparse_set_based::storage::generic_query; expression_type must include at least one component "
            "type.");
    static_assert(
       !is_const_query
     || include_sequence::template is_equal<typename include_sequence::template map<std::add_const>>,
        "heim::sparse_set_based::storage::generic_query: expression_type in a const generic_query must "
            "include only const component types.");
    static_assert(
       !is_const_query
     || exclude_sequence::template is_equal<typename exclude_sequence::template map<std::add_const>>,
        "heim::sparse_set_based::storage::generic_query: expression_type in a const generic_query must "
            "exclude only const component types.");


    using bare_include_sequence = typename include_sequence::template map<std::remove_const>;
    using bare_exclude_sequence = typename exclude_sequence::template map<std::remove_const>;

    static_assert(
        bare_include_sequence::template difference<component_sequence>::size == 0,
        "heim::sparse_set_based::storage::generic_query: expression_type must include component types "
            "that are managed by the storage.");
    static_assert(
        bare_exclude_sequence::template difference<component_sequence>::size == 0,
        "heim::sparse_set_based::storage::generic_query: expression_type must exclude component types "
            "that are managed by the storage.");

    static constexpr
    bool
    is_group_query
    = []() constexpr
        -> bool
    {
      using anchor
      = typename bare_include_sequence::template get<0>;

      if constexpr (!is_grouped<anchor>)
        return false;
      else
        return group_info_of<anchor>::template is_permutation<bare_include_sequence>;
    }();

  public:
    using value_type      = typename expression_type::template value_type     <identifier_type>;
    using reference       = typename expression_type::template reference      <identifier_type>;
    using const_reference = typename expression_type::template const_reference<identifier_type>;

  private:
    using reference_sequence       = typename to_type_sequence_t<reference      >::template erase<identifier_type const &>;
    using const_reference_sequence = typename to_type_sequence_t<const_reference>::template erase<identifier_type const &>;


    class regular_query_driver
    {
    private:
      using pivot_container_type
      = std::vector<identifier_type, allocator_type>;

    private:
      pivot_container_type const *
      m_pivot;

    private:
      template<typename ...Components>
      static constexpr
      pivot_container_type const *
      find_pivot(type_sequence<Components ...>, storage const *)
      noexcept;

      template<typename ...Components>
      static constexpr
      bool
      m_matches_all(type_sequence<Components ...>, storage const *, identifier_type)
      noexcept;

      template<typename ...Components>
      static constexpr
      bool
      m_matches_any(type_sequence<Components ...>, storage const *, identifier_type)
      noexcept;

      constexpr
      bool
      m_matches(storage const *, identifier_type) const
      noexcept;

      template<typename ...Components>
      static constexpr
      typename type_sequence<Components ...>::tuple
      s_dereference(type_sequence<Components ...>, storage *, identifier_type)
      noexcept;

      template<typename ...Components>
      static constexpr
      typename type_sequence<Components ...>::tuple
      s_dereference(type_sequence<Components ...>, storage const *, identifier_type)
      noexcept;

    public:
      constexpr explicit
      regular_query_driver(storage const *)
      noexcept;

      [[nodiscard]] constexpr
      difference_type
      size(storage const *) const
      noexcept;

      [[nodiscard]] constexpr difference_type increment(storage const *, difference_type) const noexcept;
      [[nodiscard]] constexpr difference_type decrement(storage const *, difference_type) const noexcept;

      [[nodiscard]] constexpr reference       dereference(storage *      , difference_type) const noexcept;
      [[nodiscard]] constexpr const_reference dereference(storage const *, difference_type) const noexcept;
    };


    class group_query_driver
    {
    private:
      using anchor
      = typename bare_include_sequence::template get<0>;

    private:
      template<typename ...Components>
      static constexpr
      bool
      m_matches_any(type_sequence<Components ...>, storage const *, difference_type)
      noexcept;

      template<typename ...Components>
      static constexpr
      typename type_sequence<Components ...>::tuple
      s_dereference(type_sequence<Components ...>, storage *, difference_type)
      noexcept;

      template<typename ...Components>
      static constexpr
      typename type_sequence<Components ...>::tuple
      s_dereference(type_sequence<Components ...>, storage const *, difference_type)
      noexcept;

    public:
      constexpr explicit
      group_query_driver(storage const *)
      noexcept;

      static constexpr
      difference_type
      size(storage const *)
      noexcept;

      static constexpr difference_type increment(storage const *, difference_type) noexcept;
      static constexpr difference_type decrement(storage const *, difference_type) noexcept;

      static constexpr reference       dereference(storage *      , difference_type) noexcept;
      static constexpr const_reference dereference(storage const *, difference_type) noexcept;
    };


    using query_driver
    = std::conditional_t<is_group_query, group_query_driver, regular_query_driver>;


    template<bool IsConstIt>
    class generic_iterator
    {
    public:
      friend generic_query;

    public:
      static constexpr
      bool
      is_const_iterator
      = IsConstIt;


      using difference_type
      = std::ptrdiff_t;

      using iterator_category = std::input_iterator_tag;
      using iterator_concept  = std::bidirectional_iterator_tag;

      using value_type
      = typename generic_query::value_type;

      using reference
      = std::conditional_t<
          is_const_iterator,
          typename generic_query::const_reference,
          typename generic_query::reference>;

      struct pointer
      {
      private:
        reference
        m_ref;

      public:
        constexpr explicit
        pointer(reference &&)
        noexcept;


        constexpr
        reference *
        operator->() const
        noexcept;
      };

    private:
      using storage_type
      = maybe_const_t<storage, is_const_iterator>;

    private:
      storage_type   *m_storage;
      difference_type m_index;

      [[no_unique_address]]
      query_driver
      m_driver;

    private:
      constexpr generic_iterator(storage_type *, difference_type, query_driver) noexcept;
      constexpr generic_iterator(storage_type *, query_driver)                  noexcept;

    public:
      constexpr
      generic_iterator()
      noexcept;

      constexpr generic_iterator(generic_iterator const &) = default;
      constexpr generic_iterator(generic_iterator &&)      = default;

      constexpr
      ~generic_iterator()
      = default;

      constexpr generic_iterator &operator=(generic_iterator const &) = default;
      constexpr generic_iterator &operator=(generic_iterator &&)      = default;


      constexpr generic_iterator &operator++()    noexcept;
      constexpr generic_iterator  operator++(int) noexcept;

      constexpr generic_iterator &operator--()    noexcept;
      constexpr generic_iterator  operator--(int) noexcept;


      constexpr reference operator* () const noexcept;
      constexpr pointer   operator->() const noexcept;


      [[nodiscard]] friend constexpr
      bool
      operator==(generic_iterator const &lhs, generic_iterator const &rhs)
      noexcept
      {
        return lhs.m_index == rhs.m_index;
      }

      [[nodiscard]] friend constexpr
      decltype(auto)
      operator<=>(generic_iterator const &lhs, generic_iterator const &rhs)
      noexcept
      {
        return lhs.m_index <=> rhs.m_index;
      }
    };

  public:
    using iterator       = generic_iterator<false>;
    using const_iterator = generic_iterator<true>;

    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  private:
    storage_type *
    m_storage;

  public:
    constexpr generic_query()                      = default;
    constexpr generic_query(generic_query const &) = default;
    constexpr generic_query(generic_query &&)      = default;

    constexpr explicit
    generic_query(storage_type *)
    noexcept;

    constexpr
    ~generic_query()
    = default;

    constexpr generic_query &operator=(generic_query const &) = default;
    constexpr generic_query &operator=(generic_query &&)      = default;


    [[nodiscard]] constexpr iterator       begin ()       noexcept;
    [[nodiscard]] constexpr const_iterator begin () const noexcept;
    [[nodiscard]] constexpr const_iterator cbegin() const noexcept;

    [[nodiscard]] constexpr iterator       end ()       noexcept;
    [[nodiscard]] constexpr const_iterator end () const noexcept;
    [[nodiscard]] constexpr const_iterator cend() const noexcept;

    [[nodiscard]] constexpr reverse_iterator       rbegin ()       noexcept;
    [[nodiscard]] constexpr const_reverse_iterator rbegin () const noexcept;
    [[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept;

    [[nodiscard]] constexpr reverse_iterator       rend ()       noexcept;
    [[nodiscard]] constexpr const_reverse_iterator rend () const noexcept;
    [[nodiscard]] constexpr const_reverse_iterator crend() const noexcept;
  };

public:
  template<typename Expression> using query_type       = generic_query<false, Expression>;
  template<typename Expression> using const_query_type = generic_query<true , Expression>;


  template<
      typename  Component,
      size_type PageSize  = default_container_page_size<>::value>
  using component
  = storage<
      identifier_type,
      allocator_type,
      typename component_info_sequence
          ::template append<component_info<Component, PageSize>>,
      group_info_sequence>;

  template<size_type PageSize>
  using paged
  = storage<
      identifier_type,
      allocator_type,
      typename component_info_sequence
          ::template set<
              s_last_component_index<PageSize>,
              typename component_info_sequence
                  ::template get<s_last_component_index<PageSize>>
                  ::template set<1, size_constant<PageSize>>>,
      group_info_sequence>;

  template<typename ...Components>
  using group
  = storage<
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
  s_noexcept_m_clear(type_sequence<Components ...>)
  noexcept;

  static constexpr bool s_noexcept_default_construct_false() noexcept;
  static constexpr bool s_noexcept_default_construct      () noexcept;
  static constexpr bool s_noexcept_move_alloc_construct   () noexcept;
  static constexpr bool s_noexcept_swap() noexcept;

  template<typename Component> static constexpr bool s_noexcept_erase    () noexcept;
  template<typename Component> static constexpr bool s_noexcept_try_erase() noexcept;

  static constexpr
  bool
  s_noexcept_clear()
  noexcept;


  constexpr explicit storage(bool_constant<true >) noexcept;
  constexpr explicit storage(bool_constant<false>) noexcept(s_noexcept_default_construct_false());


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
  m_clear(type_sequence<Components ...>, identifier_type)
  noexcept(s_noexcept_m_clear(type_sequence<Components ...>{}));

public:
  constexpr explicit
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

  constexpr storage &operator=(storage const &) = default;
  constexpr storage &operator=(storage &&)      = default;

  [[nodiscard]] constexpr
  allocator_type
  get_allocator() const
  noexcept;

  constexpr
  void
  swap(storage &)
  noexcept(s_noexcept_swap());


  template<typename Component>
  [[nodiscard]] constexpr
  bool
  has(identifier_type) const
  noexcept;

  template<typename Component> [[nodiscard]] constexpr Component       &get(identifier_type)       noexcept;
  template<typename Component> [[nodiscard]] constexpr Component const &get(identifier_type) const noexcept;

  template<typename Expression> [[nodiscard]] constexpr query_type<Expression>       query()       noexcept;
  template<typename Expression> [[nodiscard]] constexpr const_query_type<Expression> query() const noexcept;


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
  clear(identifier_type)
  noexcept(s_noexcept_clear());

  constexpr
  void
  clear()
  noexcept;


  friend constexpr
  void
  swap(storage &lhs, storage &rhs)
  noexcept(s_noexcept_swap())
  {
    lhs.swap(rhs);
  }

  [[nodiscard]]
  friend constexpr
  bool
  operator==(storage const &, storage const &)
  = default;
};


template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<
    bool     IsConstQuery,
    typename Expression>
template<typename ...Components>
constexpr
typename storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::template generic_query<IsConstQuery, Expression>
    ::regular_query_driver
    ::pivot_container_type const *
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::generic_query<IsConstQuery, Expression>
    ::regular_query_driver
    ::find_pivot(type_sequence<Components ...>, storage const * const s)
noexcept
{
  return std::min(
      {std::addressof(
          static_cast<sparse_set<identifier_type, container_of<Components>::page_size, allocator_type> const &>(
              s->template m_container<Components>()).container())
          ...},
      [](auto const * const lhs, auto const * const rhs)
      {
        return lhs->size() < rhs->size();
      });
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<
    bool     IsConstQuery,
    typename Expression>
template<typename ...Components>
constexpr
bool
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::generic_query<IsConstQuery, Expression>
    ::regular_query_driver
    ::m_matches_all(type_sequence<Components ...>, storage const * const s, identifier_type const id)
noexcept
{
  return (s->template m_container<Components>().contains(id) && ...);
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<
    bool     IsConstQuery,
    typename Expression>
template<typename ...Components>
constexpr
bool
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::generic_query<IsConstQuery, Expression>
    ::regular_query_driver
    ::m_matches_any(type_sequence<Components ...>, storage const * const s, identifier_type const id)
  noexcept
{
  return (s->template m_container<Components>().contains(id) || ...);
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<
    bool     IsConstQuery,
    typename Expression>
constexpr
bool
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::generic_query<IsConstQuery, Expression>
    ::regular_query_driver
    ::m_matches(storage const * const s, identifier_type const id) const
noexcept
{
  return  m_matches_all(bare_include_sequence{}, s, id)
      && !m_matches_any(bare_exclude_sequence{}, s, id);
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<
    bool     IsConstQuery,
    typename Expression>
template<typename ...Components>
constexpr
typename type_sequence<Components ...>::tuple
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::generic_query<IsConstQuery, Expression>
    ::regular_query_driver
    ::s_dereference(type_sequence<Components ...>, storage * const s, identifier_type const id)
noexcept
{
  return std::forward_as_tuple(
      s->template m_container<std::remove_cvref_t<Components>>()[id]
      ...);
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<
    bool     IsConstQuery,
    typename Expression>
template<typename ...Components>
constexpr
typename type_sequence<Components ...>::tuple
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::generic_query<IsConstQuery, Expression>
    ::regular_query_driver
    ::s_dereference(type_sequence<Components ...>, storage const * const s, identifier_type const id)
noexcept
{
  return std::forward_as_tuple(
      s->template m_container<std::remove_cvref_t<Components>>()[id]
      ...);
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<
    bool     IsConstQuery,
    typename Expression>
constexpr
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::generic_query<IsConstQuery, Expression>
    ::regular_query_driver
    ::regular_query_driver(storage const *s)
noexcept
  : m_pivot{find_pivot(bare_include_sequence{}, s)}
{ }

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<
    bool     IsConstQuery,
    typename Expression>
constexpr
typename storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::template generic_query<IsConstQuery, Expression>
    ::difference_type
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::generic_query<IsConstQuery, Expression>
    ::regular_query_driver
    ::size(storage const *) const
noexcept
{
  return m_pivot->size();
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<
    bool     IsConstQuery,
    typename Expression>
constexpr
typename storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::template generic_query<IsConstQuery, Expression>
    ::difference_type
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::generic_query<IsConstQuery, Expression>
    ::regular_query_driver
    ::increment(storage const * const s, difference_type idx) const
noexcept
{
  do
    ++idx;
  while (idx < static_cast<difference_type>(m_pivot->size())
      && !m_matches(s, m_pivot->rbegin()[idx]));

  return idx;
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<
    bool     IsConstQuery,
    typename Expression>
constexpr
typename storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::template generic_query<IsConstQuery, Expression>
    ::difference_type
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::generic_query<IsConstQuery, Expression>
    ::regular_query_driver
    ::decrement(storage const * const s, difference_type idx) const
noexcept
{
  do
    --idx;
  while (idx >= 0 && !m_matches(s, m_pivot->rbegin()[idx]));

  return idx;
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<
    bool     IsConstQuery,
    typename Expression>
constexpr
typename storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::template generic_query<IsConstQuery, Expression>
    ::reference
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::generic_query<IsConstQuery, Expression>
    ::regular_query_driver
    ::dereference(storage * const s, difference_type const idx) const
noexcept
{
  identifier_type const &
  id
  = m_pivot->rbegin()[idx];

  return std::tuple_cat(
      std::forward_as_tuple(id),
      s_dereference(reference_sequence{}, s, id));
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<
    bool     IsConstQuery,
    typename Expression>
constexpr
typename storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::template generic_query<IsConstQuery, Expression>
    ::const_reference
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::generic_query<IsConstQuery, Expression>
    ::regular_query_driver
    ::dereference(storage const * const s, difference_type const idx) const
noexcept
{
  identifier_type const &
  id
  = m_pivot->rbegin()[idx];

  return std::tuple_cat(
      std::forward_as_tuple(id),
      s_dereference(const_reference_sequence{}, s, id));
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<
    bool     IsConstQuery,
    typename Expression>
template<typename ...Components>
constexpr
bool
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::generic_query<IsConstQuery, Expression>
    ::group_query_driver
    ::m_matches_any(type_sequence<Components ...>, storage const * const s, difference_type const idx)
noexcept
{
  using sparse_set_type
  = sparse_set<identifier_type, container_of<anchor>::page_size, allocator_type>;

  auto const           &st = *s;
  identifier_type const id = static_cast<sparse_set_type const &>(st.template m_container<anchor>()).rbegin()[idx];

  return (st.template m_container<Components>().contains(id) || ...);
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<
    bool     IsConstQuery,
    typename Expression>
template<typename ...Components>
constexpr
typename type_sequence<Components ...>::tuple
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::generic_query<IsConstQuery, Expression>
    ::group_query_driver
    ::s_dereference(type_sequence<Components ...>, storage * const s, difference_type const idx)
noexcept
{
  return std::forward_as_tuple(
      s->template m_container<std::remove_cvref_t<Components>>().rbegin()[idx].second
      ...);
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<
    bool     IsConstQuery,
    typename Expression>
template<typename ...Components>
constexpr
typename type_sequence<Components ...>::tuple
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::generic_query<IsConstQuery, Expression>
    ::group_query_driver
    ::s_dereference(type_sequence<Components ...>, storage const * const s, difference_type const idx)
noexcept
{
  return std::forward_as_tuple(
      s->template m_container<std::remove_cvref_t<Components>>().rbegin()[idx].second
      ...);
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<
    bool     IsConstQuery,
    typename Expression>
constexpr
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::generic_query<IsConstQuery, Expression>
    ::group_query_driver
    ::group_query_driver(storage const *)
noexcept
{ }

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<
    bool     IsConstQuery,
    typename Expression>
constexpr
typename storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::template generic_query<IsConstQuery, Expression>
    ::difference_type
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::generic_query<IsConstQuery, Expression>
    ::group_query_driver
    ::size(storage const * const s)
noexcept
{
  return static_cast<difference_type>(s->template m_group<anchor>());
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<
    bool     IsConstQuery,
    typename Expression>
constexpr
typename storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::template generic_query<IsConstQuery, Expression>
    ::difference_type
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::generic_query<IsConstQuery, Expression>
    ::group_query_driver
    ::increment(storage const * const s, difference_type idx)
noexcept
{
  do
    ++idx;
  while (idx < static_cast<difference_type>(s->template m_group<anchor>())
      && m_matches_any(bare_exclude_sequence{}, s, idx));

  return idx;
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<
    bool     IsConstQuery,
    typename Expression>
constexpr
typename storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::template generic_query<IsConstQuery, Expression>
    ::difference_type
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::generic_query<IsConstQuery, Expression>
    ::group_query_driver
    ::decrement(storage const * const s, difference_type idx)
noexcept
{
  do
    --idx;
  while (idx >= 0
      && matches_any(bare_exclude_sequence{}, s, idx));

  return idx;
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<
    bool     IsConstQuery,
    typename Expression>
constexpr
typename storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::template generic_query<IsConstQuery, Expression>
    ::reference
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::generic_query<IsConstQuery, Expression>
    ::group_query_driver
    ::dereference(storage * const s, difference_type const idx)
noexcept
{
  using sparse_set_type
  = sparse_set<identifier_type, container_of<anchor>::page_size, allocator_type>;

  return std::tuple_cat(
      std::forward_as_tuple(static_cast<sparse_set_type const &>(s->template m_container<anchor>()).rbegin()[idx]),
      s_dereference(reference_sequence{}, s, idx));
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<
    bool     IsConstQuery,
    typename Expression>
constexpr
typename storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::template generic_query<IsConstQuery, Expression>
    ::const_reference
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::generic_query<IsConstQuery, Expression>
    ::group_query_driver
    ::dereference(storage const * const s, difference_type const idx)
noexcept
{
  using sparse_set_type
  = sparse_set<identifier_type, container_of<anchor>::page_size, allocator_type>;

  return std::tuple_cat(
      std::forward_as_tuple(static_cast<sparse_set_type const &>(s->template m_container<anchor>()).rbegin()[idx]),
      s_dereference(const_reference_sequence{}, s, idx));
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<
    bool     IsConstQuery,
    typename Expression>
template<bool IsConstIt>
constexpr
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::generic_query<IsConstQuery, Expression>
    ::generic_iterator<IsConstIt>
    ::pointer
    ::pointer(reference &&ref)
noexcept
  : m_ref(std::move(ref))
{ }

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<
    bool     IsConstQuery,
    typename Expression>
template<bool IsConstIt>
constexpr
typename storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::template generic_query<IsConstQuery, Expression>
    ::template generic_iterator<IsConstIt>
    ::reference *
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::generic_query<IsConstQuery, Expression>
    ::generic_iterator<IsConstIt>
    ::pointer
    ::operator->() const
noexcept
{
  return std::addressof(m_ref);
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<
    bool     IsConstQuery,
    typename Expression>
template<bool IsConstIt>
constexpr
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::generic_query<IsConstQuery, Expression>
    ::generic_iterator<IsConstIt>
    ::generic_iterator(storage_type * const s, difference_type const idx, query_driver const d)
noexcept
  : m_storage{s},
    m_index  {idx},
    m_driver {d}
{
  m_index = m_driver.increment(m_storage, --m_index);
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<
    bool     IsConstQuery,
    typename Expression>
template<bool IsConstIt>
constexpr
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::generic_query<IsConstQuery, Expression>
    ::generic_iterator<IsConstIt>
    ::generic_iterator(storage_type * const s, query_driver d)
noexcept
  : m_storage{s},
    m_index  {d.size(s)},
    m_driver (d)
{
  m_index = m_driver.increment(m_storage, --m_index);
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<
    bool     IsConstQuery,
    typename Expression>
template<bool IsConstIt>
constexpr
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::generic_query<IsConstQuery, Expression>
    ::generic_iterator<IsConstIt>
    ::generic_iterator()
noexcept
  : m_storage{nullptr},
    m_index  {0},
    m_driver {}
{ }

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<
    bool     IsConstQuery,
    typename Expression>
template<bool IsConstIt>
constexpr
typename storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::template generic_query<IsConstQuery, Expression>
    ::template generic_iterator<IsConstIt> &
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::generic_query<IsConstQuery, Expression>
    ::generic_iterator<IsConstIt>
    ::operator++()
noexcept
{
  m_index = m_driver.increment(m_storage, m_index);
  return *this;
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<
    bool     IsConstQuery,
    typename Expression>
template<bool IsConstIt>
constexpr
typename storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::template generic_query<IsConstQuery, Expression>
    ::template generic_iterator<IsConstIt>
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::generic_query<IsConstQuery, Expression>
    ::generic_iterator<IsConstIt>
    ::operator++(int)
noexcept
{
  generic_iterator tmp(*this);
  operator++();
  return tmp;
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<
    bool     IsConstQuery,
    typename Expression>
template<bool IsConstIt>
constexpr
typename storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::template generic_query<IsConstQuery, Expression>
    ::template generic_iterator<IsConstIt> &
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::generic_query<IsConstQuery, Expression>
    ::generic_iterator<IsConstIt>
    ::operator--()
noexcept
{
  m_index = m_driver.decrement(m_storage, m_index);
  return *this;
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<
    bool     IsConstQuery,
    typename Expression>
template<bool IsConstIt>
constexpr
typename storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::template generic_query<IsConstQuery, Expression>
    ::template generic_iterator<IsConstIt>
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::generic_query<IsConstQuery, Expression>
    ::generic_iterator<IsConstIt>
    ::operator--(int)
noexcept
{
  generic_iterator tmp(*this);
  --*this;
  return tmp;
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<
    bool     IsConstQuery,
    typename Expression>
template<bool IsConstIt>
constexpr
typename storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::template generic_query<IsConstQuery, Expression>
    ::template generic_iterator<IsConstIt>
    ::reference
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::generic_query<IsConstQuery, Expression>
    ::generic_iterator<IsConstIt>
    ::operator*() const
noexcept
{
  return m_driver.dereference(m_storage, m_index);
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<
    bool     IsConstQuery,
    typename Expression>
template<bool IsConstIt>
constexpr
typename storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::template generic_query<IsConstQuery, Expression>
    ::template generic_iterator<IsConstIt>
    ::pointer
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::generic_query<IsConstQuery, Expression>
    ::generic_iterator<IsConstIt>
    ::operator->() const
noexcept
{
  return pointer(operator*());
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<
    bool     IsConstQuery,
    typename Expression>
constexpr
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::generic_query<IsConstQuery, Expression>
    ::generic_query(storage_type *storage)
noexcept
  : m_storage{storage}
{ }

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<
    bool     IsConstQuery,
    typename Expression>
constexpr
typename storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::template generic_query<IsConstQuery, Expression>
    ::iterator
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::generic_query<IsConstQuery, Expression>
    ::begin()
noexcept
{
  return iterator(m_storage, 0, query_driver(m_storage));
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<
    bool     IsConstQuery,
    typename Expression>
constexpr
typename storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::template generic_query<IsConstQuery, Expression>
    ::const_iterator
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::generic_query<IsConstQuery, Expression>
    ::begin() const
noexcept
{
  return const_iterator(m_storage, 0, query_driver(m_storage));
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<
    bool     IsConstQuery,
    typename Expression>
constexpr
typename storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::template generic_query<IsConstQuery, Expression>
    ::const_iterator
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::generic_query<IsConstQuery, Expression>
    ::cbegin() const
noexcept
{
  return begin();
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<
    bool     IsConstQuery,
    typename Expression>
constexpr
typename storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::template generic_query<IsConstQuery, Expression>
    ::iterator
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::generic_query<IsConstQuery, Expression>
    ::end()
noexcept
{
  return iterator(m_storage, query_driver(m_storage));
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<
    bool     IsConstQuery,
    typename Expression>
constexpr
typename storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::template generic_query<IsConstQuery, Expression>
    ::const_iterator
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::generic_query<IsConstQuery, Expression>
    ::end() const
noexcept
{
  return const_iterator(m_storage, query_driver(m_storage));
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<
    bool     IsConstQuery,
    typename Expression>
constexpr
typename storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::template generic_query<IsConstQuery, Expression>
    ::const_iterator
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::generic_query<IsConstQuery, Expression>
    ::cend() const
noexcept
{
  return end();
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<
    bool     IsConstQuery,
    typename Expression>
constexpr
typename storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::template generic_query<IsConstQuery, Expression>
    ::reverse_iterator
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::generic_query<IsConstQuery, Expression>
    ::rbegin()
noexcept
{
  return std::make_reverse_iterator(end());
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<
    bool     IsConstQuery,
    typename Expression>
constexpr
typename storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::template generic_query<IsConstQuery, Expression>
    ::const_reverse_iterator
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::generic_query<IsConstQuery, Expression>
    ::rbegin() const
noexcept
{
  return std::make_reverse_iterator(end());
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<
    bool     IsConstQuery,
    typename Expression>
constexpr
typename storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::template generic_query<IsConstQuery, Expression>
    ::reverse_iterator
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::generic_query<IsConstQuery, Expression>
    ::rend()
noexcept
{
  return std::make_reverse_iterator(begin());
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<
    bool     IsConstQuery,
    typename Expression>
constexpr
typename storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::template generic_query<IsConstQuery, Expression>
    ::const_reverse_iterator
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::generic_query<IsConstQuery, Expression>
    ::rend() const
noexcept
{
  return std::make_reverse_iterator(begin());
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<
    bool     IsConstQuery,
    typename Expression>
constexpr
typename storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::template generic_query<IsConstQuery, Expression>
    ::const_reverse_iterator
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::generic_query<IsConstQuery, Expression>
    ::crbegin() const
noexcept
{
  return rbegin();
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<
    bool     IsConstQuery,
    typename Expression>
constexpr
typename storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::template generic_query<IsConstQuery, Expression>
    ::const_reverse_iterator
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::generic_query<IsConstQuery, Expression>
    ::crend() const
noexcept
{
  return rend();
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<typename Component>
constexpr
bool
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
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
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
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
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
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
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::s_noexcept_m_clear(type_sequence<Components ...>)
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
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
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
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
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
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
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
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
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
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
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
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
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
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::s_noexcept_clear()
noexcept
{
  return s_noexcept_m_clear(component_sequence{});
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
constexpr
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::storage(bool_constant<true>)
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
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::storage(bool_constant<false>)
noexcept(s_noexcept_default_construct_false())
  : storage(allocator_type{})
{ }

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<typename Component>
constexpr
auto &
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
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
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
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
typename storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::size_type &
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
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
typename storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::size_type
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
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
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
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
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::m_group_swap(identifier_type const id, difference_type const group)
noexcept(s_noexcept_m_group_swap<Component>())
{
  using sparse_set_type
  = sparse_set<identifier_type, container_of<Component>::page_size, allocator_type>;

  container_of<Component> &
  container
  = m_container<Component>();

  container.swap(id, static_cast<sparse_set_type &>(container).rbegin()[group]);
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<typename ...Components>
constexpr
void
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
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
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
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
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::m_clear(type_sequence<Components ...>, identifier_type const id)
noexcept(s_noexcept_m_clear(type_sequence<Components ...>{}))
{
  (try_erase<Components>(id), ...);
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
constexpr
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::storage(allocator_type const &alloc)
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
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::storage()
noexcept(s_noexcept_default_construct())
  : storage(bool_constant<component_info_sequence::size == 0>{})
{ }

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
constexpr
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::storage(storage const &other, allocator_type const &alloc)
  : m_containers(std::allocator_arg, alloc, other.m_containers),
    m_groups    (other.m_groups)
{ }

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
constexpr
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::storage(storage &&other, allocator_type const &alloc)
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
typename storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::allocator_type
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
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
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::swap(storage &other)
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
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::has(identifier_type const id) const
noexcept
{
  static_assert(
      component_sequence::template contains<Component>,
      "heim::sparse_set_based::storage::has: Component must be managed by the storage.");

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
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::get(identifier_type const id)
noexcept
{
  static_assert(
      component_sequence::template contains<Component>,
      "heim::sparse_set_based::storage::get: Component must be managed by the storage.");
  static_assert(
      !component_tag_value_v<Component>,
      "heim::sparse_set_based::storage::get: Component must have a tag value of false.");

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
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::get(identifier_type const id) const
noexcept
{
  static_assert(
      component_sequence::template contains<Component>,
      "heim::sparse_set_based::storage::get: Component must be managed by the storage.");
  static_assert(
      !component_tag_value_v<Component>,
      "heim::sparse_set_based::storage::get: Component must have a tag value of false.");

  return m_container<Component>()[id];
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<typename Expression>
constexpr
typename storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::template query_type<Expression>
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::query()
noexcept
{
  return query_type<Expression>(this);
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
template<typename Expression>
constexpr
typename storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::template const_query_type<Expression>
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::query() const
noexcept
{
  return const_query_type<Expression>(this);
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
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
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
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
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
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
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
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
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
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
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
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
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
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
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
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
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
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::clear(identifier_type const id)
noexcept(s_noexcept_clear())
{
  m_clear(component_sequence{}, id);
}

template<
    typename Identifier,
    typename Allocator,
    typename ComponentInfoSeq,
    typename GroupInfoSeq>
constexpr
void
storage<Identifier, Allocator, ComponentInfoSeq, GroupInfoSeq>
    ::clear()
noexcept
{
  std::apply([](auto &...containers) { (containers.clear(), ...); }, m_containers);
  std::apply([](auto &...groups    ) { ((groups = 0)      , ...); }, m_groups    );
}


}

#endif // HEIM_SPARSE_SET_BASED_STORAGE_HPP