#ifndef HEIM_ECS_REGISTRY_SPARSE_REGISTRY_HPP
#define HEIM_ECS_REGISTRY_SPARSE_REGISTRY_HPP

#include <cstddef>
#include <memory>
#include <ranges>
#include <type_traits>
#include <utility>
#include "heim/ecs/expression.hpp"
#include "heim/ecs/identifier.hpp"
#include "manager.hpp"

namespace heim::ecs::sparse
{
template<
    typename Id,
    typename Alloc,
    typename Storage>
class generic_registry
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
    if      constexpr (specialization_of_conjunction<Expr>)
      return s_noexcept_matches_conjunction(Expr{});
    else if constexpr (specialization_of_disjunction<Expr>)
      return s_noexcept_matches_disjunction(Expr{});
    else if constexpr (specialization_of_negation   <Expr>)
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
  generic_registry(allocator_type const &alloc)
  noexcept
    : m_manager{alloc}
    , m_storage{alloc}
  { }

  constexpr
  generic_registry(generic_registry const &other, allocator_type const &alloc)
    : m_manager{other.m_manager, alloc}
    , m_storage{other.m_storage, alloc}
  { }

  constexpr
  generic_registry(generic_registry &&other, allocator_type const &alloc)
  noexcept(s_noexcept_move_alloc_construct())
    : m_manager{std::move(other.m_manager), alloc}
    , m_storage{std::move(other.m_storage), alloc}
  { }

  constexpr
  generic_registry()
  noexcept(s_noexcept_default_construct())
    : generic_registry{allocator_type{}}
  { }

  constexpr
  generic_registry(generic_registry const &)
  = default;

  constexpr
  generic_registry(generic_registry &&)
  = default;

  constexpr
  ~generic_registry()
  = default;

  constexpr
  generic_registry &
  operator=(generic_registry const &)
  = default;

  constexpr
  generic_registry &
  operator=(generic_registry &&)
  = default;

  constexpr
  void
  swap(generic_registry &other)
  noexcept(s_noexcept_swap())
  {
    using std::swap;

    swap(m_manager, other.m_manager);
    swap(m_storage, other.m_storage);
  }

  friend constexpr
  void
  swap(generic_registry &lhs, generic_registry &rhs)
  noexcept(s_noexcept_swap())
  { lhs.swap(rhs); }

  [[nodiscard]] constexpr
  allocator_type
  get_allocator() const
  noexcept
  { return m_manager.get_allocator(); }


  [[nodiscard]] friend constexpr
  bool
  operator==(generic_registry const &, generic_registry const &)
  = default;


  [[nodiscard]] constexpr auto begin() noexcept       { return m_manager.begin(); }
  [[nodiscard]] constexpr auto begin() const noexcept { return m_manager.begin(); }

  [[nodiscard]] constexpr auto end() noexcept       { return m_manager.end(); }
  [[nodiscard]] constexpr auto end() const noexcept { return m_manager.end(); }

  [[nodiscard]] constexpr auto cbegin() const noexcept { return begin(); }
  [[nodiscard]] constexpr auto cend  () const noexcept { return end  (); }

  [[nodiscard]] constexpr auto size () const noexcept { return m_manager.size (); }
  [[nodiscard]] constexpr bool empty() const noexcept { return m_manager.empty(); }


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
    if      constexpr (specialization_of_conjunction<Expr>)
      return m_matches_conjunction(id, Expr{});
    else if constexpr (specialization_of_disjunction<Expr>)
      return m_matches_disjunction(id, Expr{});
    else if constexpr (specialization_of_negation   <Expr>)
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
    if      constexpr (specialization_of_conjunction<Expr>)
      return m_matches_conjunction(id, Expr{});
    else if constexpr (specialization_of_disjunction<Expr>)
      return m_matches_disjunction(id, Expr{});
    else if constexpr (specialization_of_negation   <Expr>)
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

} // namespace heim::ecs::sparse

#endif // HEIM_ECS_REGISTRY_SPARSE_REGISTRY_HPP
