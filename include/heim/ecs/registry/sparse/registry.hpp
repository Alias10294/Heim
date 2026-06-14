#ifndef HEIM_ECS_REGISTRY_SPARSE_ECS_REGISTRY_HPP
#define HEIM_ECS_REGISTRY_SPARSE_ECS_REGISTRY_HPP

#include <atomic>
#include <cstddef>
#include <memory>
#include <ranges>
#include <type_traits>
#include <utility>
#include "heim/ecs/expression.hpp"
#include "heim/ecs/identifier.hpp"
#include "detail/pool.hpp"
#include "detail/set.hpp"
#include "manager.hpp"

namespace heim::ecs::sparse
{
template<
    typename    Id,
    typename    Alloc  = std::allocator<Id>,
    std::size_t PageSz = 1024>
class auto_storage
{
public:
  using identifier_type = Id;
  using allocator_type  = Alloc;

  static constexpr std::size_t page_size
  = PageSz;

private:
  using alloc_traits
  = std::allocator_traits<Alloc>;

  using set_type      = heim::sparse::set<Id, PageSz, Alloc>;
  using set_allocator = typename alloc_traits::template rebind_alloc<set_type>;

  using set_pointer           = std::shared_ptr<set_type>;
  using set_pointer_allocator = typename alloc_traits::template rebind_alloc<set_pointer>;
  using set_pointer_container = std::vector<set_pointer, set_pointer_allocator>;

  template<typename C>
  using pool_for_type
  = heim::sparse::pool<C, Id, PageSz, Alloc>;

private:
  set_pointer_container m_container;

private:
  static constexpr
  bool
  s_noexcept_move_alloc_construct()
  noexcept
  {
    return std::is_nothrow_constructible_v<
        set_pointer_container,
        set_pointer_container, set_pointer_allocator>;
  }

  static constexpr
  bool
  s_noexcept_default_construct()
  noexcept
  { return std::is_nothrow_constructible_v<allocator_type>; }

  static constexpr
  bool
  s_noexcept_swap()
  noexcept
  { return std::is_nothrow_swappable_v<set_pointer_container>; }


  static constexpr
  std::size_t
  s_next_index()
  noexcept
  {
    static std::atomic_size_t counter{0};

    return counter.fetch_add(std::size_t{1}, std::memory_order_relaxed);
  }

  template<typename C>
  requires (!qualified<C>)
  static constexpr
  std::size_t
  s_index()
  noexcept
  {
    static std::size_t const idx{s_next_index()};

    return idx;
  }

public:
  explicit constexpr
  auto_storage(allocator_type const alloc)
  noexcept
    : m_container{alloc}
  { }

  constexpr
  auto_storage(auto_storage const &other, allocator_type const alloc)
    : m_container{other.m_container, alloc}
  { }

  constexpr
  auto_storage(auto_storage &&other, allocator_type const alloc)
  noexcept(s_noexcept_move_alloc_construct())
    : m_container{std::move(other.m_container), alloc}
  { }

  constexpr
  auto_storage()
  noexcept(s_noexcept_default_construct())
    : auto_storage{allocator_type{}}
  { }

  constexpr
  auto_storage(auto_storage const &)
  = default;

  constexpr
  auto_storage(auto_storage &&)
  = default;

  constexpr
  ~auto_storage()
  = default;

  constexpr
  auto_storage &
  operator=(auto_storage const &)
  = default;

  constexpr
  auto_storage &
  operator=(auto_storage &&)
  = default;

  constexpr
  void
  swap(auto_storage &other)
  noexcept(s_noexcept_swap())
  {
    using std::swap;

    swap(m_container, other.m_container);
  }

  friend constexpr
  void
  swap(auto_storage &lhs, auto_storage &rhs)
  noexcept(s_noexcept_swap())
  { lhs.swap(rhs); }

  [[nodiscard]] constexpr
  allocator_type
  get_allocator() const
  noexcept
  { return allocator_type{m_container.get_allocator()}; }

  [[nodiscard]] friend constexpr
  bool
  operator==(auto_storage const &, auto_storage const &)
  = default;


