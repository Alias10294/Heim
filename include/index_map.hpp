#ifndef HEIM_INDEX_MAP_HPP
#define HEIM_INDEX_MAP_HPP

#include <algorithm>
#include <array>
#include <cstddef>
#include <limits>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

namespace heim
{
template<
    typename    Index,
    typename    T,
    std::size_t PageSize = 4096,
    typename    Alloc    = std::allocator<std::pair<Index const, T>>>
class index_map
{
public:
  using index_type     = Index;
  using mapped_type    = T;

  constexpr static std::size_t page_size
  = PageSize;

  using allocator_type = Alloc;


  using size_type       = std::size_t;
  using difference_type = std::ptrdiff_t;

  using value_type = std::pair<index_type const, mapped_type>;

  using reference       = std::pair<index_type const &, mapped_type &>;
  using const_reference = std::pair<index_type const &, mapped_type const &>;


  template<bool IsConst>
  class generic_iterator;

  using iterator       = generic_iterator<false>;
  using const_iterator = generic_iterator<true>;

private:
  using alloc_traits_t
  = std::allocator_traits<allocator_type>;



  using index_alloc_t
  = typename alloc_traits_t::template rebind_alloc<index_type>;

  using index_alloc_traits_t
  = std::allocator_traits<index_alloc_t>;

  using index_vector_t
  = std::vector<index_type, index_alloc_t>;



  using mapped_alloc_t
  = typename alloc_traits_t::template rebind_alloc<mapped_type>;

  using mapped_alloc_traits_t
  = std::allocator_traits<mapped_alloc_t>;

  using mapped_vector_t
  = std::vector<mapped_type, mapped_alloc_t>;



  using page_t
  = std::array<size_type, page_size>;

  constexpr static size_type s_null_position
  = std::numeric_limits<size_type>::max();

  using page_alloc_t
  = typename alloc_traits_t::template rebind_alloc<page_t>;

  using page_alloc_traits_t
  = std::allocator_traits<page_alloc_t>;


  class page_deleter;

  using page_uptr_t
  = std::unique_ptr<page_t, page_deleter>;


  constexpr static bool is_paged_v
  = PageSize > 0;

  using position_alloc_t
  = typename alloc_traits_t::template rebind_alloc<std::conditional_t<
      is_paged_v,
      page_uptr_t,
      size_type>>;

  using position_vector_t
  = std::conditional_t<
      is_paged_v,
      std::vector<page_uptr_t, position_alloc_t>,
      std::vector<size_type  , position_alloc_t>>;

private:
  [[no_unique_address]]
  allocator_type m_allocator;

  position_vector_t m_positions;
  index_vector_t    m_indexes;
  mapped_vector_t   m_mapped;

private:
  [[nodiscard]]
  constexpr page_uptr_t
  m_clone_page_uptr(page_uptr_t const &page_uptr) const
  {
    page_alloc_t page_alloc{m_allocator};

    if (!page_uptr)
    {
      return page_uptr_t{
          nullptr,
          page_deleter{std::move(page_alloc)}};
    }

    page_t *new_page_ptr{
        page_alloc_traits_t::allocate(page_alloc, 1)};
    try
    {
      page_alloc_traits_t::construct(
          page_alloc,
          new_page_ptr,
          *page_uptr);
    }
    catch (...)
    {
      page_alloc_traits_t::deallocate(
          page_alloc,
          new_page_ptr,
          1);
      throw;
    }

    return page_uptr_t{
        new_page_ptr,
        page_deleter{std::move(page_alloc)}};
  }

  constexpr void
  m_copy_position_vector(
      position_vector_t       &pos,
      position_vector_t const &other_pos)
  {
    pos.reserve(other_pos.size());
    for (auto const &page_uptr : other_pos)
      pos.emplace_back(m_clone_page_uptr(page_uptr));
  }

  constexpr void
  m_move_assign(index_map &other)
  noexcept(
      alloc_traits_t::propagate_on_container_move_assignment::value
   || alloc_traits_t::is_always_equal::value)
  {
    m_positions = std::move(other.m_positions);
    m_indexes   = std::move(other.m_indexes);
    m_mapped    = std::move(other.m_mapped);
  }


  constexpr static size_type
  s_position_page_nb(index_type const i)
  noexcept
  requires (is_paged_v)
  {
    return static_cast<size_type>(i) / page_size;
  }

  constexpr static size_type
  s_position_line_nb(index_type const i)
  noexcept
  requires (is_paged_v)
  {
    return static_cast<size_type>(i) % page_size;
  }

  constexpr size_type &
  m_position_get(index_type const i)
  noexcept
  {
    if constexpr (is_paged_v)
      return (*m_positions[s_position_page_nb(i)])[s_position_line_nb(i)];
    else
      return m_positions[static_cast<size_type>(i)];
  }

