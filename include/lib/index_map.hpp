#ifndef HEIM_LIB_INDEX_MAP_HPP
#define HEIM_LIB_INDEX_MAP_HPP

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
//! @cond INTERNAL
/*!
 * @brief Checks whether @code T@endcode is a valid index type an index map or
 *   not.
 *
 * @tparam T The type to check the validity of.
 */
template<typename T>
struct index_map_is_index
{
  constexpr static bool
  value
  =   std::is_same_v<T, std::remove_cvref_t<T>>
   && std::is_integral_v<T>
   && std::is_unsigned_v<T>;

};

template<typename T>
constexpr inline bool
index_map_is_index_v = index_map_is_index<T>::value;


/*!
 * @brief Checks whether @code T@endcode is a valid mapped type an index map or
 *   not.
 *
 * @tparam T The type to check the validity of.
 */
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

//! @endcond

} // namespace detail


/*!
 * @brief An associative container specialized for unsigned integral keys,
 *   providing constant time search, insertion and removal of elements, as well
 *   as memory contiguity.
 *
 * @tparam Index    The type of the indexes.
 * @tparam T        The type of the values.
 * @tparam PageSize The size of each internal page of positions.
 * @tparam Alloc    The type of allocator used in the container.
 *
 * @details Implements a customized sparse set. Both keys and values are stored
 *   contiguously in their own dense dynamic array. Just like
 *   @c std::ranges::zip_view, This structure forces its iterators to
 *   dereference to a pair of references not a reference of a pair, limiting
 *   compatibility with legacy STL algorithms (but not with the ranges
 *   algorithms).
 *   The position of each pair is kept track by a third sparse dynamic array,
 *   which uses pagination to reduce memory overhead. This array allows for
 *   strictly constant-time search, insertion and removal of elements.
 *
 * @note Although the container is on the surface unordered, internal order is
 *   deterministic and can be manipulated through the iter_swap method.
 * @note Using PageSize = 0 will cause the position array to not be paginated
 *   at all. This has the advantage of slightly accelerating the operations of
 *   the container, though at the cost of significant memory overhead. Consider
 *   using this option if indexes stay low in value.
 */
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
      "heim::index_map<Index, T, PageSize, Alloc>: "
          "detail::index_map_is_index_v<Index>;");
  static_assert(
      detail::index_map_is_mapped_v<T>,
      "heim::index_map<Index, T, PageSize, Alloc>: "
          "detail::index_map_is_mapped_v<T>;");
  static_assert(
      std::is_same_v<
          typename std::allocator_traits<Alloc>::value_type,
          std::pair<Index const, T>>,
      "heim::index_map<Index, T, PageSize, Alloc>: "
          "std::is_same_v<"
              "typename std::allocator_traits<Alloc>::value_type,"
              "std::pair<Index const, T>>;");