  template<typename C>
  requires (!qualified<C>)
  [[nodiscard]] constexpr
  pool_for_type<C> &
  assure()
  {
    std::size_t const idx{s_index<C>()};

    if (m_container.size() <= idx) [[unlikely]]
      m_container.resize(idx + 1);

    if (allocator_type alloc{get_allocator()}; !m_container[idx]) [[unlikely]]
      m_container[idx] = std::allocate_shared<pool_for_type<C>>(alloc, alloc);

    return static_cast<pool_for_type<C> &>(*m_container[idx]);
  }

  template<typename C>
  requires (!qualified<C>)
  [[nodiscard]] constexpr
  pool_for_type<C> &
  pool()
  noexcept
  { return static_cast<pool_for_type<C> &>(*m_container[s_index<C>()]); }

  template<typename C>
  requires (!qualified<C>)
  [[nodiscard]] constexpr
  pool_for_type<C> const &
  pool() const
  noexcept
  { return static_cast<pool_for_type<C> &>(*m_container[s_index<C>()]); }


  constexpr
  void
  clear(identifier_type const id)
  { for (auto &pool : m_container) pool->try_erase(id); }

  constexpr
  void
  clear()
  { for (auto &pool : m_container) pool->clear(); }
};


template<
    typename Id      = default_identifier_t<>,
    typename Alloc   = std::allocator<Id>,
    typename Storage = auto_storage<Id, Alloc>>
class registry
{
public:
  using identifier_type = Id;
  using allocator_type  = Alloc;
  using storage_type    = Storage;

private:
  using manager_type
  = manager<Id, Alloc>;

private:
  manager_type m_manager;
  storage_type m_storage;

private:
  static constexpr
  bool
  s_noexcept_move_alloc_construct()
  noexcept
  {
    return std::is_nothrow_constructible_v<manager_type, manager_type, allocator_type>
        && std::is_nothrow_constructible_v<storage_type, storage_type, allocator_type>;
  }

  static constexpr
  bool
  s_noexcept_default_construct()
  noexcept
  { return std::is_nothrow_constructible_v<allocator_type>; }

  static constexpr
  bool
  s_noexcept_swap()
  noexcept
  {
    return std::is_nothrow_swappable_v<manager_type>
        && std::is_nothrow_swappable_v<storage_type>;
  }

  template<typename Expr>
  requires (!qualified<Expr>)
  static constexpr
  bool
  s_noexcept_matches()
  noexcept
  {
    if      constexpr (is_specialization_of_conjunction_v<Expr>)
      return s_noexcept_matches_conjunction(Expr{});
    else if constexpr (is_specialization_of_disjunction_v<Expr>)
      return s_noexcept_matches_disjunction(Expr{});
    else if constexpr (is_specialization_of_negation_v   <Expr>)
      return s_noexcept_matches_negation   (Expr{});
    else
      return noexcept(std::declval<storage_type &>().template assure<Expr>());
  }

  template<typename ...Expr>
  requires (!qualified<Expr> && ...)
  static constexpr
  bool
  s_noexcept_matches_conjunction(conjunction<Expr ...> const)
  noexcept
  { return (s_noexcept_matches<Expr>() && ...); }

  template<typename ...Expr>
  requires (!qualified<Expr> && ...)
  static constexpr
  bool
  s_noexcept_matches_disjunction(disjunction<Expr ...> const)
  noexcept
  { return (s_noexcept_matches<Expr>() && ...); }

  template<typename Expr>
  requires (!qualified<Expr>)
  static constexpr
  bool
  s_noexcept_matches_negation(negation<Expr> const)
  noexcept
  { return s_noexcept_matches<Expr>(); }


  template<typename ...Expr>
  requires ((!qualified<Expr>) && ...)
  [[nodiscard]] constexpr
  bool
  m_matches_conjunction(identifier_type const id, conjunction<Expr ...> const)
  noexcept((noexcept(matches<Expr>(id)) && ...))
  { return (matches<Expr>(id) && ...); }

