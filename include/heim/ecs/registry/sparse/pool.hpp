#ifndef HEIM_ECS_REGISTRY_SPARSE_POOL_HPP
#define HEIM_ECS_REGISTRY_SPARSE_POOL_HPP

#include <concepts>
#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>
#include "heim/ecs/identifier.hpp"
#include "heim/lib/utility.hpp"
#include "set.hpp"

namespace heim::sparse
{
template<typename T>
struct is_component
  : std::bool_constant<
         std ::is_object_v   <T>
     && !heim::is_qualified_v<T>
     &&  std ::is_move_assignable_v<T>>
{ };

template<typename T>
inline constexpr
bool
is_component_v
= is_component<T>::value;

template<typename T>
concept component
= is_component_v<T>;


/*!
 * \brief
 *   The main underlying container for identifiers and a specific component type.
 *
 * \details
 *   Implements a specialized sparse set data structure, that uses pagination to avoid significant memory
 *   overhead. \n
 *   This data structure allows for constant-time complexity insertion, removal and access to elements,
 *   as well as providing optimal iteration speed.
 *
 * \note
 *   Using a specializing page size of zero (0) will cause the container to not use pagination. This
 *   can be an option to very slightly improve performance when used identifier values are low.
 */
template<
    typename    Component,
    typename    Identifier = default_identifier_t<>,
    std::size_t PageSize   = default_page_size_v<>,
    typename    Allocator  = std::allocator<Identifier>>
class pool
{ };

/*!
 * \brief
 *   The main underlying container for identifiers and a specific component type.
 *
 * \details
 *   Implements a specialized sparse set data structure, that uses pagination to avoid significant memory
 *   overhead. \n
 *   This data structure allows for constant-time complexity insertion, removal and access to elements,
 *   as well as providing optimal iteration speed.
 *
 * \note
 *   Using a specializing page size of zero (0) will cause the container to not use pagination. This
 *   can be an option to very slightly improve performance when used identifier values are low.
 */
template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
requires (component<Component> && std::is_empty_v<Component>)
class pool<Component, Identifier, PageSize, Allocator>
  : public set<Identifier, PageSize, Allocator>
{
protected:
  using set_type
  = set<Identifier, PageSize, Allocator>;

public:
  using component_type  = Component;
  using identifier_type = Identifier;
  using allocator_type  = Allocator;

  static_assert(
      is_component_v<Component>,
      "heim::sparse::detail::pool_impl: component_type must be a component type.");

  static constexpr std::size_t page_size
  = PageSize;

public:
  using set_type::set_type;
  using set_type::operator=;
};


namespace detail
{
template<
    typename Component,
    typename Allocator>
requires component<Component>
class pool_component_container
{
public:
  using component_type = Component;
  using allocator_type = Allocator;

private:
  using component_allocator = typename std::allocator_traits<allocator_type>::template rebind_alloc<component_type>;
  using component_container = std::vector<component_type, component_allocator>;

private:
  component_container m_container;

private:
  static constexpr
  bool
  s_noexcept_move_alloc_construct()
  noexcept
  {
    return std::is_nothrow_constructible_v<
        component_container,
        component_container &&, allocator_type const &>;
  }

  static constexpr
  bool
  s_noexcept_swap()
  noexcept
  { return std::is_nothrow_swappable_v<component_container>; }

  static constexpr
  bool
  s_noexcept_swap_components()
  noexcept
  { return std::is_nothrow_swappable_v<component_type>; }

  static constexpr
  bool
  s_noexcept_overwrite_with_back()
  noexcept
  { return std::is_nothrow_move_assignable_v<component_type>; }

public:
  explicit constexpr
  pool_component_container(allocator_type const &alloc)
  noexcept
    : m_container(component_allocator{alloc})
  { }

  constexpr
  pool_component_container(pool_component_container const &other, allocator_type const &alloc)
    : m_container(other.m_container, component_allocator{alloc})
  { }

  constexpr
  pool_component_container(pool_component_container const &)
  = default;

  constexpr
  pool_component_container(pool_component_container &&other, allocator_type const &alloc)
  noexcept(s_noexcept_move_alloc_construct())
    : m_container(std::move(other.m_container), component_allocator{alloc})
  { }

  constexpr
  pool_component_container(pool_component_container &&)
  = default;

