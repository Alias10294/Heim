#ifndef HEIM_STATIC_REGISTRY_HPP
#define HEIM_STATIC_REGISTRY_HPP

#include <cstddef>
#include <iterator>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>
#include "heim/ecs/entity.hpp"
#include "heim/ecs/expression.hpp"
#include "heim/ecs/identifier.hpp"
#include "heim/lib/type_sequence.hpp"
#include "detail/core.hpp"
#include "detail/iterator.hpp"
#include "pool.hpp"
#include "set.hpp"

namespace heim::sparse
{
namespace detail
{
template<
    typename    Component,
    std::size_t PageSize  = default_page_size_v<>>
requires component<Component>
using generic_static_registry_descriptor
= type_sequence<Component, std::integral_constant<std::size_t, PageSize>>;


template<
    typename Identifier   = default_identifier_t<>,
    typename Allocator    = std::allocator<Identifier>,
    typename DescSequence = type_sequence<>>
class generic_static_registry_storage
{ };

template<
    typename       Identifier,
    typename       Allocator,
    typename    ...Components,
    std::size_t ...PageSizes>
requires (
    identifier   <Identifier>
 && allocator_for<Allocator, Identifier>
 && (component   <Components> && ...)
 && type_sequence<Components ...>::is_unique)
class generic_static_registry_storage<
    Identifier,
    Allocator,
    type_sequence<generic_static_registry_descriptor<Components, PageSizes> ...>>
{
public:
  using identifier_type      = Identifier;
  using allocator_type       = Allocator;
  using description_sequence = type_sequence<generic_static_registry_descriptor<Components, PageSizes> ...>;

private:
  using component_sequence = type_sequence<Components ...>;
  using container_sequence = type_sequence<pool<Components, Identifier, PageSizes, Allocator> ...>;
  using container_tuple    = typename container_sequence::tuple;

  template<typename Component>
  requires (
     !component_sequence::empty
   && component_sequence::template contains<Component>)
  static constexpr
  std::size_t
  component_index
  = component_sequence::template index<Component>;

public:
  template<typename Component>
  requires (
     !component_sequence::empty
   && component_sequence::template contains<Component>)
  using container_for
  = container_sequence::template get<component_index<Component>>;

private:
  container_tuple m_containers;

private:
  static constexpr
  bool
  s_noexcept_move_alloc_construct()
  noexcept
  {
    return std::is_nothrow_constructible_v<
        container_tuple,
        std::allocator_arg_t, allocator_type const &, container_tuple &&>;
  }

  static constexpr
  bool
  s_noexcept_swap()
  noexcept
  { return std::is_nothrow_swappable_v<container_tuple>; }

  template<typename ...Expressions>
  [[nodiscard]] constexpr
  bool
  m_matches_conjunction(identifier_type const id, conjunction<Expressions ...>) const
  noexcept
  { return (matches<Expressions>(id) && ...); }

  template<typename ...Expressions>
  [[nodiscard]] constexpr
  bool
  m_matches_disjunction(identifier_type const id, disjunction<Expressions ...>) const
  noexcept
  { return (matches<Expressions>(id) || ...); }

  template<typename Expression>
  [[nodiscard]] constexpr
  bool
  m_matches_negation(identifier_type const id, negation<Expression>) const
  noexcept
  { return !matches<Expression>(id); }

public:
  explicit constexpr
  generic_static_registry_storage(allocator_type const &alloc)
  noexcept
    : m_containers{std::allocator_arg, alloc}
  { }

  constexpr
  generic_static_registry_storage(generic_static_registry_storage const &other, allocator_type const &alloc)
    : m_containers{std::allocator_arg, alloc, other.m_containers}
  { }

  constexpr
  generic_static_registry_storage(generic_static_registry_storage const &)
  = default;

  constexpr
  generic_static_registry_storage(generic_static_registry_storage &&other, allocator_type const &alloc)
  noexcept(s_noexcept_move_alloc_construct())
    : m_containers{std::allocator_arg, alloc, std::move(other.m_containers)}
  { }

  constexpr
  generic_static_registry_storage(generic_static_registry_storage &&)
  = default;

  constexpr
  ~generic_static_registry_storage()
  = default;

  constexpr
  generic_static_registry_storage &
  operator=(generic_static_registry_storage const &)
  = default;

  constexpr
  generic_static_registry_storage &
  operator=(generic_static_registry_storage &&)
  = default;

  constexpr
  void
  swap(generic_static_registry_storage &other)
  noexcept(s_noexcept_swap())
  { std::swap(m_containers, other.m_containers); }

  friend constexpr
  void
  swap(generic_static_registry_storage &lhs, generic_static_registry_storage &rhs)
  noexcept(s_noexcept_swap())
  { lhs.swap(rhs); }

  [[nodiscard]] friend constexpr
  bool
  operator==(generic_static_registry_storage const &, generic_static_registry_storage const &)
  = default;


  template<typename Component>
  requires component_sequence::template contains<Component>
  [[nodiscard]] constexpr
  container_for<Component> &
  container()
  noexcept
  { return std::get<component_index<Component>>(m_containers); }

  template<typename Component>
  requires component_sequence::template contains<Component>
  [[nodiscard]] constexpr
  container_for<Component> const &
  container() const
  noexcept
  { return std::get<component_index<Component>>(m_containers); }

  template<typename Expression>
  [[nodiscard]] constexpr
  bool
  matches(identifier_type const id, Expression const = Expression{}) const
  noexcept
  {
    if      constexpr (is_specialization_of_conjunction_v<Expression>)
      return m_matches_conjunction(id, Expression{});
    else if constexpr (is_specialization_of_disjunction_v<Expression>)
      return m_matches_disjunction(id, Expression{});
    else if constexpr (is_specialization_of_negation_v   <Expression>)
      return m_matches_negation   (id, Expression{});
    else
      return container<Expression>().contains(id);
  }

  template<typename Component>
  requires (!std::is_empty_v<Component>)
  [[nodiscard]] constexpr
  Component &
  get(identifier_type const id)
  noexcept
  { return container<Component>()[id]; }

  template<typename Component>
  requires (!std::is_empty_v<Component>)
  [[nodiscard]] constexpr
  Component const &
  get(identifier_type const id) const
  noexcept
  { return container<Component>()[id]; }

  template<typename Component>
  [[nodiscard]] constexpr
  Component *
  get_if(identifier_type const id)
  noexcept
  {
    if (matches<Component>(id))
      return std::addressof(get<Component>(id));
    return nullptr;
  }

  template<typename Component>
  [[nodiscard]] constexpr
  Component const *
  get_if(identifier_type const id) const
  noexcept
  {
    if (matches<Component>(id))
      return std::addressof(get<Component>(id));
    return nullptr;
  }

  template<typename Component, typename ...Args>
  constexpr
  void
  emplace(identifier_type const id, Args &&...args)
  { container<Component>().emplace(id, std::forward<Args>(args)...); }

  template<typename Component, typename ...Args>
  constexpr
  bool
  try_emplace(identifier_type const id, Args &&...args)
  { return container<Component>().try_emplace(id, std::forward<Args>(args)...); }

  template<typename Component>
  constexpr
  bool
  insert(identifier_type const id, Component &&c)
  { return container<Component>().insert(id, std::forward<Component>(c)); }

  template<typename Component>
  constexpr
  bool
  insert_or_assign(identifier_type const id, Component &&c)
  { return container<Component>().insert_or_assign(id, std::forward<Component>(c)); }

  template<typename Component>
  constexpr
  void
  erase(identifier_type const id)
  { container<Component>().erase(id); }

  template<typename Component>
  constexpr
  bool
  try_erase(identifier_type const id)
  { return container<Component>().try_erase(id); }


  constexpr
  void
  clear(identifier_type const id)
  { (try_erase<Components>(id), ...); }

  constexpr
  void
  clear()
  noexcept
  { (container<Components>().clear(), ...); }
};


template<
    typename Expression,
    typename Registry>
class generic_static_registry_query_driver
{ };

template<
    typename Component,
    typename Registry>
requires component<Component>
class generic_static_registry_query_driver<Component, Registry>
{
public:
  using expression_type = Component;
  using registry_type   = Registry;

private:
  using iterator_type
  = typename registry_type::core_type::const_iterator;

public:
  constexpr
  generic_static_registry_query_driver()
  = default;

