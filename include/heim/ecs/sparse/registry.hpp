#ifndef HEIM_ECS_SPARSE_REGISTRY_HPP
#define HEIM_ECS_SPARSE_REGISTRY_HPP

#include <cstddef>
#include <iterator>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>
#include "heim/ecs/component.hpp"
#include "heim/ecs/entity.hpp"
#include "heim/ecs/identifier.hpp"
#include "heim/ecs/expression.hpp"
#include "heim/lib/type_sequence.hpp"
#include "heim/lib/utility.hpp"
#include "pool.hpp"

namespace heim
{
namespace sparse
{
/*!
 * \brief
 *   The main sparse-set-based container for entities and their components.
 */
template<
    typename Identifier   = default_identifier_t<>,
    typename Allocator    = std::allocator<Identifier>,
    typename DescSequence = type_sequence<>>
class registry;


template<typename>
struct is_specialization_of_registry
  : std::false_type
{ };

template<
    typename Identifier,
    typename Allocator,
    typename DescSequence>
struct is_specialization_of_registry<
    registry<Identifier, Allocator, DescSequence>>
  : std::true_type
{ };

template<typename T>
inline constexpr
bool
is_specialization_of_registry_v
= is_specialization_of_registry<T>::value;

template<typename T>
concept specialization_of_registry
= is_specialization_of_registry_v<T>;


namespace detail
{
template<
    typename Identifier = default_identifier_t<>,
    typename Allocator  = std::allocator<Identifier>>
requires (
    identifier   <Identifier>
 && allocator_for<Allocator, Identifier>)
class registry_core
{
public:
  using identifier_type = Identifier;
  using allocator_type  = Allocator;

private:
  using id_traits    = identifier_traits<identifier_type>;
  using alloc_traits = std::allocator_traits<allocator_type>;

  using container_type
  = std::vector<identifier_type, allocator_type>;

public:
  using iterator       = typename container_type::const_reverse_iterator;
  using const_iterator = typename container_type::const_reverse_iterator;

private:
  container_type m_dense;
  container_type m_sparse;
  std::size_t    m_begin;

private:
  static constexpr
  bool
  s_noexcept_move_alloc_construct()
  noexcept
  {
    return std::is_nothrow_constructible_v<
        container_type,
        container_type &&, allocator_type const &>;
  }

  static constexpr
  bool
  s_noexcept_swap()
  noexcept
  { return std::is_nothrow_swappable_v<container_type>; }

public:
  explicit constexpr
  registry_core(allocator_type const &alloc)
    : m_dense {alloc}
    , m_sparse{alloc}
    , m_begin {0}
  { }

  constexpr
  registry_core(registry_core const &other, allocator_type const &alloc)
    : m_dense {other.m_dense , alloc}
    , m_sparse{other.m_sparse, alloc}
    , m_begin {other.m_begin}
  { }

  constexpr
  registry_core(registry_core const &)
  = default;

  constexpr
  registry_core(registry_core &&other, allocator_type const &alloc)
  noexcept(s_noexcept_move_alloc_construct())
    : m_dense {std::move(other.m_dense ), alloc}
    , m_sparse{std::move(other.m_sparse), alloc}
    , m_begin {other.m_begin}
  { }

  constexpr
  registry_core(registry_core &&)
  = default;

  constexpr
  ~registry_core()
  = default;

  constexpr
  registry_core &
  operator=(registry_core const &)
  = default;

  constexpr
  registry_core &
  operator=(registry_core &&)
  = default;

  constexpr
  void
  swap(registry_core &other)
  noexcept(s_noexcept_swap())
  {
    std::swap(m_dense , other.m_dense);
    std::swap(m_sparse, other.m_sparse);
    std::swap(m_begin , other.m_begin);
  }

  [[nodiscard]] friend constexpr
  bool
  operator==(registry_core const &lhs, registry_core const &rhs)
  = default;

  [[nodiscard]] constexpr
  allocator_type
  get_allocator() const
  noexcept
  { return m_dense.get_allocator(); }


  [[nodiscard]] constexpr
  iterator
  begin()
  noexcept
  { return std::make_reverse_iterator(m_dense.end()); }

  [[nodiscard]] constexpr
  const_iterator
  begin() const
  noexcept
  { return std::make_reverse_iterator(m_dense.end()); }

  [[nodiscard]] constexpr
  iterator
  end()
  noexcept
  { return std::make_reverse_iterator(m_dense.begin() + static_cast<std::ptrdiff_t>(m_begin)); }

  [[nodiscard]] constexpr
  const_iterator
  end() const
  noexcept
  { return std::make_reverse_iterator(m_dense.begin() + static_cast<std::ptrdiff_t>(m_begin)); }

  [[nodiscard]] constexpr
  const_iterator
  cbegin() const
  noexcept
  { return std::make_reverse_iterator(m_dense.cend()); }