  constexpr size_type
  m_position_get(index_type const i) const
  noexcept
  {
    if constexpr (is_paged_v)
      return (*m_positions[s_position_page_nb(i)])[s_position_line_nb(i)];
    else
      return m_positions[static_cast<size_type>(i)];
  }

  constexpr bool
  m_position_contains(index_type const i) const
  noexcept
  {
    if constexpr (is_paged_v)
    {
      size_type page_nb = s_position_page_nb(i);

      return page_nb < m_positions.size()
          && static_cast<bool>(m_positions[page_nb])
          && m_position_get(i) != s_null_position;
    }
    else
    {
      size_type const size_i = static_cast<size_type>(i);

      return size_i < m_positions.size()
          && m_positions[size_i] != s_null_position;
    }
  }


  constexpr void
  m_swap_at(index_type const i, index_type const j)
  noexcept
  {
    if (i == j)
      return;

    size_type &pos_i = m_position_get(i);
    size_type &pos_j = m_position_get(j);

    using std::swap;

    swap(m_indexes[pos_i], m_indexes[pos_j]);
    swap(m_mapped [pos_i], m_mapped [pos_j]);
    swap(pos_i, pos_j);
  }

public:
  constexpr
  index_map()
  noexcept(noexcept(allocator_type{}))
    : index_map{allocator_type{}}
  { }

  constexpr explicit
  index_map(allocator_type const &alloc)
  noexcept
    : m_allocator{alloc},
      m_positions{position_alloc_t{alloc}},
      m_indexes  {index_alloc_t   {alloc}},
      m_mapped   {mapped_alloc_t  {alloc}}
  { }

  constexpr
  index_map(index_map const &other)
  requires (is_paged_v)
    : m_allocator{alloc_traits_t
          ::select_on_container_copy_construction(other.m_allocator)},
      m_positions{position_alloc_t{m_allocator}},
      m_indexes  {other.m_indexes, index_alloc_t {m_allocator}},
      m_mapped   {other.m_mapped , mapped_alloc_t{m_allocator}}
  {
    m_copy_position_vector(m_positions, other.m_positions);
  }

  constexpr
  index_map(index_map const &other)
  requires (!is_paged_v)
    : m_allocator{std::allocator_traits<allocator_type>
          ::select_on_container_copy_construction(other.m_allocator)},
      m_positions{other.m_positions, position_alloc_t{m_allocator}},
      m_indexes  {other.m_indexes  , index_alloc_t   {m_allocator}},
      m_mapped   {other.m_mapped   , mapped_alloc_t  {m_allocator}}
  { }

  constexpr
  index_map(index_map &&other)
  noexcept
  = default;

  constexpr
  index_map(
      index_map                            const &other,
      std::type_identity_t<allocator_type> const &alloc)
  requires (is_paged_v)
    : m_allocator{alloc},
      m_positions{position_alloc_t{m_allocator}},
      m_indexes  {other.m_indexes, index_alloc_t {m_allocator}},
      m_mapped   {other.m_mapped , mapped_alloc_t{m_allocator}}
  {
    m_copy_position_vector(m_positions, other.m_positions);
  }

  constexpr
  index_map(
      index_map                            const &other,
      std::type_identity_t<allocator_type> const &alloc)
  requires (!is_paged_v)
    : m_allocator{alloc},
      m_positions{other.m_positions, position_alloc_t{m_allocator}},
      m_indexes  {other.m_indexes  , index_alloc_t   {m_allocator}},
      m_mapped   {other.m_mapped   , mapped_alloc_t  {m_allocator}}
  { }

  constexpr
  index_map(
      index_map                                 &&other,
      std::type_identity_t<allocator_type> const &alloc)
    : m_allocator{alloc},
      m_positions{std::move(other.m_positions), position_alloc_t{m_allocator}},
      m_indexes  {std::move(other.m_indexes)  , index_alloc_t   {m_allocator}},
      m_mapped   {std::move(other.m_mapped)   , mapped_alloc_t  {m_allocator}}
  { }

  constexpr
  index_map(
      std::initializer_list<value_type> ilist,
      allocator_type const             &alloc = allocator_type{})
    : index_map{alloc}
  {
    // TODO: implement
  }


  constexpr
  ~index_map()
  noexcept
  = default;


  constexpr index_map &
  operator=(index_map const &other)
  {
    if (this == &other)
      return *this;

    if constexpr (alloc_traits_t::propagate_on_container_copy_assignment::value)
    {
      if constexpr (!alloc_traits_t::is_always_equal::value)
      {
        if (m_allocator != other.m_allocator)
        {
          index_map tmp{other, other.m_allocator};
          swap(tmp);
          return *this;
        }
      }
      m_allocator = other.m_allocator;
    }

    if constexpr (!is_paged_v)
      m_positions = other.m_positions;
    else
    {
      position_vector_t positions{position_alloc_t{m_allocator}};
      m_copy_position_vector(positions, other.m_positions);
      m_positions.swap(positions);
    }
    m_indexes = other.m_indexes;
    m_mapped  = other.m_mapped;

    return *this;
  }