  constexpr
  generic_static_registry_query_driver(generic_static_registry_query_driver const &)
  = default;

  constexpr
  generic_static_registry_query_driver(generic_static_registry_query_driver &&)
  = default;

  explicit constexpr
  generic_static_registry_query_driver(registry_type const * const)
  noexcept
  { }

  constexpr
  ~generic_static_registry_query_driver()
  = default;

  constexpr
  generic_static_registry_query_driver &
  operator=(generic_static_registry_query_driver const &)
  = default;

  constexpr
  generic_static_registry_query_driver &
  operator=(generic_static_registry_query_driver &&)
  = default;

  [[nodiscard]] friend constexpr
  bool
  operator==(generic_static_registry_query_driver const &, generic_static_registry_query_driver const &)
  = default;

public:
  static constexpr
  iterator_type
  begin(registry_type const * const registry)
  noexcept
  { return registry->template container<expression_type>().begin(); }

  static constexpr
  iterator_type
  end(registry_type const * const registry)
  noexcept
  { return registry->template container<expression_type>().end(); }


  static constexpr
  iterator_type
  increment(registry_type const * const, iterator_type iterator)
  noexcept
  { return iterator; }

  static constexpr
  iterator_type
  decrement(registry_type const * const, iterator_type iterator)
  noexcept
  { return iterator; }
};

template<
    typename ...Expressions,
    typename    Registry>
class generic_static_registry_query_driver<
    conjunction<Expressions ...>,
    Registry>
{
public:
  using expression_type = conjunction<Expressions ...>;
  using registry_type   = Registry;

private:
  using registry_core_type
  = std::conditional_t<
      std::is_const_v<registry_type>,
      typename registry_type::core_type const,
      typename registry_type::core_type>;

  using iterator_type
  = typename registry_type::core_type::const_iterator;

private:
  iterator_type m_begin;
  iterator_type m_end;

private:
  template<typename Component>
  constexpr
  void
  m_initialize_unfold(registry_type const * const registry, std::size_t &size)
  noexcept
  {
    auto        const &cont     {registry->template container<Component>()};
    std::size_t const  cont_size{cont.size()};

    if (cont_size >= size)
      return;

    size    = cont_size;
    m_begin = cont.begin();
    m_end   = cont.end();
  }

  template<typename ...Components>
  constexpr
  void
  m_initialize(
      registry_type const *         const registry,
      type_sequence<Components ...> const          = type_sequence<Components ...>{})
  noexcept
  {
    std::size_t size{registry->core_type::size()};

    (m_initialize_unfold<Components>(registry, size), ...);
    m_begin = increment(registry, m_begin);
  }

public:
  constexpr
  generic_static_registry_query_driver()
  noexcept
    : m_begin{}
    , m_end  {}
  { }

  constexpr
  generic_static_registry_query_driver(generic_static_registry_query_driver const &)
  = default;

  constexpr
  generic_static_registry_query_driver(generic_static_registry_query_driver &&)
  = default;

  explicit constexpr
  generic_static_registry_query_driver(registry_type const * const registry)
  noexcept
    : m_begin{registry->core_type::begin()}
    , m_end  {registry->core_type::end  ()}
  { m_initialize(registry, guaranteed_t<expression_type>{}); }

  constexpr
  ~generic_static_registry_query_driver()
  = default;

  constexpr
  generic_static_registry_query_driver &
  operator=(generic_static_registry_query_driver const &)
  = default;

  constexpr
  generic_static_registry_query_driver &
  operator=(generic_static_registry_query_driver &&)
  = default;

  constexpr
  void
  swap(generic_static_registry_query_driver &other)
  noexcept
  {
    using std::swap;

    swap(m_begin, other.m_begin);
    swap(m_end  , other.m_end);
  }

  friend constexpr
  void
  swap(generic_static_registry_query_driver &lhs, generic_static_registry_query_driver &rhs)
  noexcept
  { lhs.swap(rhs); }

  [[nodiscard]] friend constexpr
  bool
  operator==(generic_static_registry_query_driver const &, generic_static_registry_query_driver const &)
  = default;


  constexpr
  iterator_type
  begin(registry_type const * const) const
  noexcept
  { return m_begin; }

  constexpr
  iterator_type
  end(registry_type const * const) const
  noexcept
  { return m_end; }


  constexpr
  iterator_type
  increment(registry_type const * const registry, iterator_type iterator) const
  noexcept
  {
    while (iterator != m_end && !registry->template matches<expression_type>(*iterator))
      ++iterator;

    return iterator;
  }

  constexpr
  iterator_type
  decrement(registry_type const * const registry, iterator_type iterator) const
  noexcept
  {
    while (iterator != m_begin && !registry->template matches<expression_type>(*iterator))
      --iterator;

    return iterator;
  }
};

template<
    typename ...Expressions,
    typename    Registry>
class generic_static_registry_query_driver<
    disjunction<Expressions ...>,
    Registry>
{
public:
  using expression_type = disjunction<Expressions ...>;
  using registry_type   = Registry;

private:
  using registry_core_type
  = std::conditional_t<
      std::is_const_v<registry_type>,
      typename registry_type::core_type const,
      typename registry_type::core_type>;

  using iterator_type
  = typename registry_core_type::const_iterator;

private:
  iterator_type m_begin;

public:
  constexpr
  generic_static_registry_query_driver()
  noexcept
    : m_begin{}
  { }

  constexpr
  generic_static_registry_query_driver(generic_static_registry_query_driver const &)
  = default;

  constexpr
  generic_static_registry_query_driver(generic_static_registry_query_driver &&)
  = default;

  explicit constexpr
  generic_static_registry_query_driver(registry_type const * const registry)
  noexcept
    : m_begin{increment(registry, registry->core_type::begin())}
  { }

  constexpr
  ~generic_static_registry_query_driver()
  = default;

  constexpr
  generic_static_registry_query_driver &
  operator=(generic_static_registry_query_driver const &)
  = default;

  constexpr
  generic_static_registry_query_driver &
  operator=(generic_static_registry_query_driver &&)
  = default;

  [[nodiscard]] friend constexpr
  bool
  operator==(generic_static_registry_query_driver const &, generic_static_registry_query_driver const &)
  = default;

public:
  constexpr
  iterator_type
  begin(registry_type const * const) const
  noexcept
  { return m_begin; }

  static constexpr
  iterator_type
  end(registry_type const * const registry)
  noexcept
  { return registry->core_type::end(); }


  static constexpr
  iterator_type
  increment(registry_type const * const registry, iterator_type iterator)
  noexcept
  {
    iterator_type const last{end(registry)};

    while (iterator != last && !registry->template matches<expression_type>(*iterator))
      ++iterator;

    return iterator;
  }

  constexpr
  iterator_type
  decrement(registry_type const * const registry, iterator_type iterator) const
  noexcept
  {
    while (iterator != m_begin && !registry->template matches<expression_type>(*iterator))
      --iterator;

    return iterator;
  }
};

template<
    typename Expression,
    typename Registry>
class generic_static_registry_query_driver<
    negation<Expression>,
    Registry>
{
public:
  using expression_type = negation<Expression>;
  using registry_type   = Registry;

private:
  using registry_core_type
  = std::conditional_t<
      std::is_const_v<registry_type>,
      typename registry_type::core_type const,
      typename registry_type::core_type>;

  using iterator_type
  = typename registry_core_type::const_iterator;

private:
  iterator_type m_begin;

public:
  constexpr
  generic_static_registry_query_driver()
  noexcept
    : m_begin{}
  { }

  constexpr
  generic_static_registry_query_driver(generic_static_registry_query_driver const &)
  = default;

  constexpr
  generic_static_registry_query_driver(generic_static_registry_query_driver &&)
  = default;

  explicit constexpr
  generic_static_registry_query_driver(registry_type const * const registry)
  noexcept
    : m_begin{increment(registry, registry->core_type::begin())}
  { }

  constexpr
  ~generic_static_registry_query_driver()
  = default;

  constexpr
  generic_static_registry_query_driver &
  operator=(generic_static_registry_query_driver const &)
  = default;

  constexpr
  generic_static_registry_query_driver &
  operator=(generic_static_registry_query_driver &&)
  = default;

  [[nodiscard]] friend constexpr
  bool
  operator==(generic_static_registry_query_driver const &, generic_static_registry_query_driver const &)
  = default;

public:
  constexpr
  iterator_type
  begin(registry_type const * const) const
  noexcept
  { return m_begin; }

  static constexpr
  iterator_type
  end(registry_type const * const registry)
  noexcept
  { return registry->core_type::end(); }


  static constexpr
  iterator_type
  increment(registry_type const * const registry, iterator_type iterator)
  noexcept
  {
    iterator_type const last{end(registry)};

    while (iterator != last && registry->template matches<Expression>(*iterator))
      ++iterator;

    return iterator;
  }

  constexpr
  iterator_type
  decrement(registry_type const * const registry, iterator_type iterator) const
  noexcept
  {
    while (iterator != m_begin && registry->template matches<Expression>(*iterator))
      --iterator;

    return iterator;
  }
};


template<
    typename Driver,
    typename Registry>
class generic_static_registry_query_iterator
{
public:
  using driver_type     = Driver;
  using registry_type   = Registry;