  [[nodiscard]] constexpr
  const_iterator
  cend() const
  noexcept
  { return std::make_reverse_iterator(m_dense.cbegin() + static_cast<std::ptrdiff_t>(m_begin)); }

  [[nodiscard]] constexpr
  std::size_t
  size() const
  noexcept
  { return m_dense.size() - m_begin; }

  [[nodiscard]] constexpr
  bool
  empty() const
  noexcept
  { return size() == 0; }


  [[nodiscard]] constexpr
  bool
  is_valid(identifier_type const id) const
  noexcept
  {
    auto const idx = static_cast<std::size_t>(id_traits::index(id));

    if (idx >= m_sparse.size())
      return false;

    identifier_type const pos = m_sparse[idx];

    return id_traits::index     (pos) >= m_begin
        && id_traits::generation(pos) == id_traits::generation(id);
  }


  [[nodiscard]] constexpr
  identifier_type
  create()
  {
    using index_type
    = typename id_traits::index_type;

    if (m_begin != 0)
      return m_dense[--m_begin];

    identifier_type const id = id_traits::from(static_cast<index_type>(m_dense.size()), 0);

    m_dense.emplace_back(id);
    // strong exception safety guarantee
    try
    { m_sparse.emplace_back(id); }
    catch (...)
    { m_dense.pop_back(); throw; }

    return id;
  }

  constexpr
  void
  destroy(identifier_type const id)
  noexcept
  {
    using index_type
    = typename id_traits::index_type;

    identifier_type &pos        = m_sparse[static_cast<std::size_t>(id_traits::index(id))];
    auto const       pos_idx    = id_traits::index(pos);

    identifier_type &dense_begin = m_dense[m_begin];
    identifier_type &dense_id    = m_dense[static_cast<std::size_t>(pos_idx)];
    auto const       begin_idx   = static_cast<std::size_t>(id_traits::index(dense_begin));
    auto const       begin       = static_cast<index_type>(m_begin);

    std::swap(dense_begin, dense_id);

    if (pos_idx != begin)
      m_sparse[begin_idx] = id_traits::from(pos_idx, id_traits::generation(dense_id));

    dense_begin = id_traits::next(dense_begin);
    pos         = id_traits::from(begin, id_traits::generation(dense_begin));
    ++m_begin;
  }