  constexpr
  ~pool_component_container()
  = default;

  constexpr
  void
  swap(pool_component_container &other)
  noexcept(s_noexcept_swap())
  { std::swap(m_container, other.m_container); }

  constexpr
  void
  swap(std::size_t const lhs, std::size_t const rhs)
  noexcept(s_noexcept_swap_components())
  { using std::swap; swap(m_container[lhs], m_container[rhs]); }


  [[nodiscard]] constexpr
  decltype(auto)
  get(std::size_t const idx)
  noexcept
  { return m_container[idx]; }

  [[nodiscard]] constexpr
  decltype(auto)
  get(std::size_t const idx) const
  noexcept
  { return m_container[idx]; }


  template<typename ...Args>
  requires std::constructible_from<component_type, Args &&...>
  constexpr
  void
  emplace_back(Args &&...args)
  { m_container.emplace_back(std::forward<Args>(args)...); }

  constexpr
  void
  pop_back()
  noexcept
  { m_container.pop_back(); }

  constexpr
  void
  overwrite_with_back(std::size_t const idx)
  noexcept(s_noexcept_overwrite_with_back())
  requires std::is_move_assignable_v<component_type>
  { m_container[idx] = std::move(m_container.back()); }

  constexpr
  void
  clear()
  noexcept
  { m_container.clear(); }
};

} // namespace detail


/*!
 * \brief
 *   The main underlying container for identifiers and a specific component type.
 *
 * \details
 *   Implements a specialized sparse set data structure, that uses pagination to avoid significant memory
 *   overhead. \n
 *   This data structure allows for constant-time complexity insertion, removal and access to elements,
 *   as well as providing optimal iteration speed.
 *
 * \note
 *   Using a specializing page size of zero (0) will cause the container to not use pagination. This
 *   can be an option to very slightly improve performance when used identifier values are low.
 */
template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
requires (component<Component> && !std::is_empty_v<Component>)
class pool<Component, Identifier, PageSize, Allocator>
  : protected detail::pool_component_container<Component, Allocator>
  , protected set<Identifier, PageSize, Allocator>
{
protected:
  using component_container = detail::pool_component_container<Component, Allocator>;
  using set_type            = set<Identifier, PageSize, Allocator>;

public:
  using component_type  = Component;
  using identifier_type = Identifier;
  using allocator_type  = Allocator;

  static_assert(
      is_component_v<Component>,
      "heim::sparse::detail::pool_impl: component_type must be a component type.");

  static constexpr std::size_t page_size
  = PageSize;

private:
  using id_traits
  = identifier_traits<identifier_type>;

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
            component_container,
            component_container &&, allocator_type const &>
     && std::is_nothrow_constructible_v<
            set_type,
            set_type            &&, allocator_type const &>;
  }

  static constexpr
  bool
  s_noexcept_swap()
  noexcept
  {
    return std::is_nothrow_swappable_v<component_container>
        && std::is_nothrow_swappable_v<set_type>;
  }

  static constexpr
  bool
  s_noexcept_erase()
  noexcept
  { return noexcept(std::declval<component_container &>().overwrite_with_back(std::declval<std::size_t>())); }

public:
  explicit constexpr
  pool(allocator_type const &alloc)
  noexcept
    : component_container{alloc}
    , set_type           {alloc}
  { }

  constexpr
  pool()
  noexcept(s_noexcept_default_construct())
    : pool{allocator_type{}}
  { }

  constexpr
  pool(pool const &other, allocator_type const &alloc)
    : component_container{static_cast<component_container const &>(other), alloc}
    , set_type           {static_cast<set_type            const &>(other), alloc}
  { }

  constexpr
  pool(pool const &)
  = default;

  constexpr
  pool(pool &&other, allocator_type const &alloc)
  noexcept(s_noexcept_move_alloc_construct())
    : component_container{static_cast<component_container &&>(other), alloc}
    , set_type           {static_cast<set_type            &&>(other), alloc}
  { }

  constexpr
  pool(pool &&)
  = default;

  constexpr
  ~pool() override
  = default;

  constexpr
  pool &
  operator=(pool const &)
  = default;

  constexpr
  pool &
  operator=(pool &&)
  = default;

  constexpr
  void
  swap(pool &other)
  noexcept(s_noexcept_swap())
  {
    component_container::swap(static_cast<component_container &>(other));
    set_type           ::swap(static_cast<set_type            &>(other));
  }