  using difference_type  = std::ptrdiff_t;
  using iterator_concept = std::bidirectional_iterator_tag;
  using value_type       = entity<registry_type>;
  using reference        = entity<registry_type>;

private:
  using iterator_type
  = typename registry_type::core_type::const_iterator;

private:
  [[no_unique_address]]
  driver_type m_driver;

  registry_type *m_registry;
  iterator_type  m_iterator;

public:
  constexpr
  generic_static_registry_query_iterator()
  noexcept
    : m_driver  {}
    , m_registry{}
    , m_iterator{}
  { }

  constexpr
  generic_static_registry_query_iterator(generic_static_registry_query_iterator const &)
  = default;

  constexpr
  generic_static_registry_query_iterator(generic_static_registry_query_iterator &&)
  = default;

  constexpr
  generic_static_registry_query_iterator(
      driver_type              const driver,
      registry_type *          const registry,
      std::bool_constant<true> const)
  noexcept
    : m_driver  {driver}
    , m_registry{registry}
    , m_iterator{m_driver.begin(m_registry)}
  { }

  constexpr
  generic_static_registry_query_iterator(
      driver_type               const driver,
      registry_type *           const registry,
      std::bool_constant<false> const)
  noexcept
    : m_driver  {driver}
    , m_registry{registry}
    , m_iterator{m_driver.end(m_registry)}
  { }