  constexpr
  void
  clear()
  noexcept
  {
    auto valid = m_dense | std::views::drop(m_begin);

    // we shortcut the individual banish method to avoid unnecessary swap attempts
    for (identifier_type &id : valid)
    {
      identifier_type &pos = m_sparse[static_cast<std::size_t>(id_traits::index(id))];
      
      id  = id_traits::next(id);
      pos = id_traits::next(pos);
    }

    m_begin = m_dense.size();
  }
};


template<
    typename    Component,
    std::size_t PageSize  = default_page_size_v<>>
requires component<Component>
using registry_descriptor
= type_sequence<Component, std::integral_constant<std::size_t, PageSize>>;


template<
    typename Identifier   = default_identifier_t<>,
    typename Allocator    = std::allocator<Identifier>,
    typename DescSequence = type_sequence<>>
class registry_storage
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
class registry_storage<
    Identifier,
    Allocator,
    type_sequence<registry_descriptor<Components, PageSizes> ...>>
{
public:
  using identifier_type      = Identifier;
  using allocator_type       = Allocator;
  using description_sequence = type_sequence<registry_descriptor<Components, PageSizes> ...>;

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
  requires (expression<Expressions> && ...)
  [[nodiscard]] constexpr
  bool
  m_is_match_conjunction(identifier_type const id, conjunction<Expressions ...>) const
  noexcept
  { return (is_match<Expressions>(id) && ...); }

  template<typename ...Expressions>
  requires (expression<Expressions> && ...)
  [[nodiscard]] constexpr
  bool
  m_is_match_disjunction(identifier_type const id, disjunction<Expressions ...>) const
  noexcept
  { return (is_match<Expressions>(id) || ...); }

  template<typename Expression>
  requires expression<Expression>
  [[nodiscard]] constexpr
  bool
  m_is_match_negation(identifier_type const id, negation<Expression>) const
  noexcept
  { return !is_match<Expression>(id); }

public:
  explicit constexpr
  registry_storage(allocator_type const &alloc)
  noexcept
    : m_containers{std::allocator_arg, alloc}
  { }

  constexpr
  registry_storage(registry_storage const &other, allocator_type const &alloc)
    : m_containers{std::allocator_arg, alloc, other.m_containers}
  { }

  constexpr
  registry_storage(registry_storage const &)
  = default;

  constexpr
  registry_storage(registry_storage &&other, allocator_type const &alloc)
  noexcept(s_noexcept_move_alloc_construct())
    : m_containers{std::allocator_arg, alloc, std::move(other.m_containers)}
  { }

  constexpr
  registry_storage(registry_storage &&)
  = default;

  constexpr
  ~registry_storage()
  = default;

  constexpr
  registry_storage &
  operator=(registry_storage const &)
  = default;

  constexpr
  registry_storage &
  operator=(registry_storage &&)
  = default;

  constexpr
  void
  swap(registry_storage &other)
  noexcept(s_noexcept_swap())
  { std::swap(m_containers, other.m_containers); }

  friend constexpr
  void
  swap(registry_storage &lhs, registry_storage &rhs)
  noexcept(s_noexcept_swap())
  { lhs.swap(rhs); }

  [[nodiscard]] friend constexpr
  bool
  operator==(registry_storage const &, registry_storage const &)
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
  requires expression<Expression>
  [[nodiscard]] constexpr
  bool
  is_match(identifier_type const id, Expression const = Expression{}) const
  noexcept
  {
    if      constexpr (is_specialization_of_conjunction_v<Expression>)
      return m_is_match_conjunction(id, Expression{});
    else if constexpr (is_specialization_of_disjunction_v<Expression>)
      return m_is_match_disjunction(id, Expression{});
    else if constexpr (is_specialization_of_negation_v   <Expression>)
      return m_is_match_negation   (id, Expression{});
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
  Component &
  try_get(identifier_type const id)
  {
    if (is_match<Component>(id))
      return get<Component>(id);
    throw std::out_of_range("heim::sparse::detail::registry_storage::try_get");
  }

  template<typename Component>
  [[nodiscard]] constexpr
  Component const &
  try_get(identifier_type const id) const
  {
    if (is_match<Component>(id))
      return get<Component>(id);
    throw std::out_of_range("heim::sparse::detail::registry_storage::try_get");
  }

  template<typename Component>
  [[nodiscard]] constexpr
  Component *
  get_if(identifier_type const id)
  noexcept
  {
    if (is_match<Component>(id))
      return std::addressof(get<Component>(id));
    return nullptr;
  }

  template<typename Component>
  [[nodiscard]] constexpr
  Component const *
  get_if(identifier_type const id) const
  noexcept
  {
    if (is_match<Component>(id))
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


template<typename Registry>
class registry_iterator
{ };

template<typename Registry>
requires specialization_of_registry<std::remove_cvref_t<Registry>>
class registry_iterator<Registry>
{
public:
  using registry_type
  = Registry;

  using difference_type  = std::ptrdiff_t;
  using iterator_concept = std::random_access_iterator_tag;
  using value_type       = entity<registry_type>;
  using reference        = entity<registry_type>;

private:
  using iterator_type
  = typename registry_type::core_type::const_iterator;

private:
  registry_type *m_registry;
  iterator_type  m_iterator;

public:
  constexpr
  registry_iterator()
  noexcept
    : m_registry{nullptr}
    , m_iterator{}
  { }

  constexpr
  registry_iterator(registry_iterator const &)
  = default;

  constexpr
  registry_iterator(registry_iterator &&)
  = default;

  constexpr
  registry_iterator(registry_type &registry, iterator_type const iterator)
  noexcept
    : m_registry{&registry}
    , m_iterator{iterator}
  { }

  constexpr
  ~registry_iterator()
  = default;

  constexpr
  registry_iterator &
  operator=(registry_iterator const &)
  = default;

  constexpr
  registry_iterator &
  operator=(registry_iterator &&)
  = default;

  constexpr
  void
  swap(registry_iterator &other)
  noexcept
  {
    using std::swap;

    swap(m_registry, other.m_registry);
    swap(m_iterator, other.m_iterator);
  }

  friend constexpr
  void
  swap(registry_iterator &lhs, registry_iterator &rhs)
  noexcept
  { lhs.swap(rhs); }

  [[nodiscard]] friend constexpr
  bool
  operator==(registry_iterator const &, registry_iterator const &)
  = default;

  [[nodiscard]] friend constexpr
  auto
  operator<=>(registry_iterator const &, registry_iterator const &)
  = default;


  [[nodiscard]] constexpr
  reference
  operator*() const
  noexcept
  { return reference(*m_registry, *m_iterator); }

  [[nodiscard]] constexpr
  reference
  operator[](difference_type const n) const
  noexcept
  { return *(*this + n); }


  constexpr
  registry_iterator &
  operator++()
  noexcept
  { ++m_iterator; return *this; }

  constexpr
  registry_iterator &
  operator--()
  noexcept
  { --m_iterator; return *this; }

  constexpr
  registry_iterator
  operator++(int)
  noexcept
  { registry_iterator tmp(*this); ++*this; return tmp; }

  constexpr
  registry_iterator
  operator--(int)
  noexcept
  { registry_iterator tmp(*this); --*this; return tmp; }

  constexpr
  registry_iterator &
  operator+=(difference_type const n)
  { m_iterator += n; return *this; }

  constexpr
  registry_iterator &
  operator-=(difference_type const n)
  { m_iterator -= n; return *this; }

  friend constexpr
  registry_iterator
  operator+(registry_iterator it, difference_type const n)
  noexcept
  { it += n; return it; }

  friend constexpr
  registry_iterator
  operator+(difference_type const n, registry_iterator it)
  noexcept
  { it += n; return it; }

  friend constexpr
  registry_iterator
  operator-(registry_iterator it, difference_type const n)
  noexcept
  { it -= n; return it; }

  friend constexpr
  difference_type
  operator-(registry_iterator const lhs, registry_iterator const rhs)
  noexcept
  { return lhs.m_iterator - rhs.m_iterator; }
};


template<
    typename Expression,
    typename Registry>
class registry_query_driver
{ };

template<
    typename Component,
    typename Registry>
requires (
    component                 <Component>
 && specialization_of_registry<std::remove_cvref_t<Registry>>)
class registry_query_driver<Component, Registry>
{
public:
  using expression_type = Component;
  using registry_type   = Registry;

private:
  using iterator_type
  = typename registry_type::core_type::const_iterator;

public:
  constexpr
  registry_query_driver()
  = default;

  constexpr
  registry_query_driver(registry_query_driver const &)
  = default;

  constexpr
  registry_query_driver(registry_query_driver &&)
  = default;

  explicit constexpr
  registry_query_driver(registry_type const * const)
  noexcept
  { }

  constexpr
  ~registry_query_driver()
  = default;

  constexpr
  registry_query_driver &
  operator=(registry_query_driver const &)
  = default;

  constexpr
  registry_query_driver &
  operator=(registry_query_driver &&)
  = default;

  [[nodiscard]] friend constexpr
  bool
  operator==(registry_query_driver const &, registry_query_driver const &)
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
requires (
    (expression<Expressions> && ...)
 && specialization_of_registry<std::remove_cvref_t<Registry>>)
class registry_query_driver<
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
    auto        const &cont      = registry->template container<Component>();
    std::size_t const  cont_size = cont.size();

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
    std::size_t size = registry->core_type::size();
    (m_initialize_unfold<Components>(registry, size), ...);
    m_begin = increment(registry, m_begin);
  }

public:
  constexpr
  registry_query_driver()
  noexcept
    : m_begin{}
    , m_end  {}
  { }

  constexpr
  registry_query_driver(registry_query_driver const &)
  = default;

  constexpr
  registry_query_driver(registry_query_driver &&)
  = default;

  explicit constexpr
  registry_query_driver(registry_type const * const registry)
  noexcept
    : m_begin{registry->core_type::begin()}
    , m_end  {registry->core_type::end  ()}
  { m_initialize(registry, guaranteed_t<expression_type>{}); }

  constexpr
  ~registry_query_driver()
  = default;

  constexpr
  registry_query_driver &
  operator=(registry_query_driver const &)
  = default;

  constexpr
  registry_query_driver &
  operator=(registry_query_driver &&)
  = default;

  constexpr
  void
  swap(registry_query_driver &other)
  noexcept
  {
    using std::swap;

    swap(m_begin, other.m_begin);
    swap(m_end  , other.m_end);
  }

  friend constexpr
  void
  swap(registry_query_driver &lhs, registry_query_driver &rhs)
  noexcept
  { lhs.swap(rhs); }

  [[nodiscard]] friend constexpr
  bool
  operator==(registry_query_driver const &, registry_query_driver const &)
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
    while (iterator != m_end && !registry->template is_match<expression_type>(*iterator))
      ++iterator;

    return iterator;
  }

  constexpr
  iterator_type
  decrement(registry_type const * const registry, iterator_type iterator) const
  noexcept
  {
    while (iterator != m_begin && !registry->template is_match<expression_type>(*iterator))
      --iterator;

    return iterator;
  }
};

template<
    typename ...Expressions,
    typename    Registry>
requires (
    (expression<Expressions> && ...)
 && specialization_of_registry<std::remove_cvref_t<Registry>>)
class registry_query_driver<
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
  registry_query_driver()
  noexcept
    : m_begin{}
  { }

  constexpr
  registry_query_driver(registry_query_driver const &)
  = default;

  constexpr
  registry_query_driver(registry_query_driver &&)
  = default;

  explicit constexpr
  registry_query_driver(registry_type const * const registry)
  noexcept
    : m_begin{increment(registry, registry->core_type::begin())}
  { }

  constexpr
  ~registry_query_driver()
  = default;

  constexpr
  registry_query_driver &
  operator=(registry_query_driver const &)
  = default;

  constexpr
  registry_query_driver &
  operator=(registry_query_driver &&)
  = default;

  [[nodiscard]] friend constexpr
  bool
  operator==(registry_query_driver const &, registry_query_driver const &)
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
    iterator_type const last = end(registry);

    while (iterator != last && !registry->template is_match<expression_type>(*iterator))
      ++iterator;

    return iterator;
  }

