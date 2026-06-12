#ifndef HEIM_ECS_REGISTRY_SPARSE_ECS_REGISTRY_HPP
#define HEIM_ECS_REGISTRY_SPARSE_ECS_REGISTRY_HPP

#include <cstddef>
#include <memory>
#include <ranges>
#include <type_traits>
#include <utility>
#include "manager.hpp"

namespace heim::ecs::sparse
{
template<
    typename Id,
    typename Alloc   = std::allocator<Id>,
    typename Storage = auto_storage<Id, Alloc>>
class registry
{
public:
  using identifier_type = Id;
  using allocator_type  = Alloc;
  using storage_type    = Storage;

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


  [[nodiscard]] constexpr
  storage_type &
  get_storage()
  noexcept
  { return m_storage; }

  [[nodiscard]] constexpr
  storage_type const &
  get_storage() const
  noexcept
  { return m_storage; }


  [[nodiscard]] constexpr auto begin() noexcept       { return std::ranges::begin (m_manager); }
  [[nodiscard]] constexpr auto begin() const noexcept { return std::ranges::cbegin(m_manager); }

  [[nodiscard]] constexpr auto end() noexcept       { return std::ranges::end (m_manager); }
  [[nodiscard]] constexpr auto end() const noexcept { return std::ranges::cend(m_manager); }

  [[nodiscard]] constexpr auto cbegin() const noexcept { return begin(); }
  [[nodiscard]] constexpr auto cend  () const noexcept { return end  (); }

  [[nodiscard]] constexpr auto size () const noexcept { return std::ranges::size (m_manager); }
  [[nodiscard]] constexpr bool empty() const noexcept { return std::ranges::empty(m_manager); }


  template<typename C>
  [[nodiscard]] constexpr
  C &
  get(identifier_type const id)
  noexcept
  { return m_storage.template get<C>(id); }

  template<typename C>
  [[nodiscard]] constexpr
  C const &
  get(identifier_type const id) const
  noexcept
  { return m_storage.template get<C>(id); }

  template<typename C>
  [[nodiscard]] constexpr
  C *
  get_if(identifier_type const id)
  noexcept
  { return m_storage.template get_if<C>(id); }

  template<typename C>
  [[nodiscard]] constexpr
  C const *
  get_if(identifier_type const id) const
  noexcept
  { return m_storage.template get_if<C>(id); }

  template<typename Expr>
  [[nodiscard]] constexpr
  bool
  matches(identifier_type const id) const
  noexcept
  { return m_storage.template matches<Expr>(id); }


  template<typename C, typename ...Args>
  constexpr
  C &
  emplace(identifier_type const id, Args &&...args)
  { return m_storage.template emplace<C>(id, std::forward<Args>(args)...); }

  template<typename C, typename ...Args>
  constexpr
  std::pair<bool, C &>
  try_emplace(identifier_type const id, Args &&...args)
  { return m_storage.template try_emplace<C>(id, std::forward<Args>(args)...); }

  template<typename C>
  constexpr
  std::pair<bool, C &>
  insert(identifier_type const id, C &&c)
  { return m_storage.template insert<C>(id, std::forward<C>(c)); }

  template<typename C>
  constexpr
  std::pair<bool, C &>
  insert_or_assign(identifier_type const id, C &&c)
  { return m_storage.template insert_or_assign<C>(id, std::forward<C>(c)); }

  template<typename C>
  constexpr
  void
  erase(identifier_type const id)
  { return m_storage.template erase<C>(id); }

  template<typename C>
  constexpr
  bool
  try_erase(identifier_type const id)
  { return m_storage.template try_erase<C>(id); }

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
  noexcept
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

#endif // HEIM_ECS_REGISTRY_SPARSE_ECS_REGISTRY_HPP
