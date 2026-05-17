#ifndef HEIM_ECS_REGISTRY_SPARSE_DETAIL_CORE_HPP
#define HEIM_ECS_REGISTRY_SPARSE_DETAIL_CORE_HPP

#include <cstddef>
#include <iterator>
#include <memory>
#include <ranges>
#include <type_traits>
#include <utility>
#include <vector>
#include "heim/ecs/identifier.hpp"
#include "heim/lib/utility.hpp"

namespace heim::sparse::detail
{
template<
    typename Identifier = default_identifier_t<>,
    typename Allocator  = std::allocator<Identifier>>
requires (
    identifier   <Identifier>
 && allocator_for<Allocator, Identifier>)
class generic_static_registry_core
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
  generic_static_registry_core(allocator_type const &alloc)
    : m_dense {alloc}
    , m_sparse{alloc}
    , m_begin {}
  { }

  constexpr
  generic_static_registry_core(generic_static_registry_core const &other, allocator_type const &alloc)
    : m_dense {other.m_dense , alloc}
    , m_sparse{other.m_sparse, alloc}
    , m_begin {other.m_begin}
  { }

  constexpr
  generic_static_registry_core(generic_static_registry_core const &)
  = default;

  constexpr
  generic_static_registry_core(generic_static_registry_core &&other, allocator_type const &alloc)
  noexcept(s_noexcept_move_alloc_construct())
    : m_dense {std::move(other.m_dense ), alloc}
    , m_sparse{std::move(other.m_sparse), alloc}
    , m_begin {other.m_begin}
  { }

  constexpr
  generic_static_registry_core(generic_static_registry_core &&)
  = default;

  constexpr
  ~generic_static_registry_core()
  = default;

  constexpr
  generic_static_registry_core &
  operator=(generic_static_registry_core const &)
  = default;

  constexpr
  generic_static_registry_core &
  operator=(generic_static_registry_core &&)
  = default;

  constexpr
  void
  swap(generic_static_registry_core &other)
  noexcept(s_noexcept_swap())
  {
    std::swap(m_dense , other.m_dense);
    std::swap(m_sparse, other.m_sparse);
    std::swap(m_begin , other.m_begin);
  }

  [[nodiscard]] friend constexpr
  bool
  operator==(generic_static_registry_core const &, generic_static_registry_core const &)
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
  expired(identifier_type const id) const
  noexcept
  {
    auto const idx{static_cast<std::size_t>(id_traits::index(id))};

    if (idx >= m_sparse.size())
      return true;

    identifier_type const pos{m_sparse[idx]};

    return id_traits::index     (pos) < m_begin
        || id_traits::generation(pos) != id_traits::generation(id);
  }


  [[nodiscard]] constexpr
  identifier_type
  create()
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
  clear()
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

} // namespace heim::sparse::detail

#endif // HEIM_ECS_REGISTRY_SPARSE_DETAIL_CORE_HPP