  constexpr index_map &
  operator=(index_map &&other)
  noexcept(
      alloc_traits_t::propagate_on_container_move_assignment::value
   || alloc_traits_t::is_always_equal::value)
  {
    m_move_assign(other);

    if constexpr (
        alloc_traits_t::propagate_on_container_move_assignment::value
     || alloc_traits_t::is_always_equal::value)
      m_allocator = std::move(other.m_allocator);

    return *this;
  }

  constexpr index_map &
  operator=(std::initializer_list<value_type> ilist)
  {
    // TODO: implement

    return *this;
  }


  constexpr void
  swap(index_map &other)
  noexcept(
      alloc_traits_t::propagate_on_container_swap::value
   || alloc_traits_t::is_always_equal::value)
  {
    using std::swap;

    if constexpr (alloc_traits_t::propagate_on_container_swap::value)
      swap(m_allocator, other.m_allocator);

    swap(m_positions, other.m_positions);
    swap(m_indexes  , other.m_indexes);
    swap(m_mapped   , other.m_mapped);
  }

  friend constexpr void
  swap(index_map &lhs, index_map &rhs)
  noexcept(noexcept(lhs.swap(rhs)))
  {
    lhs.swap(rhs);
  }


  [[nodiscard]]
  constexpr allocator_type
  get_allocator() const
  noexcept
  {
    return m_allocator;
  }



  [[nodiscard]]
  constexpr iterator
  begin()
  noexcept
  {
    return iterator{this, 0};
  }

  [[nodiscard]]
  constexpr const_iterator
  begin() const
  noexcept
  {
    return const_iterator{this, 0};
  }


  [[nodiscard]]
  constexpr iterator
  end()
  noexcept
  {
    return iterator{this, size()};
  }

  [[nodiscard]]
  constexpr const_iterator
  end() const
  noexcept
  {
    return const_iterator{this, size()};
  }


  [[nodiscard]]
  constexpr const_iterator
  cbegin() const
  noexcept
  {
    return const_iterator{this, 0};
  }


  [[nodiscard]]
  constexpr const_iterator
  cend() const
  noexcept
  {
    return const_iterator{this, size()};
  }



  [[nodiscard]]
  constexpr size_type
  size() const
  noexcept
  {
    return m_indexes.size();
  }


  [[nodiscard]]
  constexpr size_type
  max_size() const
  noexcept
  {
    size_type const index_max  = m_indexes  .max_size();
    size_type const mapped_max = m_mapped   .max_size();

    size_type position_max = m_positions.max_size();
    if constexpr (is_paged_v)
    {
      size_type max_pages = std::numeric_limits<size_type>::max() / page_size;
      if (position_max > max_pages)
        position_max = max_pages;

      position_max *= page_size;
    }

    return std::min({position_max, index_max, mapped_max});
  }


  [[nodiscard]]
  constexpr bool
  empty() const
  noexcept
  {
    return m_indexes.empty();
  }


  constexpr void
  shrink_to_fit()
  noexcept
  {
    // TODO: implement
  }



  [[nodiscard]]
  constexpr bool
  contains(index_type const i) const
  noexcept
  {
    return m_position_contains(i)
        && m_indexes[m_position_get(i)] == i;
  }



  constexpr void
  clear()
  noexcept
  {
    // TODO: implement
  }



  friend constexpr bool
  operator==(index_map const &lhs, index_map const &rhs)
  noexcept
  {
    if (&lhs == &rhs)
      return true;

    if (lhs.size() != rhs.size())
      return false;

    for (auto [idx, val] : lhs)
    {
      if (!rhs.contains(idx))
        return false;

      if (rhs[idx] != val)
        return false;
    }

    return true;
  }


  friend constexpr bool
  operator!=(index_map const &lhs, index_map const &rhs)
  noexcept
  {
    return !(lhs == rhs);
  }

};


template<
    typename    Index,
    typename    T,
    std::size_t PageSize,
    typename    Alloc>
template<bool IsConst>
class index_map<Index, T, PageSize, Alloc>::generic_iterator
{
public:
  constexpr static bool is_const
  = IsConst;

private:
  template<typename U>
  using maybe_const_t = std::conditional_t<
      is_const,
      U const,
      U>;

public:
  using difference_type = std::ptrdiff_t;

  using value_type = std::pair<index_type const, mapped_type>;
  using reference  = std::pair<index_type const, maybe_const_t<mapped_type> &>;

  using iterator_category = std::input_iterator_tag;
  using iterator_concept  = std::random_access_iterator_tag;

private:
  maybe_const_t<index_map  > *m_map;