  constexpr
  ~generic_static_registry_query_iterator()
  = default;

  constexpr
  generic_static_registry_query_iterator &
  operator=(generic_static_registry_query_iterator const &)
  = default;

  constexpr
  generic_static_registry_query_iterator &
  operator=(generic_static_registry_query_iterator &&)
  = default;

  constexpr
  void
  swap(generic_static_registry_query_iterator &other)
  noexcept
  {
    using std::swap;

    swap(m_registry, other.m_registry);
    swap(m_iterator, other.m_iterator);
    swap(m_driver  , other.m_driver);
  }

  friend constexpr
  void
  swap(generic_static_registry_query_iterator &lhs, generic_static_registry_query_iterator &rhs)
  noexcept
  { lhs.swap(rhs); }

  [[nodiscard]] friend constexpr
  bool
  operator==(generic_static_registry_query_iterator const &, generic_static_registry_query_iterator const &)
  = default;


  [[nodiscard]] constexpr
  reference
  operator*() const
  noexcept
  { return reference{*m_registry, *m_iterator}; }


  constexpr
  generic_static_registry_query_iterator &
  operator++()
  noexcept
  {
    m_iterator = m_driver.increment(m_registry, ++m_iterator);
    return *this;
  }

  constexpr
  generic_static_registry_query_iterator &
  operator--()
  noexcept
  {
    m_iterator = m_driver.decrement(m_registry, --m_iterator);
    return *this;
  }