  constexpr
  iterator_type
  decrement(registry_type const * const registry, iterator_type iterator) const
  noexcept
  {
    while (iterator != m_begin && !registry->template is_match<expression_type>(*iterator))
      --iterator;

    return iterator;
  }
};

template<
    typename Expression,
    typename Registry>
requires (
    expression<Expression>
 && specialization_of_registry<std::remove_cvref_t<Registry>>)
class registry_query_driver<
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
  registry_query_driver()
  noexcept
    : m_begin{}
  { }

  constexpr
  registry_query_driver(registry_query_driver const &)
  = default;

  constexpr
  registry_query_driver(registry_query_driver &&)
  = default;

  explicit constexpr
  registry_query_driver(registry_type const * const registry)
  noexcept
    : m_begin{increment(registry, registry->core_type::begin())}
  { }

  constexpr
  ~registry_query_driver()
  = default;

  constexpr
  registry_query_driver &
  operator=(registry_query_driver const &)
  = default;

  constexpr
  registry_query_driver &
  operator=(registry_query_driver &&)
  = default;

  [[nodiscard]] friend constexpr
  bool
  operator==(registry_query_driver const &, registry_query_driver const &)
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
    iterator_type const last = end(registry);

    while (iterator != last && registry->template is_match<Expression>(*iterator))
      ++iterator;