  template<typename ...Expr>
  requires ((!qualified<Expr>) && ...)
  [[nodiscard]] constexpr
  bool
  m_matches_disjunction(identifier_type const id, disjunction<Expr ...> const)
  noexcept((noexcept(matches<Expr>(id)) && ...))
  { return (matches<Expr>(id) || ...); }

  template<typename Expr>
  requires (!qualified<Expr>)
  [[nodiscard]] constexpr
  bool
  m_matches_negation(identifier_type const id, negation<Expr> const)
  noexcept(noexcept(matches<Expr>(id)))
  { return !matches<Expr>(id); }


  template<typename ...Expr>
  requires ((!qualified<Expr>) && ...)
  [[nodiscard]] constexpr
  bool
  m_matches_conjunction(identifier_type const id, conjunction<Expr ...> const) const
  noexcept
  { return (matches<Expr>(id) && ...); }

  template<typename ...Expr>
  requires ((!qualified<Expr>) && ...)
  [[nodiscard]] constexpr
  bool
  m_matches_disjunction(identifier_type const id, disjunction<Expr ...> const) const
  noexcept
  { return (matches<Expr>(id) || ...); }

  template<typename Expr>
  requires (!qualified<Expr>)
  [[nodiscard]] constexpr
  bool
  m_matches_negation(identifier_type const id, negation<Expr> const) const
  noexcept
  { return !matches<Expr>(id); }

public:
  explicit constexpr
  registry(allocator_type const alloc)
  noexcept
    : m_manager{alloc}
    , m_storage{alloc}
  { }

  constexpr
  registry(registry const &other, allocator_type const alloc)
    : m_manager{other.m_manager, alloc}
    , m_storage{other.m_storage, alloc}
  { }

  constexpr
  registry(registry &&other, allocator_type const alloc)
  noexcept(s_noexcept_move_alloc_construct())
    : m_manager{std::move(other.m_manager), alloc}
    , m_storage{std::move(other.m_storage), alloc}
  { }

  constexpr
  registry()
  noexcept(s_noexcept_default_construct())
    : registry{allocator_type{}}
  { }

  constexpr
  registry(registry const &)
  = default;

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
    using std::swap;

    swap(m_manager, other.m_manager);
    swap(m_storage, other.m_storage);
  }

  friend constexpr
  void
  swap(registry &lhs, registry &rhs)
  noexcept(s_noexcept_swap())
  { lhs.swap(rhs); }

  [[nodiscard]] constexpr
  allocator_type
  get_allocator() const
  noexcept
  { return m_manager.get_allocator(); }


  [[nodiscard]] friend constexpr
  bool
  operator==(registry const &, registry const &)
  = default;


  [[nodiscard]] constexpr auto begin() noexcept       { return std::ranges::begin (m_manager); }
  [[nodiscard]] constexpr auto begin() const noexcept { return std::ranges::cbegin(m_manager); }

  [[nodiscard]] constexpr auto end() noexcept       { return std::ranges::end (m_manager); }
  [[nodiscard]] constexpr auto end() const noexcept { return std::ranges::cend(m_manager); }

  [[nodiscard]] constexpr auto cbegin() const noexcept { return begin(); }
  [[nodiscard]] constexpr auto cend  () const noexcept { return end  (); }

  [[nodiscard]] constexpr auto size () const noexcept { return std::ranges::size (m_manager); }
  [[nodiscard]] constexpr bool empty() const noexcept { return std::ranges::empty(m_manager); }


  template<typename C>
  requires (!qualified<C>)
  [[nodiscard]] constexpr
  auto &
  assure()
  noexcept(noexcept(m_storage.template assure<C>()))
  { return m_storage.template assure<C>(); }

  template<typename C>
  requires (!qualified<C>)
  [[nodiscard]] constexpr
  auto &
  pool()
  noexcept
  { return m_storage.template pool<C>(); }

  template<typename C>
  requires (!qualified<C>)
  [[nodiscard]] constexpr
  auto const &
  pool() const
  noexcept
  { return m_storage.template pool<C>(); }

  template<typename C>
  requires (!qualified<C>)
  [[nodiscard]] constexpr
  C &
  get(identifier_type const id)
  noexcept(noexcept(assure<C>()))
  { return assure<C>()[id]; }

