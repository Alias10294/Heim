#ifndef HEIM_INDEX_MAP_HPP
#define HEIM_INDEX_MAP_HPP

#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <iterator>
#include <limits>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace heim
{
namespace detail
{
template<typename T>
struct index_map_is_index
{
  constexpr static bool
  value
  =   std::is_same_v<T, std::remove_cvref_t<T>>
   && std::is_integral_v <T>
   && std::is_unsigned_v <T>;

};

template<typename T>
constexpr inline bool
index_map_is_index_v = index_map_is_index<T>::value;


template<typename T>
struct index_map_is_mapped
{
  constexpr static bool
  value
  =   std::is_same_v<T, std::remove_cv_t<T>>
   && std::is_object_v              <T>
   && std::is_nothrow_destructible_v<T>;

};

template<typename T>
constexpr inline bool
index_map_is_mapped_v = index_map_is_mapped<T>::value;


}


template<
    typename    Index,
    typename    T,
    std::size_t PageSize = 4096,
    typename    Alloc    = std::allocator<std::pair<Index const, T>>>
class index_map
{
private:
  static_assert(
      detail::index_map_is_index_v<Index>,
      "heim::index_map: detail::index_map_is_index_v<Index>");
  static_assert(
      detail::index_map_is_mapped_v<T>,
      "heim::index_map: detail::index_map_is_mapped_v<T>");

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

  using reverse_iterator       = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

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
  template<typename ...Args>
  [[nodiscard]]
  constexpr page_uptr_t
  m_make_page_uptr(Args &&...args) const
  {
    page_alloc_t page_alloc{m_allocator};

    page_t *new_page_ptr{page_alloc_traits_t::allocate(page_alloc, 1)};

    try
    {
      page_alloc_traits_t::construct(
          page_alloc,
          new_page_ptr,
          std::forward<Args>(args)...);
    }
    catch (...)
    {
      page_alloc_traits_t::deallocate(page_alloc, new_page_ptr, 1);
      throw;
    }

    return page_uptr_t{new_page_ptr, page_deleter{std::move(page_alloc)}};
  }

  [[nodiscard]]
  constexpr page_uptr_t
  m_make_page_uptr(std::nullptr_t) const
  {
    page_alloc_t page_alloc{m_allocator};
    return page_uptr_t{nullptr, page_deleter{std::move(page_alloc)}};
  }



  constexpr void
  m_copy_position_vector(
      position_vector_t       &pos,
      position_vector_t const &other_pos)
  requires (is_paged_v)
  {
    pos.reserve(other_pos.size());
    for (auto const &page_uptr : other_pos)
    {
      if (page_uptr)
        pos.emplace_back(m_make_page_uptr(*page_uptr));
      else
        pos.emplace_back(m_make_page_uptr(nullptr));
    }
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
    if constexpr (std::has_single_bit(page_size))
      return static_cast<size_type>(i) >> std::countr_zero(page_size);
    else
      return static_cast<size_type>(i) / page_size;
  }


  constexpr static size_type
  s_position_line_nb(index_type const i)
  noexcept
  requires (is_paged_v)
  {
    if constexpr (std::has_single_bit(page_size))
      return static_cast<size_type>(i) & (page_size - 1);
    else
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
      size_type const page_nb = s_position_page_nb(i);

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
  noexcept(std::is_nothrow_swappable_v<mapped_type>)
  requires (std::is_swappable_v<mapped_type>)
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



  constexpr void
  m_set_capacity(size_type const new_cap)
  {
    index_vector_t  new_indexes{index_alloc_t {m_allocator}};
    mapped_vector_t new_mapped {mapped_alloc_t{m_allocator}};

    new_indexes.reserve(new_cap);
    new_mapped .reserve(new_cap);

    new_indexes.assign(
        std::make_move_iterator(m_indexes.begin()),
        std::make_move_iterator(m_indexes.end  ()));
    new_mapped .assign(
        std::make_move_iterator(m_mapped.begin()),
        std::make_move_iterator(m_mapped.end  ()));

    using std::swap;
    swap(m_indexes, new_indexes);
    swap(m_mapped , new_mapped);
  }



  template<typename ...Args>
  constexpr void
  m_emplace(index_type const i, Args &&...args)
  requires (std::constructible_from<mapped_type, Args ...>)
  {
    // ensure available slot for position
    if constexpr (is_paged_v)
    {
      size_type const page_nb = s_position_page_nb(i);

      if (page_nb >= m_positions.size())
        m_positions.resize(page_nb + 1);

      if (!m_positions[page_nb])
      {
        m_positions[page_nb] = m_make_page_uptr();
        m_positions[page_nb] ->fill(s_null_position);
      }
    }
    else
    {
      size_type const size_i = static_cast<size_type>(i);

      if (size_i >= m_positions.size())
        m_positions.resize(size_i + 1, s_null_position);
    }

    // strong exception guarantee with pop_back being fully noexcept
    m_mapped.emplace_back(std::forward<Args>(args)...);
    try
    {
      m_indexes.emplace_back(i);
    }
    catch (...)
    {
      m_mapped.pop_back();
      throw;
    }
    m_position_get(i) = size() - 1;
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
  requires (
      is_paged_v
   && std::is_copy_constructible_v<mapped_type>)
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
  requires (
     !is_paged_v
   && std::is_copy_constructible_v<mapped_type>)
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
  requires (
      is_paged_v
   && std::is_copy_constructible_v<mapped_type>)
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
  requires (
     !is_paged_v
   && std::is_copy_constructible_v<mapped_type>)
    : m_allocator{alloc},
      m_positions{other.m_positions, position_alloc_t{m_allocator}},
      m_indexes  {other.m_indexes  , index_alloc_t   {m_allocator}},
      m_mapped   {other.m_mapped   , mapped_alloc_t  {m_allocator}}
  { }

  constexpr
  index_map(
      index_map                                 &&other,
      std::type_identity_t<allocator_type> const &alloc)
  noexcept
    : m_allocator{alloc},
      m_positions{std::move(other.m_positions), position_alloc_t{m_allocator}},
      m_indexes  {std::move(other.m_indexes)  , index_alloc_t   {m_allocator}},
      m_mapped   {std::move(other.m_mapped)   , mapped_alloc_t  {m_allocator}}
  { }

  constexpr
  index_map(
      std::initializer_list<value_type> ilist,
      allocator_type const             &alloc = allocator_type{})
  requires (std::is_copy_constructible_v<mapped_type>)
    : index_map{alloc}
  {
    reserve(ilist.size());
    insert(ilist);
  }


  constexpr
  ~index_map()
  noexcept
  = default;


  constexpr index_map &
  operator=(index_map const &other)
  requires (std::is_copy_assignable_v<mapped_type>)
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

    if constexpr (is_paged_v)
    {
      position_vector_t positions{position_alloc_t{m_allocator}};
      m_copy_position_vector(positions, other.m_positions);
      m_positions.swap(positions);
    }
    else
      m_positions = other.m_positions;

    m_indexes = other.m_indexes;
    m_mapped  = other.m_mapped;

    return *this;
  }

  constexpr index_map &
  operator=(index_map &&other)
  noexcept(
      alloc_traits_t::propagate_on_container_move_assignment::value
   || alloc_traits_t::is_always_equal::value)
  requires (std::is_move_assignable_v<mapped_type>)
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
  requires (std::is_copy_assignable_v<mapped_type>)
  {
    index_map tmp{ilist, m_allocator};
    swap(tmp);

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
  constexpr reverse_iterator
  rbegin()
  noexcept
  {
    return reverse_iterator{end()};
  }

  [[nodiscard]]
  constexpr const_reverse_iterator
  rbegin() const
  noexcept
  {
    return const_reverse_iterator{end()};
  }


  [[nodiscard]]
  constexpr reverse_iterator
  rend()
  noexcept
  {
    return reverse_iterator{begin()};
  }

  [[nodiscard]]
  constexpr const_reverse_iterator
  rend() const
  noexcept
  {
    return const_reverse_iterator{begin()};
  }


  [[nodiscard]]
  constexpr const_reverse_iterator
  crbegin() const
  noexcept
  {
    return const_reverse_iterator{cend()};
  }


  [[nodiscard]]
  constexpr const_reverse_iterator
  crend() const
  noexcept
  {
    return const_reverse_iterator{cbegin()};
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
    size_type const index_max  = m_indexes.max_size();
    size_type const mapped_max = m_mapped .max_size();

    size_type position_max = m_positions.max_size();
    if constexpr (is_paged_v)
    {
      size_type const
      max_pages = std::numeric_limits<size_type>::max() / page_size;

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


  [[nodiscard]]
  constexpr size_type
  capacity() const
  noexcept
  {
    return m_indexes.capacity();
  }


  constexpr void
  reserve(size_type const new_cap)
  {
    if (new_cap <= capacity())
      return;

    if (new_cap > max_size())
      throw std::length_error{"heim::index_map::reserve(size_type const)"};

    m_set_capacity(new_cap);
  }


  constexpr void
  shrink_to_fit()
  {
    m_set_capacity(size());

    if constexpr (is_paged_v)
    {
      for (auto &page_uptr : m_positions)
      {
        if (page_uptr)
        {
          bool const is_blank = std::all_of(
              page_uptr->begin(), page_uptr->end(),
              [](size_type const pos)
              {
                return pos == s_null_position;
              });

          if (is_blank)
            page_uptr.reset();
        }
      }
      while (!m_positions.empty() && !m_positions.back())
        m_positions.pop_back();
    }
    else
    {
      while (!m_positions.empty() && m_positions.back() == s_null_position)
        m_positions.pop_back();
    }
    // shrinking the positions' vector is not a requirement, we prefer to just
    // ignore failure here
    try
    {
      m_positions.shrink_to_fit();
    }
    catch (...)
    { }
  }



  [[nodiscard]]
  constexpr bool
  contains(index_type const i) const
  noexcept
  {
    return m_position_contains(i)
        && m_indexes[m_position_get(i)] == i;
  }


  [[nodiscard]]
  constexpr iterator
  find(index_type const i)
  noexcept
  {
    if (!contains(i))
      return end();

    return iterator{this, m_position_get(i)};
  }

  [[nodiscard]]
  constexpr const_iterator
  find(index_type const i) const
  noexcept
  {
    if (!contains(i))
      return end();

    return const_iterator{this, m_position_get(i)};
  }


  [[nodiscard]]
  constexpr mapped_type &
  operator[](index_type const i)
  noexcept
  {
    return m_mapped[m_position_get(i)];
  }

  [[nodiscard]]
  constexpr mapped_type const &
  operator[](index_type const i) const
  noexcept
  {
    return m_mapped[m_position_get(i)];
  }


  [[nodiscard]]
  constexpr mapped_type &
  at(index_type const i)
  {
    if (!contains(i))
      throw std::out_of_range{"heim::index_map::at(index_type)"};

    return operator[](i);
  }

  [[nodiscard]]
  constexpr mapped_type const &
  at(index_type const i) const
  {
    if (!contains(i))
      throw std::out_of_range{"heim::index_map::at(index_type) const"};

    return operator[](i);
  }



  template<typename ...Args>
  constexpr std::pair<iterator, bool>
  emplace(index_type const i, Args &&...args)
  requires (std::constructible_from<mapped_type, Args ...>)
  {
    if (contains(i))
      return {iterator{this, m_position_get(i)}, false};

    m_emplace(i, std::forward<Args>(args)...);
    return {--end(), true};
  }


  template<typename ...Args>
  constexpr std::pair<iterator, bool>
  emplace_or_assign(index_type const i, Args &&...args)
  requires (
      std::constructible_from<mapped_type, Args ...>
   && std::is_move_assignable_v<mapped_type>)
  {
    if (contains(i))
    {
      m_mapped[m_position_get(i)] = mapped_type{std::forward<Args>(args)...};
      return {iterator{this, m_position_get(i)}, false};
    }

    m_emplace(i, std::forward<Args>(args)...);
    return {--end(), true};
  }


  constexpr std::pair<iterator, bool>
  insert(index_type const i, mapped_type const &m)
  requires (std::is_copy_constructible_v<mapped_type>)
  {
    return emplace(i, m);
  }

  constexpr std::pair<iterator, bool>
  insert(index_type const i, mapped_type &&m)
  requires (std::is_move_constructible_v<mapped_type>)
  {
    return emplace(i, std::move(m));
  }

  template<typename M>
  constexpr std::pair<iterator, bool>
  insert(index_type const i, M &&m)
  requires (std::constructible_from<mapped_type, M &&>)
  {
    return emplace(i, std::forward<M>(m));
  }

  constexpr std::pair<iterator, bool>
  insert(value_type const &value)
  requires (std::is_copy_constructible_v<mapped_type>)
  {
    return emplace(std::get<0>(value), std::get<1>(value));
  }

  constexpr std::pair<iterator, bool>
  insert(value_type &&value)
  requires (std::is_move_constructible_v<mapped_type>)
  {
    return emplace(std::get<0>(value), std::get<1>(std::move(value)));
  }

  template<typename InputIt>
  constexpr void
  insert(InputIt first, InputIt last)
  requires (
      std::is_move_constructible_v<mapped_type>
   && std::input_iterator<InputIt>
   && std::convertible_to<
          std::tuple_element_t<0, std::iter_reference_t<InputIt>>,
          index_type>
   && std::convertible_to<
          std::tuple_element_t<1, std::iter_reference_t<InputIt>>,
          mapped_type>)
  {
    if constexpr (std::forward_iterator<InputIt>)
      reserve(size() + static_cast<size_type>(std::distance(first, last)));

    for (; first != last; ++first)
    {
      auto &&value = *first;
      insert(std::get<0>(value), std::get<1>(std::move(value)));
    }
  }

  constexpr void
  insert(std::initializer_list<value_type> ilist)
  requires (std::is_copy_constructible_v<mapped_type>)
  {
    reserve(size() + ilist.size());

    for (value_type const &value : ilist)
      insert(value);
  }


  template<typename R>
  constexpr void
  insert_range(R &&rg)
  requires (
      std::is_move_constructible_v<mapped_type>
   && std::ranges::input_range<R>
   && std::convertible_to<
          std::tuple_element_t<0, std::ranges::range_reference_t<R>>,
          index_type>
   && std::convertible_to<
          std::tuple_element_t<1, std::ranges::range_reference_t<R>>,
          mapped_type>)
  {
    insert(rg.begin(), rg.end());
  }


  constexpr bool
  erase(index_type const i)
  noexcept(std::is_nothrow_move_assignable_v<mapped_type>)
  requires (std::is_move_assignable_v<mapped_type>)
  {
    if (!contains(i))
      return false;

    size_type &pos = m_position_get(i);

    if (index_type const ilast = m_indexes.back(); i != ilast)
    {
      m_mapped [pos] = std::move(m_mapped.back());
      m_indexes[pos] = ilast;

      m_position_get(ilast) = pos;
    }

    m_indexes.pop_back();
    m_mapped .pop_back();

    pos = s_null_position;

    return true;
  }

  constexpr iterator
  erase(iterator pos)
  noexcept(noexcept(erase(std::declval<index_type>())))
  requires (std::is_move_assignable_v<mapped_type>)
  {
    erase(std::get<0>(*pos));
    return pos;
  }

  constexpr iterator
  erase(const_iterator pos)
  noexcept(noexcept(erase(std::declval<iterator>())))
  requires (std::is_move_assignable_v<mapped_type>)
  {
    return erase(begin() + (pos - cbegin()));
  }

  constexpr iterator
  erase(iterator first, iterator last)
  noexcept(noexcept(erase(std::declval<iterator>())))
  requires (std::is_move_assignable_v<mapped_type>)
  {
    while (last != first)
      last = erase(--last);

    return last;
  }

  constexpr iterator
  erase(const_iterator first, const_iterator last)
  noexcept(noexcept(erase(std::declval<iterator>(), std::declval<iterator>())))
  requires (std::is_move_assignable_v<mapped_type>)
  {
    return erase(
        begin() + (first - cbegin()),
        begin() + (last  - cbegin()));
  }


  template<typename Pred>
  friend constexpr size_type
  erase_if(index_map &c, Pred pred)
  noexcept(
      noexcept(pred(std::declval<reference>()))
   && noexcept(c.erase(std::declval<index_type const>())))
  requires (std::is_move_assignable_v<mapped_type>)
  {
    size_type old_size = c.size();

    for (auto it = c.rbegin(); it != c.rend(); ++it)
    {
      if (pred(*it))
        c.erase(std::get<0>(*it));
    }

    return old_size - c.size();
  }


  constexpr void
  clear()
  noexcept
  {
    if constexpr (is_paged_v)
    {
      for (auto &page_uptr : m_positions)
      {
        if (page_uptr)
          std::fill(page_uptr->begin(), page_uptr->end(), s_null_position);
      }
    }
    else
      std::fill(m_positions.begin(), m_positions.end(), s_null_position);

    m_indexes.clear();
    m_mapped .clear();
  }



  friend constexpr bool
  operator==(index_map const &lhs, index_map const &rhs)
  noexcept
  {
    if (&lhs == &rhs)
      return true;

    if (lhs.size() != rhs.size())
      return false;

    for (auto &&[idx, val] : lhs)
    {
      if (!rhs.contains(idx))
        return false;

      if (rhs[idx] != val)
        return false;
    }

    return true;
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
  maybe_const_t<index_map> *m_map;

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

  constexpr explicit
  generic_iterator(generic_iterator<!is_const> other)
  requires (is_const)
    : m_map{other.m_map},
      m_index {other.m_index},
      m_mapped{other.m_mapped}
  { }

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



  friend constexpr auto
  iter_move(generic_iterator const &it)
  noexcept(noexcept(std::move(*it.m_mapped)))
  {
    if constexpr (is_const)
    {
      return std::pair<index_type const, mapped_type const &&>{
          *it.m_index,
          std::move(*it.m_mapped)};
    }
    else
    {
      return std::pair<index_type const, mapped_type &&>{
          *it.m_index,
          std::move(*it.m_mapped)};
    }
  }


  friend constexpr void
  iter_swap(generic_iterator &lhs, generic_iterator &rhs)
  noexcept(noexcept(lhs.m_map->m_swap_at(*lhs.m_index, *rhs.m_index)))
  requires (!is_const && std::is_swappable_v<mapped_type>)
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

  constexpr explicit
  page_deleter(page_alloc_t alloc)
    : m_allocator{std::move(alloc)}
  { }


  constexpr
  ~page_deleter()
  noexcept
  = default;



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