  index_type const           *m_index;
  maybe_const_t<mapped_type> *m_mapped;

public:
  constexpr
  generic_iterator()
  = default;

  constexpr
  generic_iterator(generic_iterator const &other)
  = default;

  constexpr
  generic_iterator(generic_iterator &&other)
  noexcept
  = default;

  constexpr
  generic_iterator(maybe_const_t<index_map> * const map, size_type const pos)
    : m_map{map},
      m_index {map->m_indexes.data() + pos},
      m_mapped{map->m_mapped .data() + pos}
  { }


  constexpr
  ~generic_iterator()
  noexcept
  = default;


  constexpr generic_iterator &
  operator=(generic_iterator const &other)
  = default;

  constexpr generic_iterator &
  operator=(generic_iterator &&other)
  noexcept
  = default;


  constexpr void
  swap(generic_iterator &other)
  noexcept
  {
    using std::swap;

    swap(m_map   , other.m_map);
    swap(m_index , other.m_index);
    swap(m_mapped, other.m_mapped);
  }

  friend constexpr void
  swap(generic_iterator &lhs, generic_iterator &rhs)
  noexcept(noexcept(lhs.swap(rhs)))
  {
    lhs.swap(rhs);
  }



  constexpr reference
  operator*() const
  noexcept
  {
    return reference{*m_index, *m_mapped};
  }



  constexpr generic_iterator &
  operator++()
  noexcept
  {
    ++m_index;
    ++m_mapped;
    return *this;
  }

  constexpr generic_iterator
  operator++(int)
  noexcept
  {
    auto tmp = *this;
    ++*this;
    return tmp;
  }


  constexpr generic_iterator &
  operator--()
  noexcept
  {
    --m_index;
    --m_mapped;
    return *this;
  }

  constexpr generic_iterator
  operator--(int)
  noexcept
  {
    auto tmp = *this;
    --*this;
    return tmp;
  }


  constexpr generic_iterator &
  operator+=(difference_type const n)
  noexcept
  {
    m_index  += n;
    m_mapped += n;
    return *this;
  }


  constexpr generic_iterator &
  operator-=(difference_type const n)
  noexcept
  {
    m_index  -= n;
    m_mapped -= n;
    return *this;
  }


  friend constexpr generic_iterator
  operator+(generic_iterator const &it, difference_type const n)
  noexcept
  {
    generic_iterator r{it};
    r += n;
    return r;
  }

  friend constexpr generic_iterator
  operator+(difference_type const n, generic_iterator const &it)
  noexcept
  {
    generic_iterator r{it};
    r += n;
    return r;
  }


  friend constexpr generic_iterator
  operator-(generic_iterator const &it, difference_type const n)
  noexcept
  {
    generic_iterator r{it};
    r -= n;
    return r;
  }

  friend constexpr difference_type
  operator-(generic_iterator const &lhs, generic_iterator const &rhs)
  noexcept
  {
    return lhs.m_index - rhs.m_index;
  }



  constexpr reference
  operator[](difference_type const n) const
  noexcept
  {
    return *(*this + n);
  }



  friend constexpr bool
  operator==(generic_iterator const &lhs, generic_iterator const &rhs)
  noexcept
  {
    return lhs.m_index == rhs.m_index;
  }


  friend constexpr auto
  operator<=>(generic_iterator const &lhs, generic_iterator const &rhs)
  noexcept
  {
    return lhs.m_index <=> rhs.m_index;
  }



  friend constexpr maybe_const_t<mapped_type> &&
  iter_move(generic_iterator const &it)
  noexcept
  {
    using std::move;

    return move(*it.m_mapped);
  }


  friend constexpr void
  iter_swap(generic_iterator &lhs, generic_iterator &rhs)
  noexcept(noexcept(lhs.m_map->m_swap_at(*lhs.m_index, *rhs.m_index)))
  requires (!is_const)
  {
    lhs.m_map->m_swap_at(*lhs.m_index, *rhs.m_index);
  }

};


template<
    typename    Index,
    typename    T,
    std::size_t PageSize,
    typename    Alloc>
class index_map<Index, T, PageSize, Alloc>::page_deleter
{
private:
  [[no_unique_address]]
  page_alloc_t m_allocator;

public:
  constexpr
  page_deleter()
    : page_deleter{page_alloc_t{}}
  { }

  constexpr
  page_deleter(page_alloc_t alloc)
    : m_allocator{std::move(alloc)}
  { }



  void
  operator()(page_t *ptr)
  noexcept
  {
    if (!ptr)
      return;

    page_alloc_traits_t::destroy   (m_allocator, ptr);
    page_alloc_traits_t::deallocate(m_allocator, ptr, 1);
  }

};


}

#endif // HEIM_INDEX_MAP_HPP
