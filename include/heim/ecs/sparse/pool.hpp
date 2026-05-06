#ifndef HEIM_ECS_SPARSE_POOL_HPP
#define HEIM_ECS_SPARSE_POOL_HPP

#include "heim/ecs/identifier.hpp"
#include "heim/ecs/component.hpp"
#include "set.hpp"

namespace heim::sparse
{
namespace detail
{
template<
    typename    Component,
    typename    Identifier = default_identifier_t<>,
    std::size_t PageSize   = default_page_size_v<>,
    typename    Allocator  = std::allocator<Identifier>,
    bool        IsTag      = std::is_empty_v<Component>>
class pool_impl
{ };

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
requires component<Component>
class pool_impl<Component, Identifier, PageSize, Allocator, true>
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
    : m_container(component_allocator(alloc))
  { }

  constexpr
  pool_component_container(pool_component_container const &other, allocator_type const &alloc)
    : m_container(other.m_container, component_allocator(alloc))
  { }

  constexpr
  pool_component_container(pool_component_container const &)
  = default;

  constexpr
  pool_component_container(pool_component_container &&other, allocator_type const &alloc)
  noexcept(s_noexcept_move_alloc_construct())
    : m_container(std::move(other.m_container), component_allocator(alloc))
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
  component_type &
  get(std::size_t const idx)
  noexcept
  { return m_container[idx]; }

  [[nodiscard]] constexpr
  component_type const &
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


template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
class pool_impl<Component, Identifier, PageSize, Allocator, false>
  : protected pool_component_container<Component, Allocator>
  , protected set<Identifier, PageSize, Allocator>
{
protected:
  using component_container = pool_component_container<Component, Allocator>;
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
  pool_impl(allocator_type const &alloc)
  noexcept
    : component_container(alloc)
    , set_type           (alloc)
  { }

  constexpr
  pool_impl()
  noexcept(s_noexcept_default_construct())
    : pool_impl(allocator_type())
  { }

  constexpr
  pool_impl(pool_impl const &other, allocator_type const &alloc)
    : component_container(static_cast<component_container const &>(other), alloc)
    , set_type           (static_cast<set_type            const &>(other), alloc)
  { }

  constexpr
  pool_impl(pool_impl const &)
  = default;

  constexpr
  pool_impl(pool_impl &&other, allocator_type const &alloc)
  noexcept(s_noexcept_move_alloc_construct())
    : component_container(static_cast<component_container &&>(other), alloc)
    , set_type           (static_cast<set_type            &&>(other), alloc)
  { }

  constexpr
  pool_impl(pool_impl &&)
  = default;

  constexpr
  ~pool_impl() override
  = default;

  constexpr
  pool_impl &
  operator=(pool_impl const &)
  = default;

  constexpr
  pool_impl &
  operator=(pool_impl &&)
  = default;

  constexpr
  void
  swap(pool_impl &other)
  noexcept(s_noexcept_swap())
  {
    component_container::swap(static_cast<component_container &>(other));
    set_type           ::swap(static_cast<set_type            &>(other));
  }

  friend constexpr
  void
  swap(pool_impl &lhs, pool_impl &rhs)
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
  operator==(pool_impl const &lhs, pool_impl const &rhs)
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
  component_type &
  operator[](identifier_type const id)
  noexcept
  {
    auto const idx = static_cast<std::size_t>(id_traits::index(set_type::sparse_container::position(id)));
    return component_container::get(idx);
  }

  [[nodiscard]] constexpr
  component_type const &
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

} // namespace detail


/*!
 * \brief TODO
 */
template<
    typename    Component,
    typename    Identifier = default_identifier_t<>,
    std::size_t PageSize   = default_page_size_v<>,
    typename    Allocator  = std::allocator<Identifier>>
class pool
  : public detail::pool_impl<Component, Identifier, PageSize, Allocator>
{
  using impl_type
  = detail::pool_impl<Component, Identifier, PageSize, Allocator>;

public:
  using impl_type::impl_type;
  using impl_type::operator=;
};


} // namespace heim::sparse

#endif // HEIM_ECS_SPARSE_POOL_HPP