    return iterator;
  }

  constexpr
  iterator_type
  decrement(registry_type const * const registry, iterator_type iterator) const
  noexcept
  {
    while (iterator != m_begin && registry->template is_match<Expression>(*iterator))
      --iterator;

    return iterator;
  }
};


template<typename>
struct is_specialization_of_registry_query_driver
  : std::false_type
{ };

template<
    typename Expression,
    typename Registry>
struct is_specialization_of_registry_query_driver<
    registry_query_driver<Expression, Registry>>
  : std::true_type
{ };

template<typename T>
inline constexpr
bool
is_specialization_of_registry_query_driver_v
= is_specialization_of_registry_query_driver<T>::value;

template<typename T>
concept specialization_of_registry_query_driver
= is_specialization_of_registry_query_driver_v<T>;


template<
    typename Driver,
    typename Registry>
class registry_query_iterator
{ };

template<
    typename Driver,
    typename Registry>
requires (
    specialization_of_registry_query_driver<std::remove_cvref_t<Driver>>
 && std::same_as<typename Driver::registry_type, std::remove_const_t<Registry>>)
class registry_query_iterator<
    Driver,
    Registry>
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
  registry_query_iterator()
  noexcept
    : m_driver  {}
    , m_registry{nullptr}
    , m_iterator{}
  { }

  constexpr
  registry_query_iterator(registry_query_iterator const &)
  = default;

  constexpr
  registry_query_iterator(registry_query_iterator &&)
  = default;

  constexpr
  registry_query_iterator(
      driver_type              const driver,
      registry_type *          const registry,
      std::bool_constant<true> const)
  noexcept
    : m_driver  {driver}
    , m_registry{registry}
    , m_iterator{m_driver.begin(m_registry)}
  { }

  constexpr
  registry_query_iterator(
      driver_type               const driver,
      registry_type *           const registry,
      std::bool_constant<false> const)
  noexcept
    : m_driver  {driver}
    , m_registry{registry}
    , m_iterator{m_driver.end(m_registry)}
  { }

  constexpr
  ~registry_query_iterator()
  = default;

  constexpr
  registry_query_iterator &
  operator=(registry_query_iterator const &)
  = default;

  constexpr
  registry_query_iterator &
  operator=(registry_query_iterator &&)
  = default;

  constexpr
  void
  swap(registry_query_iterator &other)
  noexcept
  {
    using std::swap;

    swap(m_registry, other.m_registry);
    swap(m_iterator, other.m_iterator);
    swap(m_driver  , other.m_driver);
  }

  friend constexpr
  void
  swap(registry_query_iterator &lhs, registry_query_iterator &rhs)
  noexcept
  { lhs.swap(rhs); }

  [[nodiscard]] friend constexpr
  bool
  operator==(registry_query_iterator const &lhs, registry_query_iterator const &rhs)
  = default;


  [[nodiscard]] constexpr
  reference
  operator*() const
  noexcept
  { return reference(*m_registry, *m_iterator); }


  constexpr
  registry_query_iterator &
  operator++()
  noexcept
  {
    m_iterator = m_driver.increment(m_registry, ++m_iterator);
    return *this;
  }

  constexpr
  registry_query_iterator &
  operator--()
  noexcept
  {
    m_iterator = m_driver.decrement(m_registry, --m_iterator);
    return *this;
  }

  constexpr
  registry_query_iterator
  operator++(int)
  noexcept
  {
    registry_query_iterator tmp(*this);
    ++*this;
    return tmp;
  }

  constexpr
  registry_query_iterator
  operator--(int)
  noexcept
  {
    registry_query_iterator tmp(*this);
    --*this;
    return tmp;
  }
};


template<
    typename Expression,
    typename Registry>
