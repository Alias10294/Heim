#ifndef HEIM_ECS_REGISTRY_SPARSE_POOL_HPP
#define HEIM_ECS_REGISTRY_SPARSE_POOL_HPP

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>
#include "heim/ecs/identifier.hpp"
#include "set.hpp"

namespace heim::ecs::sparse
{
template<typename T>
struct is_component
  : std::bool_constant<
         std::is_object_v<T>
     && !qualified       <T>
     &&  std::is_move_assignable_v<T>>
{ };

template<typename T>
inline constexpr
bool
is_component_v
= is_component<T>::value;

template<typename T>
concept component
= is_component_v<T>;


template<
    typename    C,
    typename    Id,
    std::size_t PageSz,
    typename    Alloc>
requires (component<C> && identifier<Id> && allocator_for<Alloc, Id>)
class generic_pool
  : public generic_set<Id, PageSz, Alloc>
{
  using base_type
  = generic_set<Id, PageSz, Alloc>;

public:
  using component_type
  = C;

  using typename base_type::identifier_type;
  using typename base_type::allocator_type;

private:
  using typename base_type
      ::id_traits;

  using component_allocator = typename std::allocator_traits<Alloc>::template rebind_alloc<C>;
  using component_container = std::vector<component_type, component_allocator>;

private:
  using base_type::m_sparse;
  using base_type::m_dense;

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

  using base_type
      ::get_allocator;

  [[nodiscard]] friend constexpr
  bool
  operator==(generic_pool const &lhs, generic_pool const &rhs)
  noexcept
  requires std::equality_comparable<component_type>
  {
    if (lhs.size() == rhs.size())
    {
      return std::ranges::all_of(
          lhs,
          [&lhs, &rhs](auto const id) { return rhs.contains(id) && lhs[id] == rhs[id]; });
    }
    return false;
  }


  using base_type::begin;
  using base_type::end;
  using base_type::cbegin;
  using base_type::cend;
  using base_type::rbegin;
  using base_type::rend;
  using base_type::crbegin;
  using base_type::crend;
  using base_type::size;
  using base_type::empty;


  using base_type::contains;
  using base_type::find;

  [[nodiscard]] constexpr
  component_type &
  operator[](identifier_type const id)
  noexcept
  { return m_components[static_cast<std::size_t>(id_traits::index(m_sparse[id]))]; }

  [[nodiscard]] constexpr
  component_type const &
  operator[](identifier_type const id) const
  noexcept
  { return m_components[static_cast<std::size_t>(id_traits::index(m_sparse[id]))]; }


  template<typename ...Args>
  constexpr
  component_type &
  emplace(identifier_type const id, Args &&...args)
  {
    using index_type
    = typename id_traits::index_type;

    m_sparse.assure(id);

    m_components.emplace_back(std::forward<Args>(args)...);
    // strong exception safety guarantee
    try
    { m_dense.emplace_back(id); }
    catch (...)
    { m_components.pop_back(); throw; }

    m_sparse[id] = id_traits::from(static_cast<index_type>(size() - 1), id_traits::generation(id));
    return m_components.back();
  }

  template<typename ...Args>
  constexpr
  std::pair<component_type &, bool>
  try_emplace(identifier_type const id, Args &&...args)
  {
    if (contains(id))
      return {(*this)[id], false};

    return {emplace(id, std::forward<Args>(args)...), true};
  }

  constexpr
  std::pair<component_type &, bool>
  insert(identifier_type const id, C const &c)
  { return try_emplace(id, c); }

  constexpr
  std::pair<component_type &, bool>
  insert(identifier_type const id, C &&c)
  { return try_emplace(id, std::move(c)); }

  constexpr
  std::pair<component_type &, bool>
  insert_or_assign(identifier_type const id, C const &c)
  {
    if (contains(id))
    {
      component_type &ref = (*this)[id];

      ref = c;
      return {ref, false};
    }

    return {emplace(id, c), true};
  }

  constexpr
  std::pair<component_type &, bool>
  insert_or_assign(identifier_type const id, C &&c)
  {
    if (contains(id))
    {
      component_type &ref = (*this)[id];

      ref = std::move(c);
      return {ref, false};
    }

    return {emplace(id, std::move(c)), true};
  }

  constexpr
  void
  erase(identifier_type const id) override
  {
    using index_type
    = typename id_traits::index_type;

    if (auto const idx{static_cast<std::size_t>(id_traits::index(m_sparse[id]))};
        idx != static_cast<std::size_t>(size() - 1))
    {
      identifier_type const back{m_dense.back()};

      m_components[idx]  = std::move(m_components.back());
      m_dense     [idx]  = back;
      m_sparse    [back] = id_traits::from(static_cast<index_type>(idx), id_traits::generation(back));
    }

    m_components.pop_back();
    m_dense     .pop_back();
    m_sparse[id] = id_traits::null;
  }

  constexpr
  bool
  try_erase(identifier_type const id) override
  {
    if (!contains(id))
      return false;

    erase(id);
    return true;
  }

  constexpr
  void
  clear()
  noexcept override
  {
    m_components.clear();
    base_type  ::clear();
  }
};

template<
    typename    C,
    typename    Id,
    std::size_t PageSz,
    typename    Alloc>
requires (component<C> && identifier<Id> && allocator_for<Alloc, Id>
      &&  std::is_empty_v<C>)
class generic_pool<C, Id, PageSz, Alloc>
  : public generic_set<Id, PageSz, Alloc>
{
  using base_type
  = generic_set<Id, PageSz, Alloc>;

public:
  using component_type
  = C;

  using base_type
      ::base_type;
};

} // namespace heim::ecs::sparse

#endif // HEIM_ECS_REGISTRY_SPARSE_POOL_HPP