  template<typename C>
  requires (!qualified<C>)
  [[nodiscard]] constexpr
  C const &
  get(identifier_type const id) const
  noexcept
  { return pool<C>()[id]; }

  template<typename C>
  requires (!qualified<C>)
  [[nodiscard]] constexpr
  C *
  get_if(identifier_type const id)
  noexcept(noexcept(assure<C>()))
  {
    if (auto &p{assure<C>()}; p.contains(id))
      return std::addressof(p[id]);
    return nullptr;
  }

  template<typename C>
  requires (!qualified<C>)
  [[nodiscard]] constexpr
  C const *
  get_if(identifier_type const id) const
  noexcept
  {
    if (auto const &p{pool<C>()}; p.contains(id))
      return std::addressof(p[id]);
    return nullptr;
  }

  template<typename Expr>
  requires (!qualified<Expr>)
  [[nodiscard]] constexpr
  bool
  matches(identifier_type const id)
  noexcept(s_noexcept_matches<Expr>())
  {
    if      constexpr (is_specialization_of_conjunction_v<Expr>)
      return m_matches_conjunction(id, Expr{});
    else if constexpr (is_specialization_of_disjunction_v<Expr>)
      return m_matches_disjunction(id, Expr{});
    else if constexpr (is_specialization_of_negation_v   <Expr>)
      return m_matches_negation   (id, Expr{});
    else
      return assure<Expr>().contains(id);
  }

  template<typename Expr>
  requires (!qualified<Expr>)
  [[nodiscard]] constexpr
  bool
  matches(identifier_type const id) const
  noexcept
  {
    if      constexpr (is_specialization_of_conjunction_v<Expr>)
      return m_matches_conjunction(id, Expr{});
    else if constexpr (is_specialization_of_disjunction_v<Expr>)
      return m_matches_disjunction(id, Expr{});
    else if constexpr (is_specialization_of_negation_v   <Expr>)
      return m_matches_negation   (id, Expr{});
    else
      return pool<Expr>().contains(id);
  }


  template<typename C, typename ...Args>
  requires (!qualified<C>)
  constexpr
  C &
  emplace(identifier_type const id, Args &&...args)
  { return assure<C>().emplace(id, std::forward<Args>(args)...); }

  template<typename C, typename ...Args>
  requires (!qualified<C>)
  constexpr
  std::pair<bool, C &>
  try_emplace(identifier_type const id, Args &&...args)
  { return assure<C>().try_emplace(id, std::forward<Args>(args)...); }

  template<typename C>
  requires (!qualified<C>)
  constexpr
  std::pair<bool, C &>
  insert(identifier_type const id, C &&c)
  { return assure<C>().insert(id, std::forward<C>(c)); }

  template<typename C>
  requires (!qualified<C>)
  constexpr
  std::pair<bool, C &>
  insert_or_assign(identifier_type const id, C &&c)
  { return assure<C>().insert_or_assign(id, std::forward<C>(c)); }

  template<typename C>
  requires (!qualified<C>)
  constexpr
  void
  erase(identifier_type const id)
  { return assure<C>().erase(id); }

  template<typename C>
  requires (!qualified<C>)
  constexpr
  bool
  try_erase(identifier_type const id)
  { return assure<C>().try_erase(id); }

  constexpr
  void
  clear(identifier_type const id)
  { return m_storage.clear(id); }

  constexpr
  void
  clear()
  noexcept
  { return m_storage.clear(); }


  constexpr
  bool
  expired(identifier_type const id) const
  noexcept
  { return m_manager.expired(id); }


  [[nodiscard]] constexpr
  identifier_type
  make()
  { return m_manager.make(); }

  constexpr
  void
  destroy(identifier_type const id)
  {
    clear(id);
    m_manager.destroy(id);
  }

  constexpr
  void
  destroy_all()
  noexcept
  {
    clear();
    m_manager.destroy_all();
  }
};


using auto_registry
= registry<>;

} // namespace heim::ecs::sparse

#endif // HEIM_ECS_REGISTRY_SPARSE_ECS_REGISTRY_HPP