class registry_query
  : public std::ranges::view_interface<registry_query<Expression, Registry>>
{
public:
  using expression_type = Expression;
  using registry_type   = Registry;

private:
  using driver_type
  = registry_query_driver<expression_type, registry_type>;

public:
  using iterator       = registry_query_iterator<driver_type, registry_type>;
  using const_iterator = registry_query_iterator<driver_type, registry_type const>;

private:
  registry_type *m_registry;
  driver_type    m_driver;

public:
  constexpr
  registry_query()
  noexcept
    : m_registry{nullptr}
    , m_driver  {}
  { }

  constexpr
  registry_query(registry_query const &)
  = default;

  constexpr
  registry_query(registry_query &&)
  = default;

  explicit constexpr
  registry_query(registry_type &registry)
  noexcept
    : m_registry{&registry}
    , m_driver  {m_registry}
  { }

  constexpr
  ~registry_query()
  = default;

  constexpr
  registry_query &
  operator=(registry_query const &)
  = default;

  constexpr
  registry_query &
  operator=(registry_query &&)
  = default;

  constexpr
  void
  swap(registry_query &other)
  noexcept
  {
    using std::swap;

    swap(m_registry, other.m_registry);
    swap(m_driver  , other.m_driver);
  }

  friend constexpr
  void
  swap(registry_query &lhs, registry_query &rhs)
  noexcept
  { lhs.swap(rhs); }

  [[nodiscard]] friend constexpr
  bool
  operator==(registry_query const &, registry_query const &)
  = default;


  [[nodiscard]] constexpr
  iterator
  begin()
  noexcept
  { return iterator(m_driver, m_registry, std::bool_constant<true>{}); }

  [[nodiscard]] constexpr
  const_iterator
  begin() const
  noexcept
  { return const_iterator(m_driver, m_registry, std::bool_constant<true>{}); }

  [[nodiscard]] constexpr
  iterator
  end()
  noexcept
  { return iterator(m_driver, m_registry, std::bool_constant<false>{}); }

  [[nodiscard]] constexpr
  const_iterator
  end() const
  noexcept
  { return const_iterator(m_driver, m_registry, std::bool_constant<false>{}); }

  [[nodiscard]] constexpr
  const_iterator
  cbegin() const
  noexcept
  { return const_iterator(m_driver, m_registry, std::bool_constant<true>{}); }

  [[nodiscard]] constexpr
  const_iterator
  cend() const
  noexcept
  { return const_iterator(m_driver, m_registry, std::bool_constant<false>{}); }
};

} // namespace detail


template<
    typename Identifier,
    typename Allocator,
    typename DescSequence>
