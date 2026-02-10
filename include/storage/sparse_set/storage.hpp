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
#include "query_expression.hpp"
#include "type_sequence.hpp"

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
    struct to_page_size
    {
      using type
      = typename ComponentInfo::template get<1>;
    };

    template<typename ComponentInfo>
    struct to_tag_value
    {
      using type
      = typename ComponentInfo::template get<2>;
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

  public:
    using component_sequence = typename component_info_sequence::template map<to_component>;
    using page_size_sequence = typename component_info_sequence::template map<to_page_size>;
    using tag_value_sequence = typename component_info_sequence::template map<to_tag_value>;

    static_assert(
        std::is_same_v<
            component_sequence,
            typename component_sequence::template map<std::remove_cvref>>,
        "component_info_sequence must not contain any reference types.");

    using pool_sequence = typename component_info_sequence::template map<to_pool>;


    template<typename Component>
    using component
    = typename component_info_sequence
        ::template append<
            type_sequence<
                std::remove_cvref_t<Component>,
                size_constant<default_pool_page_size<>::value>,
                std::is_empty<std::remove_cvref_t<Component>>>>;

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
    requires(component_sequence::template contains<std::remove_cvref_t<Component>>)
    static constexpr
    size_type
    index
    = component_sequence::template index<std::remove_cvref_t<Component>>;

    static_assert(
        component_sequence::is_unique,
        "Component types can only be declared once in the storage.");


    template<typename Component>
    struct page_size_of
      : size_constant<page_size_sequence::template get<index<Component>>::value>
    { };

    template<typename Component>
    struct tag_value_of
      : bool_constant<tag_value_sequence::template get<index<Component>>::value>
    { };

    template<typename Component>
    struct pool_for
    {
      using type
      = typename pool_sequence::template get<index<Component>>;
    };


    using pool_tuple = typename pool_sequence::tuple;
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


  template<typename Expression>
  class generic_query
  {
  public:
    using expression_type = Expression;

    static_assert(
        specializes_query_expression_v<expression_type>,
        "expression_type must be a specialization of query_expression.");

  private:
    template<typename Component>
    struct not_tag
      : bool_constant<!component_info_sequence_traits::template tag_value_of<Component>::value>
    { };

    using include_sequence = typename expression_type::include_sequence;
    using exclude_sequence = typename expression_type::exclude_sequence;

    static_assert(
        include_sequence
            ::template map<std::remove_cvref>
            ::template difference<typename component_info_sequence_traits::component_sequence>
            ::size
         == 0,
        "All included types included in expression_type must be managed by the storage.");
    static_assert(
        exclude_sequence
            ::template map<std::remove_cvref>
            ::template difference<typename component_info_sequence_traits::component_sequence>
            ::size
         == 0,
        "All excluded types included in expression_type must be managed by the storage.");
    static_assert(
        include_sequence::size > 0,
        "expression_type must at least include one component_type.");

    using value_include_sequence = typename include_sequence::template filter<not_tag>;
    using value_exclude_sequence = typename exclude_sequence::template filter<not_tag>;

  public:
    using value_type
    = typename type_sequence<entity_type>
        ::template concatenate<
            typename value_include_sequence
                ::template map<std::remove_cvref>>
        ::tuple;

    using reference
    = typename type_sequence<entity_type const &>
        ::template concatenate<
            typename value_include_sequence
                ::template map<std::add_lvalue_reference>>
        ::tuple;

    using const_reference
    = typename type_sequence<entity_type const &>
        ::template concatenate<
            typename value_include_sequence
                ::template map<std::add_lvalue_reference>
                ::template map<std::add_const>>
        ::tuple;

  private:
    template<bool IsConst>
    class generic_iterator
    {
    public:
      using difference_type = std::ptrdiff_t;


      static constexpr bool is_const = IsConst;

      using iterator_category = std::input_iterator_tag;
      using iterator_concept  = std::bidirectional_iterator_tag;

      using value_type
      = typename generic_query::value_type;

      using reference
      = std::conditional_t<
          is_const,
          typename generic_query::const_reference,
          typename generic_query::reference>;

      struct pointer
      {
      private:
        reference m_ref;

      public:
        explicit constexpr
        pointer(reference &&)
        noexcept;


        constexpr
        reference *
        operator->() const
        noexcept;
      };


      struct pivot_info
      {
        entity_type const *entities;
        size_type          index;
        size_type          size;
      };


      friend generic_query;
      friend generic_iterator<!is_const>;

    private:
      maybe_const_t<storage, is_const> *m_storage;
      pivot_info                        m_pivot;
      difference_type                   m_index;

    private:
      // iterating over all entities that match the query with compile-time pools is difficult, we
      // basically to use "compile-time recursivity" to loop on the pools
      template<std::size_t = 0>
      [[nodiscard]] constexpr
      bool
      is_included(entity_type const) const
      noexcept;

      template<std::size_t = 0>
      [[nodiscard]] constexpr
      bool
      is_excluded(entity_type const) const
      noexcept;

      [[nodiscard]] constexpr
      bool
      is_verified(entity_type const) const
      noexcept;

      constexpr
      void
      increment()
      noexcept;

      constexpr
      void
      decrement()
      noexcept;


      // to find the best pivot for our query, because the pools' size is runtime information, we
      // again need to loop over all pools using "compile-time recursivity" to solve the issue.
      template<std::size_t = 0>
      [[nodiscard]] constexpr
      pivot_info
      pivot_at() const
      noexcept;

      template<std::size_t = 1>
      constexpr
      void
      find_best_pivot(pivot_info &) const
      noexcept;

      [[nodiscard]] constexpr
      pivot_info
      make_pivot() const
      noexcept;


      // to construct the reference of the iterator, again "compile-time recursivity".
      template<typename Component>
      [[nodiscard]] constexpr
      decltype(auto)
      value_at(entity_type const) const
      noexcept;

      template<std::size_t = 0>
      [[nodiscard]] constexpr
      auto
      make_value_tuple(entity_type const) const
      noexcept;


      constexpr
      generic_iterator(maybe_const_t<storage, is_const> * const, difference_type const)
      noexcept;

      explicit constexpr
      generic_iterator(maybe_const_t<storage, is_const> * const)
      noexcept;

    public:
      constexpr
      generic_iterator()
      = default;

      constexpr
      generic_iterator(generic_iterator const &)
      = default;

      constexpr
      generic_iterator(generic_iterator &&)
      = default;

      explicit constexpr
      generic_iterator(generic_iterator<!is_const>)
      noexcept;

      constexpr
      ~generic_iterator()
      = default;

      constexpr
      generic_iterator &
      operator=(generic_iterator const &)
      = default;

      constexpr
      generic_iterator &
      operator=(generic_iterator &&)
      = default;


      constexpr
      generic_iterator &
      operator++()
      noexcept;

      constexpr
      generic_iterator
      operator++(int)
      noexcept;

      constexpr
      generic_iterator &
      operator--()
      noexcept;

      constexpr
      generic_iterator
      operator--(int)
      noexcept;


      constexpr
      reference
      operator*() const
      noexcept;

      constexpr
      pointer
      operator->() const
      noexcept;

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
    using const_iterator = generic_iterator<true >;

    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  private:
    storage *m_storage;

  public:
    explicit constexpr
    generic_query(storage * const)
    noexcept;

    explicit constexpr
    generic_query(storage &)
    noexcept;

    constexpr
    generic_query()
    noexcept;

    constexpr
    generic_query(generic_query const &)
    = default;

    constexpr
    generic_query(generic_query &&)
    = default;

    constexpr
    ~generic_query()
    = default;

    constexpr
    generic_query &
    operator=(generic_query const &)
    = default;

    constexpr
    generic_query &
    operator=(generic_query &&)
    = default;


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
  };


  template<typename>
  friend class generic_query;

private:
  pool_tuple m_pools;

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

  template<typename Component>
  static constexpr
  bool
  s_noexcept_erase()
  noexcept;

  template<std::size_t ...Is>
  static constexpr
  bool
  s_noexcept_clear(std::index_sequence<Is ...>)
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


  template<typename Component>
  [[nodiscard]] constexpr
  bool
  has(entity_type const) const
  noexcept;

  template<typename Component>
  [[nodiscard]] constexpr
  Component &
  get(entity_type const)
  noexcept;

  template<typename Component>
  [[nodiscard]] constexpr
  Component const &
  get(entity_type const) const
  noexcept;


  template<typename Expression>
  [[nodiscard]] constexpr
  auto
  query()
  noexcept;

  template<typename Expression>
  [[nodiscard]] constexpr
  auto
  query() const
  noexcept;


  template<
      typename    Component,
      typename ...Args>
  constexpr
  bool
  emplace(entity_type const, Args &&...);

  template<
      typename    Component,
      typename ...Args>
  constexpr
  bool
  try_emplace(entity_type const, Args &&...);

  template<typename Component>
  constexpr
  bool
  insert_or_assign(entity_type const, Component const &);

  template<typename Component>
  constexpr
  bool
  insert_or_assign(entity_type const, Component &&);

  template<typename Component>
  constexpr
  void
  erase(entity_type const)
  noexcept(s_noexcept_erase<Component>());

  constexpr
  void
  clear()
  noexcept(s_noexcept_clear(std::make_index_sequence<std::tuple_size_v<pool_tuple>>()));


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
template<typename Expression>
template<bool IsConst>
constexpr
storage<Entity, Allocator, ComponentInfoSeq>
    ::generic_query<Expression>
    ::generic_iterator<IsConst>
    ::pointer
    ::pointer(reference &&ref)
noexcept
  : m_ref(std::move(ref))
{ }



template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
template<typename Expression>
template<bool IsConst>
constexpr
typename storage<Entity, Allocator, ComponentInfoSeq>
    ::template generic_query<Expression>
    ::template generic_iterator<IsConst>
    ::reference *
storage<Entity, Allocator, ComponentInfoSeq>
    ::generic_query<Expression>
    ::generic_iterator<IsConst>
    ::pointer
    ::operator->() const
noexcept
{
  return std::addressof(m_ref);
}



template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
template<typename Expression>
template<bool IsConst>
template<std::size_t I>
constexpr
bool
storage<Entity, Allocator, ComponentInfoSeq>
    ::generic_query<Expression>
    ::generic_iterator<IsConst>
    ::is_included(entity_type const e) const
noexcept
{
  if constexpr (I == include_sequence::size)
    return true;
  else
  {
    static constexpr
    size_type
    index
    = s_component_index<typename include_sequence::template get<I>>;

    if (index == m_pivot.index)
      return is_included<I + 1>(e);

    return std::get<index>((*m_storage).m_pools).contains(e)
        && is_included<I + 1>(e);
  }

}


template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
template<typename Expression>
template<bool IsConst>
template<std::size_t I>
constexpr
bool
storage<Entity, Allocator, ComponentInfoSeq>
    ::generic_query<Expression>
    ::generic_iterator<IsConst>
    ::is_excluded(entity_type const e) const
noexcept
{
  if constexpr (I == exclude_sequence::size)
    return true;
  else
  {
    static constexpr
    size_type
    index
    = s_component_index<typename exclude_sequence::template get<I>>;

    return !std::get<index>((*m_storage).m_pools).contains(e)
        && is_excluded<I + 1>(e);
  }
}

template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
template<typename Expression>
template<bool IsConst>
constexpr
bool
storage<Entity, Allocator, ComponentInfoSeq>
    ::generic_query<Expression>
    ::generic_iterator<IsConst>
    ::is_verified(entity_type const e) const
noexcept
{
  return is_included<>(e) && is_excluded<>(e);
}


template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
template<typename Expression>
template<bool IsConst>
constexpr
void
storage<Entity, Allocator, ComponentInfoSeq>
    ::generic_query<Expression>
    ::generic_iterator<IsConst>
    ::increment()
noexcept
{
  while (static_cast<size_type>(++m_index) < m_pivot.size && !is_verified(m_pivot.entities[m_index]));
}


template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
template<typename Expression>
template<bool IsConst>
constexpr
void
storage<Entity, Allocator, ComponentInfoSeq>
    ::generic_query<Expression>
    ::generic_iterator<IsConst>
    ::decrement()
noexcept
{
  while (--m_index >= 0 && !is_verified(m_pivot.entities[m_index]));
}



template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
template<typename Expression>
template<bool IsConst>
template<std::size_t I>
constexpr
typename storage<Entity, Allocator, ComponentInfoSeq>
    ::template generic_query<Expression>
    ::template generic_iterator<IsConst>
    ::pivot_info
storage<Entity, Allocator, ComponentInfoSeq>
    ::generic_query<Expression>
    ::generic_iterator<IsConst>
    ::pivot_at() const
noexcept
{
  static constexpr
  size_type
  index
  = s_component_index<typename include_sequence::template get<I>>;

  auto const &
  pool
  = std::get<index>((*m_storage).m_pools);

  size_type const
  size
  = pool.size();

  return pivot_info(
      size == 0
        ? nullptr
        : std::addressof(std::get<0>(*(pool.begin()))),
      index,
      size);
}


template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
template<typename Expression>
template<bool IsConst>
template<std::size_t I>
constexpr
void
storage<Entity, Allocator, ComponentInfoSeq>
    ::generic_query<Expression>
    ::generic_iterator<IsConst>
    ::find_best_pivot(pivot_info &best) const
noexcept
{
  if constexpr (I != include_sequence::size)
  {
    pivot_info pivot = pivot_at<I>();
    if (best.size > pivot.size)
      best = pivot;

    find_best_pivot<I + 1>(best);
  }
}


template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
template<typename Expression>
template<bool IsConst>
constexpr
typename storage<Entity, Allocator, ComponentInfoSeq>
    ::template generic_query<Expression>
    ::template generic_iterator<IsConst>
    ::pivot_info
storage<Entity, Allocator, ComponentInfoSeq>
    ::generic_query<Expression>
    ::generic_iterator<IsConst>
    ::make_pivot() const
noexcept
{
  pivot_info pivot = pivot_at<0>();
  find_best_pivot(pivot);
  return pivot;
}



template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
template<typename Expression>
template<bool IsConst>
template<typename Component>
constexpr
decltype(auto)
storage<Entity, Allocator, ComponentInfoSeq>
    ::generic_query<Expression>
    ::generic_iterator<IsConst>
    ::value_at(entity_type const e) const
noexcept
{
  static constexpr
  size_type
  index
  = s_component_index<Component>;

  auto &
  pool
  = std::get<index>((*m_storage).m_pools);

  if constexpr (std::is_const_v<Component> || is_const)
    return std::as_const(pool)[e];
  else
    return pool[e];
}

template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
template<typename Expression>
template<bool IsConst>
template<std::size_t I>
constexpr
auto
storage<Entity, Allocator, ComponentInfoSeq>
    ::generic_query<Expression>
    ::generic_iterator<IsConst>
    ::make_value_tuple(entity_type const e) const
noexcept
{
  if constexpr (I == value_include_sequence::size)
    return std::tuple();
  else
  {
    using component_type = value_include_sequence::template get<I>;

    return std::tuple_cat(
        std::tuple<decltype(value_at<component_type>(e))>(value_at<component_type>(e)),
        make_value_tuple<I + 1>(e));
  }
}



template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
template<typename Expression>
template<bool IsConst>
constexpr
storage<Entity, Allocator, ComponentInfoSeq>
    ::generic_query<Expression>
    ::generic_iterator<IsConst>
    ::generic_iterator(maybe_const_t<storage, is_const>* const storage, difference_type const index)
noexcept
  : m_storage(storage     ),
    m_pivot  (make_pivot()),
    m_index  (index       )
{ }

template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
template<typename Expression>
template<bool IsConst>
constexpr
storage<Entity, Allocator, ComponentInfoSeq>
    ::generic_query<Expression>
    ::generic_iterator<IsConst>
    ::generic_iterator(maybe_const_t<storage, is_const> * const storage)
noexcept
  : m_storage(storage     ),
    m_pivot  (make_pivot()),
    m_index  (m_pivot.size)
{ }



template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
template<typename Expression>
template<bool IsConst>
constexpr
storage<Entity, Allocator, ComponentInfoSeq>
    ::generic_query<Expression>
    ::generic_iterator<IsConst>
    ::generic_iterator(generic_iterator<!is_const> it)
noexcept
  : m_storage(it.m_storage),
    m_pivot  (it.m_pivot  ),
    m_index  (it.m_index  )
{
  static_assert(is_const, "is_const must be true.");
}



template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
template<typename Expression>
template<bool IsConst>
constexpr
typename storage<Entity, Allocator, ComponentInfoSeq>
    ::template generic_query<Expression>
    ::template generic_iterator<IsConst> &
storage<Entity, Allocator, ComponentInfoSeq>
    ::generic_query<Expression>
    ::generic_iterator<IsConst>
    ::operator++()
noexcept
{
  increment();
  return *this;
}

template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
template<typename Expression>
template<bool IsConst>
constexpr
typename storage<Entity, Allocator, ComponentInfoSeq>
    ::template generic_query<Expression>
    ::template generic_iterator<IsConst>
storage<Entity, Allocator, ComponentInfoSeq>
    ::generic_query<Expression>
    ::generic_iterator<IsConst>
    ::operator++(int)
noexcept
{
  generic_iterator tmp(*this);
  ++*this;
  return tmp;
}


template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
template<typename Expression>
template<bool IsConst>
constexpr
typename storage<Entity, Allocator, ComponentInfoSeq>
    ::template generic_query<Expression>
    ::template generic_iterator<IsConst> &
storage<Entity, Allocator, ComponentInfoSeq>
    ::generic_query<Expression>
    ::generic_iterator<IsConst>
    ::operator--()
noexcept
{
  decrement();
  return *this;
}

template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
template<typename Expression>
template<bool IsConst>
constexpr
typename storage<Entity, Allocator, ComponentInfoSeq>
    ::template generic_query<Expression>
    ::template generic_iterator<IsConst>
storage<Entity, Allocator, ComponentInfoSeq>
    ::generic_query<Expression>
    ::generic_iterator<IsConst>
    ::operator--(int)
noexcept
{
  generic_iterator tmp(*this);
  --*this;
  return tmp;
}



template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
template<typename Expression>
template<bool IsConst>
constexpr
typename storage<Entity, Allocator, ComponentInfoSeq>
    ::template generic_query<Expression>
    ::template generic_iterator<IsConst>
    ::reference
storage<Entity, Allocator, ComponentInfoSeq>
    ::generic_query<Expression>
    ::generic_iterator<IsConst>
    ::operator*() const
noexcept
{
  entity_type const e = m_pivot.entities[m_index];

  return std::tuple_cat(
      std::tuple<entity_type const &>(e),
      make_value_tuple<>(e));
}


template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
template<typename Expression>
template<bool IsConst>
constexpr
typename storage<Entity, Allocator, ComponentInfoSeq>
    ::template generic_query<Expression>
    ::template generic_iterator<IsConst>
    ::pointer
storage<Entity, Allocator, ComponentInfoSeq>
    ::generic_query<Expression>
    ::generic_iterator<IsConst>
    ::operator->() const
noexcept
{
  return pointer(**this);
}



template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
template<typename Expression>
constexpr
storage<Entity, Allocator, ComponentInfoSeq>
    ::generic_query<Expression>
    ::generic_query(storage * const storage)
noexcept
  : m_storage(storage)
{ }

template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
template<typename Expression>
constexpr
storage<Entity, Allocator, ComponentInfoSeq>
    ::generic_query<Expression>
    ::generic_query(storage &storage)
noexcept
  : generic_query(&storage)
{ }

template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
template<typename Expression>
constexpr
storage<Entity, Allocator, ComponentInfoSeq>
    ::generic_query<Expression>
    ::generic_query()
noexcept
  : generic_query(nullptr)
{ }



template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
template<typename Expression>
constexpr
typename storage<Entity, Allocator, ComponentInfoSeq>
    ::template generic_query<Expression>
    ::iterator
storage<Entity, Allocator, ComponentInfoSeq>
    ::generic_query<Expression>
    ::begin()
noexcept
{
  return ++iterator(m_storage, -1);
}

template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
template<typename Expression>
constexpr
typename storage<Entity, Allocator, ComponentInfoSeq>
    ::template generic_query<Expression>
    ::const_iterator
storage<Entity, Allocator, ComponentInfoSeq>
    ::generic_query<Expression>
    ::begin() const
noexcept
{
  return ++const_iterator(m_storage, -1);
}


template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
template<typename Expression>
constexpr
typename storage<Entity, Allocator, ComponentInfoSeq>
    ::template generic_query<Expression>
    ::iterator
storage<Entity, Allocator, ComponentInfoSeq>
    ::generic_query<Expression>
    ::end()
noexcept
{
  return iterator(m_storage);
}

template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
template<typename Expression>
constexpr
typename storage<Entity, Allocator, ComponentInfoSeq>
    ::template generic_query<Expression>
    ::const_iterator
storage<Entity, Allocator, ComponentInfoSeq>
    ::generic_query<Expression>
    ::end() const
noexcept
{
  return const_iterator(m_storage);
}


template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
template<typename Expression>
constexpr
typename storage<Entity, Allocator, ComponentInfoSeq>
    ::template generic_query<Expression>
    ::const_iterator
storage<Entity, Allocator, ComponentInfoSeq>
    ::generic_query<Expression>
    ::cbegin() const
noexcept
{
  return begin();
}


template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
template<typename Expression>
constexpr
typename storage<Entity, Allocator, ComponentInfoSeq>
    ::template generic_query<Expression>
    ::const_iterator
storage<Entity, Allocator, ComponentInfoSeq>
    ::generic_query<Expression>
    ::cend() const
noexcept
{
  return end();
}


template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
template<typename Expression>
constexpr
typename storage<Entity, Allocator, ComponentInfoSeq>
    ::template generic_query<Expression>
    ::reverse_iterator
storage<Entity, Allocator, ComponentInfoSeq>
    ::generic_query<Expression>
    ::rbegin()
noexcept
{
  return reverse_iterator(end());
}

template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
template<typename Expression>
constexpr
typename storage<Entity, Allocator, ComponentInfoSeq>
    ::template generic_query<Expression>
    ::const_reverse_iterator
storage<Entity, Allocator, ComponentInfoSeq>
    ::generic_query<Expression>
    ::rbegin() const
noexcept
{
  return const_reverse_iterator(end());
}


template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
template<typename Expression>
constexpr
typename storage<Entity, Allocator, ComponentInfoSeq>
    ::template generic_query<Expression>
    ::reverse_iterator
storage<Entity, Allocator, ComponentInfoSeq>
    ::generic_query<Expression>
    ::rend()
noexcept
{
  return reverse_iterator(begin());
}

template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
template<typename Expression>
constexpr
typename storage<Entity, Allocator, ComponentInfoSeq>
    ::template generic_query<Expression>
    ::const_reverse_iterator
storage<Entity, Allocator, ComponentInfoSeq>
    ::generic_query<Expression>
    ::rend() const
noexcept
{
  return const_reverse_iterator(begin());
}


template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
template<typename Expression>
constexpr
typename storage<Entity, Allocator, ComponentInfoSeq>
    ::template generic_query<Expression>
    ::const_reverse_iterator
storage<Entity, Allocator, ComponentInfoSeq>
    ::generic_query<Expression>
    ::crbegin() const
noexcept
{
  return rbegin();
}


template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
template<typename Expression>
constexpr
typename storage<Entity, Allocator, ComponentInfoSeq>
    ::template generic_query<Expression>
    ::const_reverse_iterator
storage<Entity, Allocator, ComponentInfoSeq>
    ::generic_query<Expression>
    ::crend() const
noexcept
{
  return rend();
}



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
template<typename Component>
constexpr
bool
storage<Entity, Allocator, ComponentInfoSeq>
    ::s_noexcept_erase()
noexcept
{
  return noexcept(std::get<s_component_index<Component>>(std::declval<pool_tuple &>())
      .erase(std::declval<entity_type const>()));
}

template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
template<std::size_t ...Is>
constexpr
bool
storage<Entity, Allocator, ComponentInfoSeq>
    ::s_noexcept_clear(std::index_sequence<Is ...>)
noexcept
{
  return (noexcept(std::get<Is>(std::declval<pool_tuple &>()).clear()) && ...);
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
template<typename Component>
constexpr
bool
storage<Entity, Allocator, ComponentInfoSeq>
    ::has(entity_type const e) const
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
Component &
storage<Entity, Allocator, ComponentInfoSeq>
    ::get(entity_type const e)
noexcept
{
  return m_pool<Component>()[e];
}

template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
template<typename Component>
constexpr
Component const &
storage<Entity, Allocator, ComponentInfoSeq>
    ::get(entity_type const e) const
noexcept
{
  return m_pool<Component>()[e];
}



template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
template<typename Expression>
constexpr
auto
storage<Entity, Allocator, ComponentInfoSeq>
    ::query()
noexcept
{
  return generic_query<Expression>(this);
}

template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
template<typename Expression>
constexpr
auto
storage<Entity, Allocator, ComponentInfoSeq>
    ::query() const
noexcept
{
  return generic_query<Expression>(this);
}



template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
template<
    typename    Component,
    typename ...Args>
constexpr
bool
storage<Entity, Allocator, ComponentInfoSeq>
    ::emplace(entity_type const e, Args &&...args)
{
  return m_pool<Component>().emplace(e, std::forward<Args>(args)...).second;
}


template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
template<
    typename    Component,
    typename ...Args>
constexpr
bool
storage<Entity, Allocator, ComponentInfoSeq>
    ::try_emplace(entity_type const e, Args &&...args)
{
  return m_pool<Component>().try_emplace(e, std::forward<Args>(args)...).second;
}


template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
template<typename Component>
constexpr
bool
storage<Entity, Allocator, ComponentInfoSeq>
    ::insert_or_assign(entity_type const e, Component const &c)
{
  return m_pool<Component>().insert_or_assign(e, c).second;
}

template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
template<typename Component>
constexpr
bool
storage<Entity, Allocator, ComponentInfoSeq>
    ::insert_or_assign(entity_type const e, Component &&c)
{
  return m_pool<Component>().insert_or_assign(e, std::move(c)).second;
}


template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
template<typename Component>
constexpr
void
storage<Entity, Allocator, ComponentInfoSeq>
    ::erase(entity_type const e)
noexcept(s_noexcept_erase<Component>())
{
  m_pool<Component>().erase(e);
}


template<
    typename Entity,
    typename Allocator,
    typename ComponentInfoSeq>
constexpr
void
storage<Entity, Allocator, ComponentInfoSeq>
    ::clear()
noexcept(s_noexcept_clear(std::make_index_sequence<std::tuple_size_v<pool_tuple>>()))
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