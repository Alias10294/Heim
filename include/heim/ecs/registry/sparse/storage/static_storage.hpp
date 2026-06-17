#ifndef HEIM_ECS_REGISTRY_SPARSE_STORAGE_STATIC_STORAGE_HPP
#define HEIM_ECS_REGISTRY_SPARSE_STORAGE_STATIC_STORAGE_HPP

#include <cstddef>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>
#include "heim/ecs/identifier.hpp"
#include "heim/lib/type_sequence.hpp"
#include "pool.hpp"
#include "set.hpp"

namespace heim::ecs::sparse
{
template<
    typename    C,
    std::size_t PageSz  = default_page_size_v>
using static_storage_descriptor
= type_sequence<C, std::integral_constant<std::size_t, PageSz>>;


template<
    typename Id      = default_identifier_t,
    typename Alloc   = std::allocator<Id>,
    typename DescSeq = type_sequence<>>
class generic_static_storage
{ };

template<
    typename Id,
    typename Alloc>
class generic_static_storage<Id, Alloc, type_sequence<>>
{ };

template<
    typename       Id,
    typename       Alloc,
    typename    ...Cs,
    std::size_t ...PageSzs>
class generic_static_storage<
    Id,
    Alloc,
    type_sequence<static_storage_descriptor<Id, PageSzs> ...>>
{
public:
  using identifier_type      = Id;
  using allocator_type       = Alloc;
  using description_sequence = type_sequence<static_storage_descriptor<Id, PageSzs> ...>;

private:
  using component_sequence = type_sequence<Cs ...>;
  using container_sequence = type_sequence<generic_pool<Cs, Id, PageSzs, Alloc> ...>;
  using container_tuple    = typename container_sequence::tuple;

  template<typename C>
  requires component_sequence::template contains<C>
  static constexpr
  std::size_t
  component_index
  = component_sequence::template index<C>;

  template<typename C>
  requires component_sequence::template contains<C>
  using pool_for_type
  = container_sequence::template get<component_index<C>>;

private:
  container_tuple m_tuple;

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
  s_noexcept_default_construct()
  noexcept
  { return std::is_nothrow_constructible_v<allocator_type>; }

  static constexpr
  bool
  s_noexcept_swap()
  noexcept
  { return std::is_nothrow_swappable_v<container_tuple>; }

public:
  explicit constexpr
  generic_static_storage(allocator_type const &alloc)
  noexcept
    : m_tuple{std::allocator_arg, alloc}
  { }

  constexpr
  generic_static_storage(generic_static_storage const &other, allocator_type const &alloc)
    : m_tuple{std::allocator_arg, alloc, other.m_tuple}
  { }

  constexpr
  generic_static_storage(generic_static_storage &&other, allocator_type const &alloc)
  noexcept(s_noexcept_move_alloc_construct())
    : m_tuple{std::allocator_arg, alloc, std::move(other.m_tuple)}
  { }

  constexpr
  generic_static_storage()
  noexcept(s_noexcept_default_construct())
    : generic_static_storage{allocator_type{}}
  { }

  constexpr
  generic_static_storage(generic_static_storage const &)
  = default;

  constexpr
  generic_static_storage(generic_static_storage &&)
  = default;

  constexpr
  ~generic_static_storage()
  = default;

  constexpr
  generic_static_storage &
  operator=(generic_static_storage const &)
  = default;

  constexpr
  generic_static_storage &
  operator=(generic_static_storage &&)
  = default;

  constexpr
  void
  swap(generic_static_storage &other)
  noexcept(s_noexcept_swap())
  {
    using std::swap;

    swap(m_tuple, other.m_tuple);
  }

  friend constexpr
  void
  swap(generic_static_storage &lhs, generic_static_storage &rhs)
  noexcept(s_noexcept_swap())
  { lhs.swap(rhs); }

  [[nodiscard]] friend constexpr
  bool
  operator==(generic_static_storage const &, generic_static_storage const &)
  = default;


  template<typename C>
  requires component_sequence::template contains<C>
  [[nodiscard]] constexpr
  pool_for_type<C> &
  assure()
  noexcept
  { return pool<C>(); }

  template<typename C>
  requires component_sequence::template contains<C>
  [[nodiscard]] constexpr
  pool_for_type<C> &
  pool()
  noexcept
  { return std::get<component_index<C>>(m_tuple); }

  template<typename C>
  requires component_sequence::template contains<C>
  [[nodiscard]] constexpr
  pool_for_type<C> const &
  pool() const
  noexcept
  { return std::get<component_index<C>>(m_tuple); }

  constexpr
  void
  clear(identifier_type const id)
  {
    std::apply(
        [id](auto &pool)
        { pool.try_erase(id); },
        m_tuple);
  }

  constexpr
  void
  clear()
  noexcept
  {
    std::apply(
        [](auto &pool)
        { pool.clear(); },
        m_tuple);
  }
};


using static_storage
= generic_static_storage<>;

} // namespace heim::ecs::sparse

#endif // HEIM_ECS_REGISTRY_SPARSE_STORAGE_STATIC_STORAGE_HPP