class registry
  : protected detail::registry_core   <Identifier, Allocator>
  , protected detail::registry_storage<Identifier, Allocator, DescSequence>
{
  using core_type    = detail::registry_core   <Identifier, Allocator>;
  using storage_type = detail::registry_storage<Identifier, Allocator, DescSequence>;

  template<typename>           friend class detail::registry_iterator;
  template<typename, typename> friend class detail::registry_query_driver;
  template<typename, typename> friend class detail::registry_query_iterator;

public:
  using identifier_type      = Identifier;
  using allocator_type       = Allocator;
  using description_sequence = DescSequence;

  using entity_type       = entity<registry>;
  using const_entity_type = entity<registry const>;

  using iterator       = detail::registry_iterator<registry>;
  using const_iterator = detail::registry_iterator<registry const>;

  template<typename Expression> using query_for       = detail::registry_query<Expression, registry>;
  template<typename Expression> using const_query_for = detail::registry_query<Expression, registry const>;


  template<
      typename    Component,
      std::size_t PageSize  = default_page_size_v<>>
  using with
  = registry<
      identifier_type,
      allocator_type,
      type_sequence_append_t<
          description_sequence,
          detail::registry_descriptor<Component, PageSize>>>;

  template<typename ...Components>
  using with_all
  = registry<
      identifier_type,
      allocator_type,
      type_sequence_append_t<
          description_sequence,
          detail::registry_descriptor<Components> ...>>;

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
            core_type    &&, allocator_type const &>
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
  registry(allocator_type const &alloc)
  noexcept
    : core_type   {alloc}
    , storage_type{alloc}
  { }

  constexpr
  registry()
  noexcept(s_noexcept_default_construct())
    : registry{allocator_type{}}
  { }

  constexpr
  registry(registry const &other, allocator_type const &alloc)
    : core_type   {static_cast<core_type    const &>(other), alloc}
    , storage_type{static_cast<storage_type const &>(other), alloc}
  { }

  constexpr
  registry(registry const &)
  = default;

  constexpr
  registry(registry &&other, allocator_type const &alloc)
  noexcept(s_noexcept_move_alloc_construct())
    : core_type   {static_cast<core_type    &&>(other), alloc}
    , storage_type{static_cast<storage_type &&>(other), alloc}
  { }

  constexpr
  registry(registry &&)
  = default;

  constexpr
  ~registry()
  = default;

  constexpr
  registry &
  operator=(registry const &)
  = default;

  constexpr
  registry &
  operator=(registry &&)
  = default;

  constexpr
  void
  swap(registry &other)
  noexcept(s_noexcept_swap())
  {
    core_type   ::swap(static_cast<core_type    &>(other));
    storage_type::swap(static_cast<storage_type &>(other));
  }

  friend constexpr
  void
  swap(registry &lhs, registry &rhs)
  noexcept(s_noexcept_swap())
  { lhs.swap(rhs); }

  [[nodiscard]] friend constexpr
  bool
  operator==(registry const &, registry const &)
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
  { return iterator(*this, core_type::begin()); }

  [[nodiscard]] constexpr
  const_iterator
  begin() const
  noexcept
  { return const_iterator(*this, core_type::begin()); }

  [[nodiscard]] constexpr
  iterator
  end()
  noexcept
  { return iterator(*this, core_type::end()); }

  [[nodiscard]] constexpr
  const_iterator
  end() const
  noexcept
  { return const_iterator(*this, core_type::end()); }


  [[nodiscard]] constexpr
  const_iterator
  cbegin() const
  noexcept
  { return const_iterator(*this, core_type::cbegin()); }

  [[nodiscard]] constexpr
  const_iterator
  cend() const
  noexcept
  { return const_iterator(*this, core_type::cend()); }

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
  is_valid(identifier_type const id) const
  noexcept
  { return core_type::is_valid(id); }

  [[nodiscard]] constexpr
  bool
  is_valid(entity_type const e) const
  noexcept
  { return is_valid(e.identifier()); }

  [[nodiscard]] constexpr
  bool
  is_valid(const_entity_type const ce) const
  noexcept
  { return is_valid(ce.identifier()); }

  template<typename Expression>
  [[nodiscard]] constexpr
  bool
  is_match(identifier_type const id, Expression const = Expression{}) const
  noexcept
  { return storage_type::template is_match<Expression>(id); }

  template<typename Expression>
  [[nodiscard]] constexpr
  bool
  is_match(entity_type const e, Expression const = Expression{}) const
  noexcept
  { return is_match<Expression>(e.identifier()); }

  template<typename Expression>
  [[nodiscard]] constexpr
  bool
  is_match(const_entity_type const ce, Expression const = Expression{}) const
  noexcept
  { return is_match<Expression>(ce.identifier()); }

  template<typename Expression>
  [[nodiscard]] constexpr
  query_for<Expression>
  query()
  noexcept
  { return query_for<Expression>(*this); }

  template<typename Expression>
  [[nodiscard]] constexpr
  const_query_for<Expression>
  query() const
  noexcept
  { return const_query_for<Expression>(*this); }


  template<typename Component>
  [[nodiscard]] constexpr
  Component &
  get(identifier_type const id)
  noexcept
  { return storage_type::template get<Component>(id); }

  template<typename Component>
  [[nodiscard]] constexpr
  Component &
  get(entity_type const e)
  noexcept
  { return get<Component>(e.identifier()); }

  template<typename Component>
  [[nodiscard]] constexpr
  Component &
  get(const_entity_type const ce)
  noexcept
  { return get<Component>(ce.identifier()); }

  template<typename Component>
  [[nodiscard]] constexpr
  Component const &
  get(identifier_type const id) const
  noexcept
  { return storage_type::template get<Component>(id); }

  template<typename Component>
  [[nodiscard]] constexpr
  Component const &
  get(entity_type const e) const
  noexcept
  { return get<Component>(e.identifier()); }

  template<typename Component>
  [[nodiscard]] constexpr
  Component const &
  get(const_entity_type const ce) const
  noexcept
  { return get<Component>(ce.identifier()); }

  template<typename Component>
  [[nodiscard]] constexpr
  Component &
  try_get(identifier_type const id)
  { return storage_type::template try_get<Component>(id); }

  template<typename Component>
  [[nodiscard]] constexpr
  Component &
  try_get(entity_type const e)
  { return try_get<Component>(e.identifier()); }

  template<typename Component>
  [[nodiscard]] constexpr
  Component &
  try_get(const_entity_type const ce)
  { return try_get<Component>(ce.identifier()); }

  template<typename Component>
  [[nodiscard]] constexpr
  Component const &
  try_get(identifier_type const id) const
  { return storage_type::template try_get<Component>(id); }

  template<typename Component>
  [[nodiscard]] constexpr
  Component const &
  try_get(entity_type const e) const
  { return try_get<Component>(e.identifier()); }

  template<typename Component>
  [[nodiscard]] constexpr
  Component const &
  try_get(const_entity_type const ce) const
  { return try_get<Component>(ce.identifier()); }

  template<typename Component>
  [[nodiscard]] constexpr
  Component *
  get_if(identifier_type const id)
  noexcept
  { return storage_type::template get_if<Component>(id); }

  template<typename Component>
  [[nodiscard]] constexpr
  Component *
  get_if(entity_type const e)
  noexcept
  { return get_if<Component>(e.identifier()); }

  template<typename Component>
  [[nodiscard]] constexpr
  Component *
  get_if(const_entity_type const ce)
  noexcept
  { return get_if<Component>(ce.identifier()); }

  template<typename Component>
  [[nodiscard]] constexpr
  Component const *
  get_if(identifier_type const id) const
  noexcept
  { return storage_type::template get_if<Component>(id); }

  template<typename Component>
  [[nodiscard]] constexpr
  Component const *
  get_if(entity_type const e) const
  noexcept
  { return get_if<Component>(e.identifier()); }

  template<typename Component>
  [[nodiscard]] constexpr
  Component const *
  get_if(const_entity_type const ce) const
  noexcept
  { return get_if<Component>(ce.identifier()); }


  [[nodiscard]] constexpr
  identifier_type
  create()
  { return core_type::create(); }

  template<typename Component, typename ...Args>
  constexpr
  void
  emplace(identifier_type const id, Args &&...args)
  { storage_type::template emplace<Component>(id, std::forward<Args>(args)...); }

  template<typename Component, typename ...Args>
  constexpr
  void
  emplace(entity_type const e, Args &&...args)
  { emplace<Component>(e.identifier(), std::forward<Args>(args)...); }

  template<typename Component, typename ...Args>
  constexpr
  void
  emplace(const_entity_type const ce, Args &&...args)
  { emplace<Component>(ce.identifier(), std::forward<Args>(args)...); }

  template<typename Component, typename ...Args>
  constexpr
  bool
  try_emplace(identifier_type const id, Args &&...args)
  { return storage_type::template try_emplace<Component>(id, std::forward<Args>(args)...); }

  template<typename Component, typename ...Args>
  constexpr
  bool
  try_emplace(entity_type const e, Args &&...args)
  { return try_emplace<Component>(e.identifier(), std::forward<Args>(args)...); }

  template<typename Component, typename ...Args>
  constexpr
  bool
  try_emplace(const_entity_type const ce, Args &&...args)
  { return try_emplace<Component>(ce.identifier(), std::forward<Args>(args)...); }

  template<typename Component>
  constexpr
  bool
  insert(identifier_type const id, Component &&c)
  { return storage_type::template insert<Component>(id, std::forward<Component>(c)); }

  template<typename Component>
  constexpr
  bool
  insert(entity_type const e, Component &&c)
  { return insert(e.identifier(), std::forward<Component>(c)); }

  template<typename Component>
  constexpr
  bool
  insert(const_entity_type const ce, Component &&c)
  { return insert(ce.identifier(), std::forward<Component>(c)); }

  template<typename Component>
  constexpr
  bool
  insert_or_assign(identifier_type const id, Component &&c)
  { return storage_type::template insert_or_assign<Component>(id, std::forward<Component>(c)); }

  template<typename Component>
  constexpr
  bool
  insert_or_assign(entity_type const e, Component &&c)
  { return insert_or_assign(e.identifier(), std::forward<Component>(c)); }

  template<typename Component>
  constexpr
  bool
  insert_or_assign(const_entity_type const ce, Component &&c)
  { return insert_or_assign(ce.identifier(), std::forward<Component>(c)); }

  template<typename Component>
  constexpr
  void
  erase(identifier_type const id)
  { storage_type::template erase<Component>(id); }

  template<typename Component>
  constexpr
  void
  erase(entity_type const e)
  { erase<Component>(e.identifier()); }

  template<typename Component>
  constexpr
  void
  erase(const_entity_type const ce)
  { erase<Component>(ce.identifier()); }

  template<typename Component>
  constexpr
  bool
  try_erase(identifier_type const id)
  { return storage_type::template try_erase<Component>(id); }

  template<typename Component>
  constexpr
  bool
  try_erase(entity_type const e)
  { return try_erase<Component>(e.identifier()); }

  template<typename Component>
  constexpr
  bool
  try_erase(const_entity_type const ce)
  { return try_erase<Component>(ce.identifier()); }

  constexpr
  void
  clear(identifier_type const id)
  { storage_type::clear(id); }

  constexpr
  void
  clear(entity_type const e)
  { clear(e.identifier()); }

  constexpr
  void
  clear(const_entity_type const ce)
  { clear(ce.identifier()); }

  constexpr
  void
  clear()
  noexcept
  { storage_type::clear(); core_type::clear(); }

  constexpr
  bool
  destroy(identifier_type const id)
  {
    if (!is_valid(id))
      return false;

    clear(id);
    core_type::destroy(id);
    return true;
  }

  constexpr
  void
  destroy(entity_type const e)
  { destroy(e.identifier()); }

  constexpr
  void
  destroy(const_entity_type const ce)
  { destroy(ce.identifier()); }
};

} // namespace sparse


template<
    typename Identifier,
    typename Allocator,
    typename DescSequence>
struct is_registry<
    sparse::registry<Identifier, Allocator, DescSequence>>
  : std::true_type
{ };

} // namespace heim


#endif // HEIM_ECS_SPARSE_REGISTRY_HPP
