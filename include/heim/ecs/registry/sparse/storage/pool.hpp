#ifndef HEIM_ECS_REGISTRY_SPARSE_POOL_HPP
#define HEIM_ECS_REGISTRY_SPARSE_POOL_HPP

#include <algorithm>
#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>
#include "set.hpp"

namespace heim::ecs::sparse
{
template<
    typename    C,
    typename    Id,
    std::size_t PageSz,
    typename    Alloc>
class generic_pool
  : public generic_set<Id, PageSz, Alloc>
{
  using base_type
  = generic_set<Id, PageSz, Alloc>;

public:
  using component_type
  = C;

  using base_type::identifier_type;
  using base_type::allocator_type;

private:
  using component_allocator = typename std::allocator_traits<Alloc>::template rebind_alloc<C>;
  using component_container = ;

private:
  component_container m_components;

private:
  static constexpr
  bool
  s_noexcept_move_alloc_construct()
  noexcept
  {
    return base_type::s_noexcept_move_alloc_construct()
        && std::is_nothrow_constructible_v<
               component_container,
               component_container &&, component_allocator const &>;
  }

  using base_type
      ::s_noexcept_default_construct;

  static constexpr
  bool
  s_noexcept_swap()
  noexcept
  {
    return base_type::s_noexcept_swap()
        && std::is_nothrow_swappable_v<component_container>;
  }

public:
  explicit constexpr
  generic_pool(allocator_type const &alloc)
  noexcept
    : base_type   {alloc}
    , m_components{alloc}
  { }

  constexpr
  generic_pool(generic_pool const &other, allocator_type const &alloc)
    : base_type   {static_cast<base_type const &>(other), alloc}
    , m_components{other.m_components, alloc}
  { }

  constexpr
  generic_pool(generic_pool &&other, allocator_type const &alloc)
  noexcept(s_noexcept_move_alloc_construct())
    : base_type   {static_cast<base_type &&>(other), alloc}
    , m_components{std::move(other.m_components), alloc}
  { }

  constexpr
  generic_pool()
  noexcept(s_noexcept_default_construct())
    : generic_pool{allocator_type{}}
  { }

  constexpr
  generic_pool(generic_pool const &)
  = default;

  constexpr
  generic_pool(generic_pool &&)
  = default;

  constexpr
  ~generic_pool() override
  = default;

  constexpr
  generic_pool &
  operator=(generic_pool const &)
  = default;

  constexpr
  generic_pool &
  operator=(generic_pool &&)
  = default;

  constexpr
  void
  swap(generic_pool &other)
  noexcept(s_noexcept_swap())
  {
    using std::swap;

    swap(static_cast<base_type &>(*this), static_cast<base_type &>(other));
    swap(m_components, other.m_components);
  }

  friend constexpr
  void
  swap(generic_pool &lhs, generic_pool &rhs)
  noexcept(s_noexcept_swap())
  { lhs.swap(rhs); }

  [[nodiscard]] friend constexpr
  bool
  operator==(generic_pool const &lhs, generic_pool const &rhs)
  noexcept
  {
    if (lhs.size() == rhs.size())
    {
      return std::ranges::all_of(
          lhs,
          [&lhs, &rhs](auto const id) { return rhs.contains(id) && lhs[id] == rhs[id]; });
    }
    return false;
  }


  [[nodiscard]] constexpr
  C &
  operator[](identifier_type const id)
  noexcept;

  [[nodiscard]] constexpr
  C const &
  operator[](identifier_type const id) const
  noexcept;


  template<typename ...Args>
  constexpr
  C &
  emplace(identifier_type const id, Args &&...args);

  template<typename ...Args>
  constexpr
  std::pair<C &, bool>
  try_emplace(identifier_type const id, Args &&...args);

  constexpr
  std::pair<C &, bool>
  insert(identifier_type const id, C const &c);

  constexpr
  std::pair<C &, bool>
  insert(identifier_type const id, C &&c);

  constexpr
  std::pair<C &, bool>
  insert_or_assign(identifier_type const id, C const &c);

  constexpr
  std::pair<C &, bool>
  insert_or_assign(identifier_type const id, C &&c);

  constexpr
  void
  erase(identifier_type const id) override;

  constexpr
  bool
  try_erase(identifier_type const id) override;

  constexpr
  void
  clear()
  noexcept;
};

template<
    typename    C,
    typename    Id,
    std::size_t PageSz,
    typename    Alloc>
requires std::is_empty_v<C>
class generic_pool
  : public generic_set<Id, PageSz, Alloc>
{ };

} // namespace heim::ecs::sparse

#endif // HEIM_ECS_REGISTRY_SPARSE_POOL_HPP
