#ifndef HEIM_ECS_REGISTRY_SPARSE_MANAGER_HPP
#define HEIM_ECS_REGISTRY_SPARSE_MANAGER_HPP

#include <cstddef>
#include <ranges>
#include <type_traits>
#include <utility>
#include <vector>
#include "heim/ecs/identifier.hpp"

namespace heim::ecs::sparse
{
template<
    typename Id,
    typename Alloc>
class manager
{
public:
  using identifier_type = Id;
  using allocator_type  = Alloc;

private:
  using id_traits
  = identifier_traits<Id>;

  using container_type
  = std::vector<Id, Alloc>;

private:
  container_type m_dense;
  container_type m_sparse;
  std::size_t    m_begin;

private:
  static constexpr
  bool
  s_noexcept_move_alloc_construct()
  noexcept
  { return std::is_nothrow_constructible_v<container_type, container_type, allocator_type>; }

  static constexpr
  bool
  s_noexcept_default_construct()
  noexcept
  { return std::is_nothrow_constructible_v<allocator_type>; }

  static constexpr
  bool
  s_noexcept_swap()
  noexcept
  { return std::is_nothrow_swappable_v<container_type>; }

public:
  explicit constexpr
  manager(allocator_type const alloc)
  noexcept
    : m_dense {alloc}
    , m_sparse{alloc}
    , m_begin {}
  { }

  constexpr
  manager(manager const &other, allocator_type const alloc)
    : m_dense {other.m_dense , alloc}
    , m_sparse{other.m_sparse, alloc}
    , m_begin {other.m_begin}
  { }

  constexpr
  manager(manager &&other, allocator_type const alloc)
  noexcept(s_noexcept_move_alloc_construct())
    : m_dense {std::move(other.m_dense ), alloc}
    , m_sparse{std::move(other.m_sparse), alloc}
    , m_begin {other.m_begin}
  { }

  constexpr
  manager()
  noexcept(s_noexcept_default_construct())
    : manager{allocator_type{}}
  { }

  constexpr
  manager(manager const &)
  = default;

  constexpr
  manager(manager &&)
  = default;

  constexpr
  ~manager()
  = default;

  constexpr
  manager &
  operator=(manager const &)
  = default;

  constexpr
  manager &
  operator=(manager &&)
  = default;

  constexpr
  void
  swap(manager &other)
  noexcept(s_noexcept_swap())
  {
    using std::swap;

    swap(m_dense , other.m_dense);
    swap(m_sparse, other.m_sparse);
    swap(m_begin , other.m_begin);
  }

  friend constexpr
  void
  swap(manager &lhs, manager &rhs)
  noexcept(s_noexcept_swap())
  { lhs.swap(rhs); }

  [[nodiscard]] constexpr
  allocator_type
  get_allocator() const
  noexcept
  { return m_dense.get_allocator(); }

  [[nodiscard]] friend constexpr
  bool
  operator==(manager const &, manager const &)
  = default;


  [[nodiscard]] constexpr auto begin() noexcept       { return std::ranges::crbegin(m_dense); }
  [[nodiscard]] constexpr auto begin() const noexcept { return std::ranges::crbegin(m_dense); }

  [[nodiscard]] constexpr auto end() noexcept       { return std::ranges::crend(m_dense); }
  [[nodiscard]] constexpr auto end() const noexcept { return std::ranges::crend(m_dense); }

  [[nodiscard]] constexpr auto cbegin() const noexcept { return begin(); }
  [[nodiscard]] constexpr auto cend  () const noexcept { return end  (); }

  [[nodiscard]] constexpr auto size () const noexcept { return std::ranges::size(m_dense) - m_begin; }
  [[nodiscard]] constexpr bool empty() const noexcept { return size() == 0; }


  [[nodiscard]] constexpr
  bool
  expired(identifier_type const id) const
  noexcept
  {
    auto const idx{static_cast<std::size_t>(id_traits::index(id))};

    if (idx >= m_sparse.size())
      return true;

    identifier_type const pos{m_sparse[idx]};

    return id_traits::index     (pos)  < m_begin
        || id_traits::generation(pos) != id_traits::generation(id);
  }


  [[nodiscard]] constexpr
  identifier_type
  make()
  {
    using index_type
    = typename id_traits::index_type;


    if (m_begin != 0)
      return m_dense[--m_begin];

    identifier_type const id{id_traits::from(static_cast<index_type>(m_dense.size()), 0)};

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


    identifier_type &pos    {m_sparse[static_cast<std::size_t>(id_traits::index(id))]};
    auto const       pos_idx{id_traits::index(pos)};

    identifier_type &dense_begin{m_dense[m_begin]};
    identifier_type &dense_id   {m_dense[static_cast<std::size_t>(pos_idx)]};
    auto const       begin_idx  {static_cast<std::size_t>(id_traits::index(dense_begin))};
    auto const       begin      {static_cast<index_type>(m_begin)};

    std::swap(dense_begin, dense_id);

    if (pos_idx != begin)
      m_sparse[begin_idx] = id_traits::from(pos_idx, id_traits::generation(dense_id));

    dense_begin = id_traits::next(dense_begin);
    pos         = id_traits::from(begin, id_traits::generation(dense_begin));
    ++m_begin;
  }

  constexpr
  void
  destroy_all()
  noexcept
  {
    auto valid{m_dense | std::views::drop(m_begin)};

    // we shortcut the individual destroy method to avoid unnecessary swaps
    for (identifier_type &id : valid)
    {
      identifier_type &pos{m_sparse[static_cast<std::size_t>(id_traits::index(id))]};

      id  = id_traits::next(id);
      pos = id_traits::next(pos);
    }

    m_begin = m_dense.size();
  }
};

} // namespace heim::ecs::sparse

#endif // HEIM_ECS_REGISTRY_SPARSE_MANAGER_HPP