  friend constexpr
  void
  swap(pool &lhs, pool &rhs)
  noexcept(s_noexcept_swap())
  { lhs.swap(rhs); }

  constexpr
  void
  swap(identifier_type lhs, identifier_type const rhs)
  {
    using std::swap;

    auto const lhs_idx = static_cast<std::size_t>(id_traits::index(set_type::sparse_container::position(lhs)));
    auto const rhs_idx = static_cast<std::size_t>(id_traits::index(set_type::sparse_container::position(rhs)));

    component_container       ::swap(lhs_idx, rhs_idx);
    set_type::dense_container ::swap(lhs_idx, rhs_idx);
    set_type::sparse_container::swap(lhs    , rhs);
  }

  [[nodiscard]] friend constexpr
  bool
  operator==(pool const &lhs, pool const &rhs)
  noexcept
  {
    if (lhs.size() != rhs.size())
      return false;

    for (identifier_type const id : lhs)
    {
      if (!rhs.contains(id))
        return false;

      if (lhs[id] != rhs[id])
        return false;
    }
    return true;
  }

  using set_type::get_allocator;
  using set_type::begin;
  using set_type::end;
  using set_type::cbegin;
  using set_type::cend;
  using set_type::size;
  using set_type::empty;
  using set_type::contains;
  using set_type::iterator_to;
  using set_type::find;

  [[nodiscard]] constexpr
  decltype(auto)
  operator[](identifier_type const id)
  noexcept
  {
    auto const idx = static_cast<std::size_t>(id_traits::index(set_type::sparse_container::position(id)));
    return component_container::get(idx);
  }

  [[nodiscard]] constexpr
  decltype(auto)
  operator[](identifier_type const id) const
  noexcept
  {
    auto const idx = static_cast<std::size_t>(id_traits::index(set_type::sparse_container::position(id)));
    return component_container::get(idx);
  }

  template<typename ...Args>
  constexpr
  void
  emplace(identifier_type const id, Args && ...args)
  {
    using index_type
    = typename id_traits::index_type;

    set_type::sparse_container::reserve_for(id);

    component_container::emplace_back(std::forward<Args>(args)...);
    // strong exception safety guarantee
    try
    { set_type::dense_container::insert(id); }
    catch (...)
    { component_container::pop_back(); throw; }

    set_type::sparse_container::position(id) = id_traits::from(static_cast<index_type>(size() - 1), id_traits::generation(id));
  }

  template<typename ...Args>
  constexpr
  bool
  try_emplace(identifier_type const id, Args && ...args)
  {
    if (contains(id))
      return false;

    emplace(id, std::forward<Args>(args)...);
    return true;
  }

  constexpr
  bool
  insert(identifier_type const id, component_type const &c)
  { return try_emplace(id, c); }

  constexpr
  bool
  insert(identifier_type const id, component_type &&c)
  { return try_emplace(id, std::move(c)); }

  constexpr
  bool
  insert_or_assign(identifier_type const id, component_type const &c)
  {
    if (contains(id))
    { (*this)[id] = c; return false; }

    emplace(id, c);
    return true;
  }

  constexpr
  bool
  insert_or_assign(identifier_type const id, component_type &&c)
  {
    if (contains(id))
    { (*this)[id] = std::move(c); return false; }

    emplace(id, std::move(c));
    return true;
  }


  constexpr
  void
  erase(identifier_type const id) override
  {
    using index_type
    = typename id_traits::index_type;

    if (auto const idx = static_cast<std::size_t>(id_traits::index(set_type::sparse_container::position(id)));
        idx != size() - 1)
    {
      identifier_type const back = set_type::dense_container::back();

      component_container       ::overwrite_with_back(idx);
      set_type::dense_container ::overwrite_with_back(idx);
      set_type::sparse_container::position(back) = id_traits::from(static_cast<index_type>(idx), id_traits::generation(back));
    }

    component_container       ::pop_back();
    set_type::dense_container ::pop_back();
    set_type::sparse_container::position(id) = id_traits::null;
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
  noexcept
  {
    component_container::clear();
    set_type           ::clear();
  }
};

} // namespace heim::sparse

#endif // HEIM_ECS_REGISTRY_SPARSE_POOL_HPP