public:
  using index_type  = Index;
  using mapped_type = T;

  static constexpr std::size_t
  page_size = PageSize;

  using allocator_type = Alloc;


  using size_type       = std::size_t;
  using difference_type = std::ptrdiff_t;

  using value_type = std::pair<index_type const, mapped_type>;

  using reference       = std::pair<index_type const &, mapped_type &>;
  using const_reference = std::pair<index_type const &, mapped_type const &>;


  /*!
   * @brief The random-access iterator class of the index map.
   *
   * @tparam IsConst Whether the iterator is to be used in a const setting or
   *   not.
   *
   * @warning This iterator dereferences to pairs of references rather than
   *   references to pairs. This makes it incompatible with most of the STL's
   *   legacy algorithms, though still compatible with its ranges algorithms,
   *   due to iterator requirements having been relaxed.
   */
  template<bool IsConst>
  class generic_iterator;

  using iterator       = generic_iterator<false>;
  using const_iterator = generic_iterator<true>;

  using reverse_iterator       = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:
  //! @cond INTERNAL

  using alloc_traits_t
  = std::allocator_traits<allocator_type>;



  using index_alloc_t
  = alloc_traits_t::template rebind_alloc<index_type>;

  using index_alloc_traits_t
  = std::allocator_traits<index_alloc_t>;

  using index_vector_t
  = std::vector<index_type, index_alloc_t>;



  using mapped_alloc_t
  = alloc_traits_t::template rebind_alloc<mapped_type>;

  using mapped_alloc_traits_t
  = std::allocator_traits<mapped_alloc_t>;

  using mapped_vector_t
  = std::vector<mapped_type, mapped_alloc_t>;



  using page_t
  = std::array<size_type, page_size>;

  static constexpr size_type s_null_position
  = std::numeric_limits<size_type>::max();

  using page_alloc_t
  = alloc_traits_t::template rebind_alloc<page_t>;

  using page_alloc_traits_t
  = std::allocator_traits<page_alloc_t>;


  /*!
   * @brief The custom deleter for the page pointers, using the
   *   @code allocator_type@endcode.
   *
   * @details Is in charge of destroying the pointed-to page and of
   *   deallocating its memory.
   */
  class page_deleter;

  using page_uptr_t
  = std::unique_ptr<page_t, page_deleter>;


  constexpr static bool is_paged_v
  = PageSize > 0;

  using position_alloc_t
  = alloc_traits_t::template rebind_alloc<std::conditional_t<
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
  /*!
   * @brief Returns the pointer to a new page for the position vector.
   *
   * @tparam Args The types of the arguments to construct the page.
   * @param args The arguments to construct the page.
   * @returns The newly constructed page pointer.
   * @pre @code PageSize > 0@endcode.
   */
  template<typename ...Args>
  [[nodiscard]]
  constexpr page_uptr_t
  m_make_page_uptr(Args &&...args) const
  requires (is_paged_v)
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

  /*!
   * @brief Creates an empty page pointer.
   *
   * @returns The new empty page pointer.
   * @pre @code PageSize > 0@endcode.
   */
  [[nodiscard]]
  constexpr page_uptr_t
  m_make_page_uptr(std::nullptr_t) const
  requires (is_paged_v)
  {
    page_alloc_t page_alloc{m_allocator};
    return page_uptr_t{nullptr, page_deleter{std::move(page_alloc)}};
  }



  /*!
   * @brief Copies the pages from @code other_pos @endcode into
   *   @code pos@endcode.
   *
   * @param pos       The vector to copy into.
   * @param other_pos The vector to copy from.
   * @pre @code PageSize > 0@endcode.
   */
  constexpr void
  m_copy_position_vector(
      position_vector_t       &pos,
      position_vector_t const &other_pos)
  requires (is_paged_v)
  {
    pos.reserve(pos.size() + other_pos.size());
    for (auto const &page_uptr : other_pos)
    {
      if (page_uptr)
        pos.emplace_back(m_make_page_uptr(*page_uptr));
      else
        pos.emplace_back(m_make_page_uptr(nullptr));
    }
  }


  /*!
   * @brief The elementary move assignment operation of this object.
   *
   * @param other The index_map chose contents to move.
   */
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



  /*!
   * @brief Returns the number of the page for the index @code i@endcode.
   *
   * @param i The index to get the page number of.
   * @returns The number of the page for the index @code i@endcode.
   * @pre @code PageSize > 0@endcode.
   */
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


  /*!
   * @brief Returns the page number (number of the slot in its page) for the
   *   index @code i@endcode.
   *
   * @param i The index to get the line number of.
   * @returns The line number for the index @code i@endcode.
   * @pre @code PageSize > 0@endcode.
   */
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


  /*!
   * @brief Returns a reference to the position in the dense vectors of the
   *   element with the index @code i@endcode.
   *
   * @param i The index of the element to get the position of.
   * @returns The reference to the position in the dense vectors of the element
   *   with the index @code i@endcode.
   */
  constexpr size_type &
  m_position_get(index_type const i)
  noexcept
  {
    if constexpr (is_paged_v)
      return (*m_positions[s_position_page_nb(i)])[s_position_line_nb(i)];
    else
      return m_positions[static_cast<size_type>(i)];
  }


  /*!
   * @brief Returns the position in the dense vectors of the element with the
   *   index @code i@endcode.
   *
   * @param i The index of the element to get the position of.
   * @returns The position in the dense vectors of the element with the index
   *   @code i@endcode.
   */
  constexpr size_type
  m_position_get(index_type const i) const
  noexcept
  {
    if constexpr (is_paged_v)
      return (*m_positions[s_position_page_nb(i)])[s_position_line_nb(i)];
    else
      return m_positions[static_cast<size_type>(i)];
  }


  /*!
   * @brief Checks if the position vector has a position linked to the element
   *   with the index @code i@endcode.
   *
   * @param i The index of the element to check for.
   * @returns @c true if the position vector has a position linked to the
   *   element, @c false otherwise.
   */
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



  /*!
   * @brief Swaps the position in the dense vectors of the elements with
   *   indexes @code i@endcode and @code j@endcode.
   *
   * @param i The index of the first  element to swap.
   * @param j The index of the second element to swap.
   * @pre @code std::is_swappable_v<mapped_type>@endcode.
   */
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



  /*!
   * @brief Sets the capacity of the object to @code new_cap@endcode.
   *
   * @param new_cap The new capacity to reserve memory for.
   */
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
    swap(m_mapped , new_mapped );
  }



  /*!
   * @brief Emplaces at the back of the dense containers a new element,
   *   constructing it in-place using @code i@endcode and @code args@endcode.
   *
   * @tparam Args The type of the arguments to construct the mapped value of
   *   the element.
   * @param i    The index of the element.
   * @param args The arguments to construct the mapped value of the element.
   * @pre @code std::constructible_from<mapped_type, Args ...>@endcode.
   */
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

    // strong exception safety guarantee with pop_back being fully noexcept
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

  //! @endcond