  constexpr
  generic_static_registry_query_iterator
  operator++(int)
  noexcept
  {
    generic_static_registry_query_iterator tmp{*this};
    ++*this;
    return tmp;
  }

  constexpr
  generic_static_registry_query_iterator
  operator--(int)
  noexcept
  {
    generic_static_registry_query_iterator tmp{*this};
    --*this;
    return tmp;
  }
};


template<
    typename Expression,
    typename Registry>
class generic_static_registry_query
  : public std::ranges::view_interface<generic_static_registry_query<Expression, Registry>>
{
public:
  using expression_type = Expression;
  using registry_type   = Registry;

private:
  using driver_type
  = generic_static_registry_query_driver<expression_type, registry_type>;

public:
  using iterator       = generic_static_registry_query_iterator<driver_type, registry_type>;
  using const_iterator = generic_static_registry_query_iterator<driver_type, registry_type const>;

private:
  registry_type *m_registry;
  driver_type    m_driver;

public:
  constexpr
  generic_static_registry_query()
  noexcept
    : m_registry{}
    , m_driver  {}
  { }

  constexpr
  generic_static_registry_query(generic_static_registry_query const &)
  = default;

  constexpr
  generic_static_registry_query(generic_static_registry_query &&)
  = default;

  explicit constexpr
  generic_static_registry_query(registry_type &registry)
  noexcept
    : m_registry{&registry}
    , m_driver  {m_registry}
  { }

  constexpr
  ~generic_static_registry_query()
  = default;

  constexpr
  generic_static_registry_query &
  operator=(generic_static_registry_query const &)
  = default;

  constexpr
  generic_static_registry_query &
  operator=(generic_static_registry_query &&)
  = default;

  constexpr
  void
  swap(generic_static_registry_query &other)
  noexcept
  {
    using std::swap;

    swap(m_registry, other.m_registry);
    swap(m_driver  , other.m_driver);
  }

  friend constexpr
  void
  swap(generic_static_registry_query &lhs, generic_static_registry_query &rhs)
  noexcept
  { lhs.swap(rhs); }

  [[nodiscard]] friend constexpr
  bool
  operator==(generic_static_registry_query const &, generic_static_registry_query const &)
  = default;


  [[nodiscard]] constexpr
  iterator
  begin()
  noexcept
  { return iterator{m_driver, m_registry, std::bool_constant<true>{}}; }

  [[nodiscard]] constexpr
  const_iterator
  begin() const
  noexcept
  { return const_iterator{m_driver, m_registry, std::bool_constant<true>{}}; }

  [[nodiscard]] constexpr
  iterator
  end()
  noexcept
  { return iterator{m_driver, m_registry, std::bool_constant<false>{}}; }

  [[nodiscard]] constexpr
  const_iterator
  end() const
  noexcept
  { return const_iterator{m_driver, m_registry, std::bool_constant<false>{}}; }

  [[nodiscard]] constexpr
  const_iterator
  cbegin() const
  noexcept
  { return const_iterator{m_driver, m_registry, std::bool_constant<true>{}}; }

  [[nodiscard]] constexpr
  const_iterator
  cend() const
  noexcept
  { return const_iterator{m_driver, m_registry, std::bool_constant<false>{}}; }
};

} // namespace detail


template<
    typename Identifier   = default_identifier_t<>,
    typename Allocator    = std::allocator<Identifier>,
    typename DescSequence = type_sequence<>>
class generic_static_registry
  : protected detail::registry_core                  <Identifier, Allocator>
  , protected detail::generic_static_registry_storage<Identifier, Allocator, DescSequence>
{
  using core_type    = detail::registry_core                  <Identifier, Allocator>;
  using storage_type = detail::generic_static_registry_storage<Identifier, Allocator, DescSequence>;

  template<typename>           friend class detail::registry_iterator;
  template<typename, typename> friend class detail::generic_static_registry_query_driver;
  template<typename, typename> friend class detail::generic_static_registry_query_iterator;

public:
  using identifier_type      = Identifier;
  using allocator_type       = Allocator;
  using description_sequence = DescSequence;

  using iterator       = detail::registry_iterator<generic_static_registry>;
  using const_iterator = detail::registry_iterator<generic_static_registry const>;


  template<
      typename    Component,
      std::size_t PageSize  = default_page_size_v<>>
  using with
  = generic_static_registry<
      identifier_type,
      allocator_type,
      type_sequence_append_t<
          description_sequence,
          detail::generic_static_registry_descriptor<Component, PageSize>>>;

  template<typename ...Components>
  using with_all
  = generic_static_registry<
      identifier_type,
      allocator_type,
      type_sequence_append_t<
          description_sequence,
          detail::generic_static_registry_descriptor<Components> ...>>;

private:
  static constexpr
  bool
  s_noexcept_default_construct()
  noexcept
  { return std::is_nothrow_default_constructible_v<allocator_type>; }

  static constexpr
  bool
  s_noexcept_move_alloc_construct()
  noexcept
  {
    return
        std::is_nothrow_constructible_v<
            core_type,
            core_type &&, allocator_type const &>
     && std::is_nothrow_constructible_v<
            storage_type,
            storage_type &&, allocator_type const &>;
  }

  static constexpr
  bool
  s_noexcept_swap()
  noexcept
  {
    return std::is_nothrow_swappable_v<core_type>
        && std::is_nothrow_swappable_v<storage_type>;
  }

public:
  explicit constexpr
  generic_static_registry(allocator_type const &alloc)
  noexcept
    : core_type   {alloc}
    , storage_type{alloc}
  { }

  constexpr
  generic_static_registry()
  noexcept(s_noexcept_default_construct())
    : generic_static_registry{allocator_type{}}
  { }

  constexpr
  generic_static_registry(generic_static_registry const &other, allocator_type const &alloc)
    : core_type   {static_cast<core_type    const &>(other), alloc}
    , storage_type{static_cast<storage_type const &>(other), alloc}
  { }

  constexpr
  generic_static_registry(generic_static_registry const &)
  = default;

  constexpr
  generic_static_registry(generic_static_registry &&other, allocator_type const &alloc)
  noexcept(s_noexcept_move_alloc_construct())
    : core_type   {static_cast<core_type    &&>(other), alloc}
    , storage_type{static_cast<storage_type &&>(other), alloc}
  { }

  constexpr
  generic_static_registry(generic_static_registry &&)
  = default;

  constexpr
  ~generic_static_registry()
  = default;

  constexpr
  generic_static_registry &
  operator=(generic_static_registry const &)
  = default;

  constexpr
  generic_static_registry &
  operator=(generic_static_registry &&)
  = default;

  constexpr
  void
  swap(generic_static_registry &other)
  noexcept(s_noexcept_swap())
  {
    core_type   ::swap(static_cast<core_type    &>(other));
    storage_type::swap(static_cast<storage_type &>(other));
  }

  friend constexpr
  void
  swap(generic_static_registry &lhs, generic_static_registry &rhs)
  noexcept(s_noexcept_swap())
  { lhs.swap(rhs); }

  [[nodiscard]] friend constexpr
  bool
  operator==(generic_static_registry const &, generic_static_registry const &)
  = default;

  [[nodiscard]] constexpr
  allocator_type
  get_allocator() const
  noexcept
  { return core_type::get_allocator(); }


  [[nodiscard]] constexpr
  iterator
  begin()
  noexcept
  { return iterator{*this, core_type::begin()}; }

  [[nodiscard]] constexpr
  const_iterator
  begin() const
  noexcept
  { return const_iterator{*this, core_type::begin()}; }

  [[nodiscard]] constexpr
  iterator
  end()
  noexcept
  { return iterator{*this, core_type::end()}; }

  [[nodiscard]] constexpr
  const_iterator
  end() const
  noexcept
  { return const_iterator{*this, core_type::end()}; }


  [[nodiscard]] constexpr
  const_iterator
  cbegin() const
  noexcept
  { return const_iterator{*this, core_type::cbegin()}; }

  [[nodiscard]] constexpr
  const_iterator
  cend() const
  noexcept
  { return const_iterator{*this, core_type::cend()}; }

  [[nodiscard]] constexpr
  std::size_t
  size() const
  noexcept
  { return core_type::size(); }

  [[nodiscard]] constexpr
  bool
  empty() const
  noexcept
  { return core_type::empty(); }


  [[nodiscard]] constexpr
  bool
  expired(identifier_type const id) const
  noexcept
  { return core_type::expired(id); }

  template<typename Expression>
  [[nodiscard]] constexpr
  bool
  matches(identifier_type const id, Expression const = Expression{}) const
  noexcept
  { return storage_type::template matches<Expression>(id); }

  template<typename Expression>
  [[nodiscard]] constexpr
  auto
  query()
  noexcept
  {
    return detail::generic_static_registry_query<
        Expression,
        generic_static_registry>
        {*this};
  }

  template<typename Expression>
  [[nodiscard]] constexpr
  auto
  query() const
  noexcept
  {
    return detail::generic_static_registry_query<
        Expression,
        generic_static_registry const>
        {*this};
  }

  template<typename Component>
  [[nodiscard]] constexpr
  Component &
  get(identifier_type const id)
  noexcept
  { return storage_type::template get<Component>(id); }

  template<typename Component>
  [[nodiscard]] constexpr
  Component const &
  get(identifier_type const id) const
  noexcept
  { return storage_type::template get<Component>(id); }

  template<typename Component>
  [[nodiscard]] constexpr
  Component *
  get_if(identifier_type const id)
  noexcept
  { return storage_type::template get_if<Component>(id); }

  template<typename Component>
  [[nodiscard]] constexpr
  Component const *
  get_if(identifier_type const id) const
  noexcept
  { return storage_type::template get_if<Component>(id); }


  [[nodiscard]] constexpr
  auto
  entity()
  { return heim::entity<generic_static_registry>{*this, core_type::create()}; }

  template<typename Component, typename ...Args>
  constexpr
  void
  emplace(identifier_type const id, Args &&...args)
  { storage_type::template emplace<Component>(id, std::forward<Args>(args)...); }

  template<typename Component, typename ...Args>
  constexpr
  bool
  try_emplace(identifier_type const id, Args &&...args)
  { return storage_type::template try_emplace<Component>(id, std::forward<Args>(args)...); }

  template<typename Component>
  constexpr
  bool
  insert(identifier_type const id, Component &&c)
  { return storage_type::template insert<Component>(id, std::forward<Component>(c)); }

  template<typename Component>
  constexpr
  bool
  insert_or_assign(identifier_type const id, Component &&c)
  { return storage_type::template insert_or_assign<Component>(id, std::forward<Component>(c)); }

  template<typename Component>
  constexpr
  void
  erase(identifier_type const id)
  { storage_type::template erase<Component>(id); }

  template<typename Component>
  constexpr
  bool
  try_erase(identifier_type const id)
  { return storage_type::template try_erase<Component>(id); }

  constexpr
  void
  clear(identifier_type const id)
  { storage_type::clear(id); }

  constexpr
  void
  clear()
  noexcept
  { storage_type::clear(); core_type::clear(); }

  constexpr
  bool
  destroy(identifier_type const id)
  {
    if (expired(id))
      return false;

    clear(id);
    core_type::destroy(id);
    return true;
  }
};

using static_registry
= generic_static_registry<>;


} // namespace heim::sparse

#endif // HEIM_STATIC_REGISTRY_HPP