public:
  /*!
   * @brief Default-constructs the index map.
   */
  constexpr
  index_map()
  noexcept(noexcept(allocator_type{}))
    : index_map{allocator_type{}}
  { }

  /*!
   * @brief Constructs the index map using the allocator @code alloc@endcode.
   *
   * @param alloc The allocator to construct the index map with.
   */
  constexpr explicit
  index_map(allocator_type const &alloc)
  noexcept
    : m_allocator{alloc},
      m_positions{position_alloc_t{alloc}},
      m_indexes  {index_alloc_t   {alloc}},
      m_mapped   {mapped_alloc_t  {alloc}}
  { }

  /*!
   * @brief Constructs the index map to be a copy of @code other@endcode.
   *
   * @param other The index map to copy.
   * @pre - @code is_paged_v@endcode.
   * @pre - @code std::is_copy_constructible_v<mapped_type>@endcode.
   */
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

  /*!
   * @brief Constructs the index map to be a copy of @code other@endcode.
   *
   * @param other The index map to copy.
   * @pre - @code !is_paged_v@endcode.
   * @pre - @code std::is_copy_constructible_v<mapped_type>@endcode.
   */
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

  /*!
   * @brief Constructs the index map to be the moved @code other@endcode.
   *
   * @param other The moved index map.
   */
  constexpr
  index_map(index_map &&other)
  noexcept
  = default;

  /*!
   * @brief Constructs the index map to be a copy of @code other@endcode and
   *   using the allocator @code alloc@endcode.
   *
   * @param other The index map to copy.
   * @param alloc The allocator to construct the index map with.
   * @pre - @code is_paged_v@endcode.
   * @pre - @code std::is_copy_constructible_v<mapped_type>@endcode.
   */
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

  /*!
   * @brief Constructs the index map to be a copy of @code other@endcode and
   *   using the allocator @code alloc@endcode.
   *
   * @param other The index map to copy.
   * @param alloc The allocator to construct the index map with.
   * @pre - @code !is_paged_v@endcode.
   * @pre - @code std::is_copy_constructible_v<mapped_type>@endcode.
   */
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

  /*!
   * @brief Constructs the index map to be the moved @code other@endcode and
   *   using the allocator @code alloc@endcode.
   *
   * @param other The moved index map.
   * @param alloc The allocator to construct the index map with.
   */
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

  /*!
   * @brief Constructs the index map using the initializer list
   *   @code ilist@endcode and using the allocator @code alloc@endcode.
   *
   * @param ilist The initializer list to construct the index map with.
   * @param alloc The allocator to construct the index map with.
   * @pre @code std::is_copy_constructible_v<mapped_type>@endcode.
   */
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


  /*!
   * @brief Destroys the index map.
   */
  constexpr
  ~index_map()
  noexcept
  = default;


  /*!
   * @brief Assigns @c *this to be a copy of @code other@endcode.
   *
   * @param other The index map to copy.
   * @returns @c *this .
   * @pre @code std::is_copy_assignable_v<mapped_type>@endcode.
   */
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

  /*!
   * @brief Assigns the index map to be the moved @code other@endcode.
   *
   * @param other The moved index map.
   * @returns @c *this .
   * @pre @code std::is_move_assignable_v<mapped_type>@endcode.
   */
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

  /*!
   * @brief Assigns the index map using the initializer list
   *   @code ilist@endcode.
   *
   * @param ilist The initializer list to assign the index map with.
   * @returns @c *this .
   * @pre @code std::is_copy_assignable_v<mapped_type>@endcode.
   */
  constexpr index_map &
  operator=(std::initializer_list<value_type> ilist)
  requires (std::is_copy_assignable_v<mapped_type>)
  {
    index_map tmp{ilist, m_allocator};
    swap(tmp);

    return *this;
  }


  /*!
   * @brief Swaps the contents of @c *this and @code other@endcode.
   *
   * @param other The other index map whose contents to swap.
   */
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

  /*!
   * @brief Swaps the contents of @code lhs @endcode and @code rhs@endcode.
   *
   * @param lhs The first  index map whose contents to swap.
   * @param rhs The second index map whose contents to swap.
   */
  friend constexpr void
  swap(index_map &lhs, index_map &rhs)
  noexcept(noexcept(lhs.swap(rhs)))
  {
    lhs.swap(rhs);
  }


  /*!
   * @brief Returns the index map's allocator.
   *
   * @returns The index map's allocator.
   */
  [[nodiscard]]
  constexpr allocator_type
  get_allocator() const
  noexcept
  {
    return m_allocator;
  }



  /*!
   * @brief Returns an iterator to the first element in the index map.
   *
   * @returns An iterator to the first element in the index map.
   */
  [[nodiscard]]
  constexpr iterator
  begin()
  noexcept
  {
    return iterator{this, 0};
  }

  /*!
   * @brief Returns a const iterator to the first element in the index map.
   *
   * @returns A const iterator to the first element in the index map.
   */
  [[nodiscard]]
  constexpr const_iterator
  begin() const
  noexcept
  {
    return const_iterator{this, 0};
  }


  /*!
   * @brief Returns an iterator past the last element in the index map.
   *
   * @warning This returned iterator only acts as a sentinel, and is not to be
   *   dereferenced.
   *
   * @returns An iterator past the last element in the index map.
   */
  [[nodiscard]]
  constexpr iterator
  end()
  noexcept
  {
    return iterator{this, size()};
  }

  /*!
   * @brief Returns a const iterator past the last element in the index map.
   *
   * @warning This returned const iterator only acts as a sentinel, and is not
   *   to be dereferenced.
   *
   * @returns A const iterator past the last element in the index map.
   */
  [[nodiscard]]
  constexpr const_iterator
  end() const
  noexcept
  {
    return const_iterator{this, size()};
  }

  /*!
   * @brief Returns a const iterator to the first element in the index map.
   *
   * @returns A const iterator to the first element in the index map.
   */
  [[nodiscard]]
  constexpr const_iterator
  cbegin() const
  noexcept
  {
    return const_iterator{this, 0};
  }


  /*!
   * @brief Returns a const iterator past the last element in the index map.
   *
   * @warning This returned const iterator only acts as a sentinel, and is not
   *   to be dereferenced.
   *
   * @returns A const iterator past the last element in the index map.
   */
  [[nodiscard]]
  constexpr const_iterator
  cend() const
  noexcept
  {
    return const_iterator{this, size()};
  }


  /*!
   * @brief Returns a reverse iterator to the first element of the reversed
   *   index map.
   *
   * @note It corresponds to the last element of the non-reversed index map.
   *
   * @return A reverse iterator to the first element of the reversed index map.
   */
  [[nodiscard]]
  constexpr reverse_iterator
  rbegin()
  noexcept
  {
    return reverse_iterator{end()};
  }

  /*!
   * @brief Returns a const reverse iterator to the last element of the
   *   reversed index map.
   *
   * @note It corresponds to the last element of the non-reversed index map.
   *
   * @return A const reverse iterator to the first element of the reversed
   *   index map.
   */
  [[nodiscard]]
  constexpr const_reverse_iterator
  rbegin() const
  noexcept
  {
    return const_reverse_iterator{end()};
  }


  /*!
   * @brief Returns a reverse iterator past the last element of the reversed
   *   index map.
   *
   * @note It corresponds to before the first element of the non-reversed index
   *   map.
   * @warning This returned reverse iterator only acts as a sentinel, and is
   *   not to be dereferenced.
   *
   * @return A reverse iterator past the last element of the reversed index
   *   map.
   */
  [[nodiscard]]
  constexpr reverse_iterator
  rend()
  noexcept
  {
    return reverse_iterator{begin()};
  }

  /*!
   * @brief Returns a const reverse iterator past the last element of the
   *   reversed index map.
   *
   * @note It corresponds to before the first element of the non-reversed index
   *   map.
   * @warning This returned const reverse iterator only acts as a sentinel, and
   *   is not to be dereferenced.
   *
   * @return A const reverse iterator past the last element of the reversed
   *   index map.
   */
  [[nodiscard]]
  constexpr const_reverse_iterator
  rend() const
  noexcept
  {
    return const_reverse_iterator{begin()};
  }


  /*!
   * @brief Returns a const reverse iterator to the last element of the
   *   reversed index map.
   *
   * @note It corresponds to the last element of the non-reversed index map.
   *
   * @return A const reverse iterator to the first element of the reversed
   *   index map.
   */
  [[nodiscard]]
  constexpr const_reverse_iterator
  crbegin() const
  noexcept
  {
    return const_reverse_iterator{cend()};
  }


  /*!
   * @brief Returns a const reverse iterator past the last element of the
   *   reversed index map.
   *
   * @note It corresponds to before the first element of the non-reversed index
   *   map.
   * @warning This returned const reverse iterator only acts as a sentinel, and
   *   is not to be dereferenced.
   *
   * @return A const reverse iterator past the last element of the reversed
   *   index map.
   */
  [[nodiscard]]
  constexpr const_reverse_iterator
  crend() const
  noexcept
  {
    return const_reverse_iterator{cbegin()};
  }



  /*!
   * @brief Returns the number of elements in the index map.
   *
   * @returns The number of elements in the index map.
   */
  [[nodiscard]]
  constexpr size_type
  size() const
  noexcept
  {
    return m_indexes.size();
  }


  /*!
   * @brief Returns the maximum number of elements that the index map can
   *   contain.
   *
   * @returns The maximum number of elements that the index map can contain.
   */
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


  /*!
   * @brief Checks whether the index map has no elements or not.
   *
   * @returns @c true if the index map has no elements, @c false otherwise.
   */
  [[nodiscard]]
  constexpr bool
  empty() const
  noexcept
  {
    return m_indexes.empty();
  }


  /*!
   * @brief Returns the maximum number of elements the index map can hold
   *   without requiring reallocation.
   *
   * @returns The maximum number of elements the index map can hold without
   *   requiring reallocation.
   */
  [[nodiscard]]
  constexpr size_type
  capacity() const
  noexcept
  {
    return m_indexes.capacity();
  }


  /*!
   * @brief Increases the capacity (the maximum number of elements the index
   *   map can hold without requiring reallocation) to a value that is at least
   *   equal to @code new_cap@endcode.
   *
   * @param new_cap The new capacity of the index map.
   */
  constexpr void
  reserve(size_type const new_cap)
  {
    if (new_cap <= capacity())
      return;

    if (new_cap > max_size())
      throw std::length_error{"heim::index_map::reserve(size_type const)"};

    m_set_capacity(new_cap);
  }


  /*!
   * @brief Request the removal of unused capacity.
   *
   * @details It is a non-binding request to reduce the index map's capacity to
   *   its size. It depends on the implementation whether the request is
   *   fulfilled.
   */
  constexpr void
  shrink_to_fit()
  {
    m_set_capacity(size());

    if constexpr (is_paged_v)
    {
      // for a paginated vector, shrinking means destroying unused pages.
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



  /*!
   * @brief Checks whether an element with the index @code i@endcode is
   *   contained in the index map.
   *
   * @param i The index of the element to check for.
   * @returns @c true if the index map contains an element with the index
   *   @code i@endcode, @c false otherwise.
   */
  [[nodiscard]]
  constexpr bool
  contains(index_type const i) const
  noexcept
  {
    return m_position_contains(i)
        && m_indexes[m_position_get(i)] == i;
  }


  /*!
   * @brief Returns an iterator to the element with the index @code i@endcode,
   *   or if not contained @c end().
   *
   * @param i The index of the element whose iterator to return.
   * @returns An iterator to the element with the index @code i@endcode, or if
   *   not contained @c end().
   */
  [[nodiscard]]
  constexpr iterator
  find(index_type const i)
  noexcept
  {
    if (!contains(i))
      return end();

    return iterator{this, m_position_get(i)};
  }

  /*!
   * @brief Returns a const iterator to the element with the index
   *   @code i@endcode, or if not contained @c end().
   *
   * @param i The index of the element whose const iterator to return.
   * @returns A const iterator to the element with the index @code i@endcode,
   *   or if not contained @c end().
   */
  [[nodiscard]]
  constexpr const_iterator
  find(index_type const i) const
  noexcept
  {
    if (!contains(i))
      return end();

    return const_iterator{this, m_position_get(i)};
  }


  /*!
   * @brief Returns a reference to the value mapped to the index
   *   @code i@endcode.
   *
   * @param i The index to get the mapped value of.
   * @returns A reference to the value mapped to the index @code i@endcode.
   */
  [[nodiscard]]
  constexpr mapped_type &
  operator[](index_type const i)
  noexcept
  {
    return m_mapped[m_position_get(i)];
  }

  /*!
   * @brief Returns a const reference to the value mapped to the index
   *   @code i@endcode.
   *
   * @param i The index to get the mapped value of.
   * @returns A const reference to the value mapped to the index
   *   @code i@endcode.
   */
  [[nodiscard]]
  constexpr mapped_type const &
  operator[](index_type const i) const
  noexcept
  {
    return m_mapped[m_position_get(i)];
  }


  /*!
   * @brief Returns a reference to the value mapped to the index
   *   @code i@endcode.
   *
   * @param i The index to get the mapped value of.
   * @returns A reference to the value mapped to the index @code i@endcode.
   * @exception std::out_of_range if @code !contains(i)@endcode.
   */
  [[nodiscard]]
  constexpr mapped_type &
  at(index_type const i)
  {
    if (!contains(i))
      throw std::out_of_range{"heim::index_map::at(index_type)"};

    return operator[](i);
  }

  /*!
   * @brief Returns a const reference to the value mapped to the index
   *   @code i@endcode.
   *
   * @param i The index to get the mapped value of.
   * @returns A const reference to the value mapped to the index
   *   @code i@endcode.
   * @exception std::out_of_range if @code !contains(i)@endcode.
   */
  [[nodiscard]]
  constexpr mapped_type const &
  at(index_type const i) const
  {
    if (!contains(i))
      throw std::out_of_range{"heim::index_map::at(index_type) const"};

    return operator[](i);
  }



  /*!
   * @brief Tries to emplace a new element, constructing it in-place using
   *   @code i@endcode and @code args@endcode.
   *
   * @tparam Args The type of the arguments to construct the mapped value of
   *   the element.
   * @param i    The index of the element.
   * @param args The arguments to construct the mapped value of the element.
   * @returns A pair containing an iterator to the element with the index
   *   @code i@endcode, and @c true if the element is newly constructed,
   *   @c false otherwise.
   * @pre @code std::constructible_from<mapped_type, Args ...>@endcode.
   */
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


  /*!
   * @brief Tries to emplace a new element, constructing it in-place using
   *   @code i@endcode and @code args@endcode, or if it already exists
   *   reassigning its mapped value to one constructed with @code args@endcode.
   *
   * @tparam Args The type of the arguments to construct the mapped value of
   *   the element.
   * @param i    The index of the element.
   * @param args The arguments to construct the mapped value of the element.
   * @returns A pair containing an iterator to the element with the index
   *   @code i@endcode, and @c true if the element is newly constructed,
   *   @c false if its mapped value has been reassigned.
   * @pre - @code std::constructible_from<mapped_type, Args ...>@endcode.
   * @pre - @code std::is_move_assignable_v<mapped_type>@endcode.
   */
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


  /*!
   * @brief Tries to insert a new element, constructed using @code i@endcode
   *   and @code m@endcode.
   *
   * @param i The index of the new element.
   * @param m The mapped value to copy into the element.
   * @returns A pair containing an iterator to the element with the index
   *   @code i@endcode, and @c true if the element is newly constructed,
   *   @c false otherwise.
   * @pre @code std::is_copy_constructible_v<mapped_type>@endcode.
   */
  constexpr std::pair<iterator, bool>
  insert(index_type const i, mapped_type const &m)
  requires (std::is_copy_constructible_v<mapped_type>)
  {
    return emplace(i, m);
  }

  /*!
   * @brief Tries to insert a new element, constructed using @code i@endcode
   *   and @code m@endcode.
   *
   * @param i The index of the new element.
   * @param m The mapped value to move into the element.
   * @returns A pair containing an iterator to the element with the index
   *   @code i@endcode, and @c true if the element is newly constructed,
   *   @c false otherwise.
   * @pre @code std::is_move_constructible_v<mapped_type>@endcode.
   */
  constexpr std::pair<iterator, bool>
  insert(index_type const i, mapped_type &&m)
  requires (std::is_move_constructible_v<mapped_type>)
  {
    return emplace(i, std::move(m));
  }

  /*!
   * @brief Tries to insert a new element, constructed using
   *   @code value@endcode, a pair representing the element to insert.
   *
   * @param value The pair representing the element to insert.
   * @returns A pair containing an iterator to the element with the index
   *   in @code value@endcode, and @c true if the element is newly constructed,
   *   @c false otherwise.
   * @pre @code std::is_copy_constructible_v<mapped_type>@endcode.
   */
  constexpr std::pair<iterator, bool>
  insert(value_type const &value)
  requires (std::is_copy_constructible_v<mapped_type>)
  {
    return emplace(std::get<0>(value), std::get<1>(value));
  }

  /*!
   * @brief Tries to insert a new element, constructed using
   *   @code value@endcode, a pair representing the element to insert.
   *
   * @param value The pair representing the element to insert.
   * @returns A pair containing an iterator to the element with the index
   *   in @code value@endcode, and @c true if the element is newly constructed,
   *   @c false otherwise.
   * @pre @code std::is_move_constructible_v<mapped_type>@endcode.
   */
  constexpr std::pair<iterator, bool>
  insert(value_type &&value)
  requires (std::is_move_constructible_v<mapped_type>)
  {
    return emplace(std::get<0>(value), std::get<1>(std::move(value)));
  }

  /*!
   * @brief Tries to insert new elements using the pairs in the range
   *   [@code first@endcode, @code last@endcode).
   *
   * @tparam InputIt The type of iterator of the range to construct the new
   *   elements from.
   * @param first The iterator to the first element of the range to construct
   *   the new elements from.
   * @param last  The iterator past the last element of the range to construct
   *   the new elements from.
   * @pre - @code std::is_move_constructible_v<mapped_type>@endcode.
   * @pre - @code std::input_iterator<InputIt>@endcode.
   * @pre - @code std::convertible_to<std::tuple_element_t<0,
   *   std::iter_reference_t<InputIt>>, index_type>@endcode.
   * @pre - @code std::convertible_to< std::tuple_element_t<1,
   *   std::iter_reference_t<InputIt>>, mapped_type>@endcode.
   */
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

  /*!
   * @brief Tries to insert new elements using the initializer list
   *   @code ilist @endcode.
   *
   * @param ilist The initializer list chose elements to construct from.
   * @pre @code std::is_copy_constructible_v<mapped_type>@endcode.
   */
  constexpr void
  insert(std::initializer_list<value_type> ilist)
  requires (std::is_copy_constructible_v<mapped_type>)
  {
    reserve(size() + ilist.size());

    for (value_type const &value : ilist)
      insert(value);
  }


  /*!
   * @brief Tries to insert new elements using the pairs in the range
   *   @code rg @endcode.
   *
   * @tparam R The type of range to construct the new elements from.
   * @param rg The range to construct the new elements from.
   * @pre - @code std::is_move_constructible_v<mapped_type>@endcode.
   * @pre - @code std::ranges::input_range<R>@endcode.
   * @pre - @code std::convertible_to<std::tuple_element_t<0,
   *   std::ranges::range_reference_t<R>>, index_type>@endcode.
   * @pre - @code std::convertible_to<std::tuple_element_t<1,
   *   std::ranges::range_reference_t<R>>, mapped_type>@endcode.
   */
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


  /*!
   * @brief Erases the element with the index @code i@endcode if it is
   *   contained in the index map.
   *
   * @param i The index of the element to erase.
   * @returns @c true if an element has been erased, @c false otherwise.
   * @pre @code std::is_move_assignable_v<mapped_type>@endcode.
   *
   * @details Uses the swap-and-pop method, replacing the erased element with
   *   the last element of the index map.
   */
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

  /*!
   * @brief Erases the element dereferenced by @code pos@endcode.
   *
   * @param pos The iterator to the element to erase.
   * @returns The iterator to the element taking the erased element's place in
   *   the index map, or @c end() if it was the last.
   * @pre @code std::is_move_assignable_v<mapped_type>@endcode.
   */
  constexpr iterator
  erase(iterator pos)
  noexcept(noexcept(erase(std::declval<index_type>())))
  requires (std::is_move_assignable_v<mapped_type>)
  {
    erase(std::get<0>(*pos));
    return pos;
  }

  /*!
   * @brief Erases the element dereferenced by @code pos@endcode.
   *
   * @param pos The const iterator to the element to erase.
   * @returns The iterator to the element taking the erased element's place in
   *   the index map, or @c end() if it was the last.
   * @pre @code std::is_move_assignable_v<mapped_type>@endcode.
   */
  constexpr iterator
  erase(const_iterator pos)
  noexcept(noexcept(erase(std::declval<iterator>())))
  requires (std::is_move_assignable_v<mapped_type>)
  {
    return erase(begin() + (pos - cbegin()));
  }

  /*!
   * @brief Erases the elements dereferenced by the iterators in the range
   *   [@code pos@endcode, @code last@endcode).
   *
   * @param first The iterator to the first element to erase.
   * @param last  The iterator past the last element to erase.
   * @returns The iterator to the element taking the first erased element's
   *   place in the index map, or @c end() if it was the last.
   * @pre @code std::is_move_assignable_v<mapped_type>@endcode.
   *
   */
  constexpr iterator
  erase(iterator first, iterator last)
  noexcept(noexcept(erase(std::declval<iterator>())))
  requires (std::is_move_assignable_v<mapped_type>)
  {
    // The swap-and-pop behaviour in the erasure process messes with the order
    // of elements, which is avoided backwards
    while (last != first)
      last = erase(--last);

    return last;
  }

  /*!
   * @brief Erases the elements dereferenced by the const iterators in the
   *   range [@code pos@endcode, @code last@endcode).
   *
   * @param first The const iterator to the first element to erase.
   * @param last  The const iterator past the last element to erase.
   * @returns The iterator to the element taking the first erased element's
   *   place in the index map, or @c end() if it was the last.
   * @pre @code std::is_move_assignable_v<mapped_type>@endcode.
   */
  constexpr iterator
  erase(const_iterator first, const_iterator last)
  noexcept(noexcept(erase(std::declval<iterator>(), std::declval<iterator>())))
  requires (std::is_move_assignable_v<mapped_type>)
  {
    return erase(
        begin() + (first - cbegin()),
        begin() + (last  - cbegin()));
  }


  /*!
   * @brief Erases all elements that satisfies @code pred@endcode from
   *   @code c@endcode.
   *
   * @tparam Pred The type of the predicate to use.
   * @param c    The index map whose elements to erase.
   * @param pred The predicate used to erase the elements.
   * @returns The number of erased elements.
   * @pre @code std::is_move_assignable_v<mapped_type>@endcode.
   */
  template<typename Pred>
  friend constexpr size_type
  erase_if(index_map &c, Pred pred)
  noexcept(
      noexcept(pred(std::declval<reference>()))
   && noexcept(c.erase(std::declval<index_type const>())))
  requires (std::is_move_assignable_v<mapped_type>)
  {
    size_type old_size = c.size();

    // The swap-and-pop behaviour in the erasure process messes with the order
    // of elements, which is avoided backwards
    for (auto it = c.rbegin(); it != c.rend(); ++it)
    {
      if (pred(*it))
        c.erase(std::get<0>(*it));
    }

    return old_size - c.size();
  }


  /*!
   * @brief Clears all elements from the index map.
   */
  constexpr void
  clear()
  noexcept
  {
    // clearing the position vector means setting all existing positions to the
    // null position
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



  /*!
   * @brief Checks if @code lhs@endcode contains the same elements as
   *   @code rhs@endcode.
   *
   * @param lhs The first  index map to check the equality of.
   * @param rhs The second index map to check the equality of.
   * @returns @c true if the index maps contain all same elements, @c false
   *   otherwise.
   */
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
  /*!
   * @brief Default-constructs the iterator.
   */
  constexpr
  generic_iterator()
  = default;

  /*!
   * @brief Constructs the iterator to be a copy of @code other@endcode.
   *
   * @param other The iterator to copy from.
   */
  constexpr
  generic_iterator(generic_iterator const &other)
  = default;

  /*!
   * @brief Constructs the iterator to be the moved @code other@endcode into.
   *
   * @param other The moved iterator.
   */
  constexpr
  generic_iterator(generic_iterator &&other)
  noexcept
  = default;

  /*!
   * @brief Constructs the iterator to be a copy of the not-const iterator
   *   @code other@endcode.
   *
   * @param other The non-const iterator to copy from.
   * @pre IsConst.
   */
  constexpr explicit
  generic_iterator(generic_iterator<!is_const> other)
  requires (is_const)
    : m_map{other.m_map},
      m_index {other.m_index},
      m_mapped{other.m_mapped}
  { }

  /*!
   * @brief Constructs the iterator by using its index map @code map@endcode
   *   and its position @code pos@endcode.
   *
   * @param map The index map whose elements to iterate over.
   * @param pos The position in its index map.
   */
  constexpr
  generic_iterator(maybe_const_t<index_map> * const map, size_type const pos)
    : m_map{map},
      m_index {map->m_indexes.data() + pos},
      m_mapped{map->m_mapped .data() + pos}
  { }


  /*!
   * @brief The iterator's destructor.
   */
  constexpr
  ~generic_iterator()
  noexcept
  = default;


  /*!
   * @brief Assigns the iterator to be a copy of @code other@endcode.
   *
   * @param other The iterator to copy from.
   * @returns @c *this .
   */
  constexpr generic_iterator &
  operator=(generic_iterator const &other)
  = default;

  /*!
   * @brief Assigns the iterator to be the moved @code other@endcode into.
   *
   * @param other The moved iterator.
   * @returns @c *this .
   */
  constexpr generic_iterator &
  operator=(generic_iterator &&other)
  noexcept
  = default;


  /*!
   * @brief Swaps the contents of @c *this and @code other@endcode.
   *
   * @param other The other iterator whose contents to swap.
   */
  constexpr void
  swap(generic_iterator &other)
  noexcept
  {
    using std::swap;

    swap(m_map   , other.m_map);
    swap(m_index , other.m_index);
    swap(m_mapped, other.m_mapped);
  }

  /*!
   * @brief Swaps the contents of @code lhs @endcode and @code rhs@endcode.
   *
   * @param lhs The first  iterator whose contents to swap.
   * @param rhs The second iterator whose contents to swap.
   */
  friend constexpr void
  swap(generic_iterator &lhs, generic_iterator &rhs)
  noexcept(noexcept(lhs.swap(rhs)))
  {
    lhs.swap(rhs);
  }



  /*!
   * @brief Dereferences the iterator to the pointed element.
   *
   * @returns A pair of references to the element's index and mapped value.
   */
  constexpr reference
  operator*() const
  noexcept
  {
    return reference{*m_index, *m_mapped};
  }



  /*!
   * @brief Returns @c *this moved forward once.
   *
   * @returns @c *this moved forward once.
   */
  constexpr generic_iterator &
  operator++()
  noexcept
  {
    ++m_index;
    ++m_mapped;
    return *this;
  }

  /*!
   * @brief Returns a copy of @c *this before being moved forward once.
   *
   * @returns A copy of @c *this before being moved forward once.
   */
  constexpr generic_iterator
  operator++(int)
  noexcept
  {
    auto tmp = *this;
    ++*this;
    return tmp;
  }


  /*!
   * @brief Returns @c *this moved backwards once.
   *
   * @returns @c *this moved backwards once.
   */
  constexpr generic_iterator &
  operator--()
  noexcept
  {
    --m_index;
    --m_mapped;
    return *this;
  }

  /*!
   * @brief Returns a copy of @c *this before being moved forward once.
   *
   * @returns A copy of @c *this before being moved forward once.
   */
  constexpr generic_iterator
  operator--(int)
  noexcept
  {
    auto tmp = *this;
    --*this;
    return tmp;
  }


  /*!
   * @brief Returns @c *this moved forward @code n@endcode times.
   *
   * @param n The number of times to move @c *this forward.
   * @returns @c *this moved forward @code n@endcode times.
   */
  constexpr generic_iterator &
  operator+=(difference_type const n)
  noexcept
  {
    m_index  += n;
    m_mapped += n;
    return *this;
  }


  /*!
   * @brief Returns @c *this moved backwards @code n@endcode times.
   *
   * @param n The number of times to move @c *this backwards.
   * @returns @c *this moved backwards @code n@endcode times.
   */
  constexpr generic_iterator &
  operator-=(difference_type const n)
  noexcept
  {
    m_index  -= n;
    m_mapped -= n;
    return *this;
  }


  /*!
   * @brief Returns a copy of @code it@endcode before being moved forward
   *   @code n@endcode times.
   *
   * @param it The iterator to move forward.
   * @param n  The number of times to move @code it@endcode forward.
   * @returns A copy of @code it@endcode before being moved forward
   *   @code n@endcode times.
   */
  friend constexpr generic_iterator
  operator+(generic_iterator const &it, difference_type const n)
  noexcept
  {
    generic_iterator r{it};
    r += n;
    return r;
  }

  /*!
   * @brief Returns a copy of @code it@endcode before being moved forward
   *   @code n@endcode times.
   *
   * @param n  The number of times to move @code it@endcode forward.
   * @param it The iterator to move forward.
   * @returns A copy of @code it@endcode before being moved forward
   *   @code n@endcode times.
   */
  friend constexpr generic_iterator
  operator+(difference_type const n, generic_iterator const &it)
  noexcept
  {
    generic_iterator r{it};
    r += n;
    return r;
  }


  /*!
   * @brief Returns a copy of @code it@endcode before being moved backwards
   *   @code n@endcode times.
   *
   * @param it The iterator to move backwards.
   * @param n  The number of times to move @code it@endcode backwards.
   * @returns A copy of @code it@endcode before being moved backwards
   *   @code n@endcode times.
   */
  friend constexpr generic_iterator
  operator-(generic_iterator const &it, difference_type const n)
  noexcept
  {
    generic_iterator r{it};
    r -= n;
    return r;
  }

  /*!
   * @brief Returns the signed distance between @code lhs@endcode and
   *   @code lhs@endcode.
   *
   * @param lhs The first  iterator to calculate the distance of.
   * @param rhs The second iterator to calculate the distance of.
   * @returns The signed distance between @code lhs@endcode and
   *   @code lhs@endcode.
   */
  friend constexpr difference_type
  operator-(generic_iterator const &lhs, generic_iterator const &rhs)
  noexcept
  {
    return lhs.m_index - rhs.m_index;
  }



  /*!
   * @brief Dereferences the iterator to the pointed element @code n@endcode
   *   times forward.
   *
   * @returns A pair of references to the element's index and mapped value.
   */
  constexpr reference
  operator[](difference_type const n) const
  noexcept
  {
    return *(*this + n);
  }



  /*!
   * @brief Checks if @code lhs@endcode and @code rhs@endcode point to the same
   *   element.
   *
   * @param lhs The first  iterator to check the equality of.
   * @param rhs The second iterator to check the equality of.
   * @returns @c true if the iterators are equal, @c false otherwise.
   */
  friend constexpr bool
  operator==(generic_iterator const &lhs, generic_iterator const &rhs)
  noexcept
  {
    return lhs.m_index == rhs.m_index;
  }


  /*!
   * @brief Compares @code lhs@endcode and @code rhs@endcode.
   *
   * @param lhs The first  iterator to compare.
   * @param rhs The second iterator to compare.
   * @returns The three-way comparison of @code lhs@endcode and
   *   @code rhs@endcode.
   */
  friend constexpr auto
  operator<=>(generic_iterator const &lhs, generic_iterator const &rhs)
  noexcept
  {
    return lhs.m_index <=> rhs.m_index;
  }



  /*!
   * @brief Returns a pair of rvalue references to @code it@endcode 's pointed
   *   element's index and mapped value.
   *
   * @param it The iterator whose pointed element to move.
   * @returns A pair of rvalue references to @code it@endcode 's pointed
   *   element's index and mapped value.
   */
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


  /*!
   * @brief Swaps the pointed elements of @code lhs@endcode and
   *   @code rhs@endcode.
   *
   * @param lhs The first  iterator whose pointed element to swap.
   * @param rhs The second iterator whose pointed element to swap.
   */
  friend constexpr void
  iter_swap(generic_iterator lhs, generic_iterator rhs)
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
  /*!
   * @brief Default-constructs the deleter.
   */
  constexpr
  page_deleter()
    : page_deleter{page_alloc_t{}}
  { }

  /*!
   * @brief Constructs the deleter using @code alloc@endcode.
   *
   * @param alloc The page allocator to use.
   */
  constexpr explicit
  page_deleter(page_alloc_t alloc)
    : m_allocator{std::move(alloc)}
  { }


  /*!
   * @brief Destroys the deleter.
   */
  constexpr
  ~page_deleter()
  noexcept
  = default;



  /*!
   * @brief Deletes the page pointed to by @code ptr@endcode.
   *
   * @param ptr The pointer to the page to delete.
   *
   * @details Destroys the page and deallocates its used memory using the
   *   special page allocator.
   */
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


} // namespace heim

#endif // HEIM_LIB_INDEX_MAP_HPP
