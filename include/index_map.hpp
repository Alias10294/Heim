#ifndef HEIM_INDEX_MAP_HPP
#define HEIM_INDEX_MAP_HPP

#include <array>
#include <cstddef>
#include <limits>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace heim
{
namespace internal
{
template<typename T>
struct index_map_is_index
{
  constexpr
  static bool value
  =  !std::is_const_v   <T> &&
     !std::is_volatile_v<T> &&
      std::unsigned_integral<T>;

};

template<typename T>
constexpr
inline bool index_map_is_index_v
= index_map_is_index<T>::value;



template<typename T>
struct index_map_is_value
{
  constexpr
  static bool value
  = !std::is_const_v   <T> &&
    !std::is_volatile_v<T> &&
     std::is_object_v  <T> &&
     std::is_nothrow_constructible_v     <T> &&
     std::is_nothrow_move_constructible_v<T> &&
     std::is_nothrow_move_assignable_v   <T> &&
     std::is_nothrow_destructible_v      <T>;

};

template<typename T>
constexpr
inline bool index_map_is_value_v
= index_map_is_value<T>::value;



template<typename T, typename Value>
struct index_map_is_value_allocator
{
  static_assert(
      index_map_is_value_v<Value>,
      "heim::internal::index_map_is_value_allocator: Value must satisfy: "
          "index_map_is_value_v<Value>.");

  constexpr
  static bool value
  = std::is_same_v<
      typename std::allocator_traits<T>::value_type,
      Value>;

};

template<typename T, typename Value>
constexpr
inline bool index_map_is_value_allocator_v
= index_map_is_value_allocator<T, Value>::value;


}


/*!
 * @brief The associative container for indexes and values of Heim.
 *
 * @tparam Index The type of indexes (indexes) to hold.
 * @tparam Value The type of values (values) to hold.
 * @tparam PageSize The size of each page of indexes.
 * @tparam Allocator The type of allocator for values.
 *
 * @details Implements a customised sparse set, providing constant-time
 *   complexity on insertion, deletion and search of element. Uses pagination
 *   to limit memory usage on the sparse array.
 */
template<
    typename    Index,
    typename    Value,
    std::size_t PageSize,
    typename    Allocator>
class index_map
{
  static_assert(
      internal::index_map_is_index_v<Index>,
      "heim::index_map: Index must satisfy: "
          "internal::index_map_is_index_v<Index> ");
  static_assert(
      internal::index_map_is_value_v<Value>,
      "heim::index_map: Value must satisfy: "
          "internal::index_map_is_value_v<Value> ");
  static_assert(
      internal::index_map_is_value_allocator_v<Allocator, Value>,
      "heim::index_map: Allocator must satisfy: "
          "internal::index_map_is_value_allocator_v<Allocator, Value> ");

public:
  //! @brief The size integer type.
  using size_type
  = std::size_t;



  //! @brief The type of indexes.
  using index_type
  = Index;

  //! @brief The type of values.
  using value_type
  = Value;

  //! @brief The size of each page of indexes.
  constexpr
  static size_type page_size
  = PageSize;

  //! @brief The type of allocator for values.
  using value_allocator_type
  = Allocator;

  //! @cond INTERNAL
private:
  /*!
   * @brief The iterator type of the index_map.
   *
   * @tparam IsConst Whether the iterator is to be used in a const setting or
   *   not.
   *
   * @details Because of the data structure of the index_map, this iterator
   *   dereferences to a proxy type (not the real pair of elements) and thus
   *   can not comply with the iterator requirements of the STL.
   *   As of this version, this iterator's implementation is the closest to one
   *   of a simple input iterator, but will be improved in the future.
   */
  template<bool IsConst>
  class generic_iterator
  {
  public:
    //! @brief Whether the iterator is to be used in a const setting or not.
    constexpr
    static bool is_const
        = IsConst;


    //! @brief The type of indexes.
    using index_type
    = Index;

    //! @brief The type of values.
    using value_type
    = std::conditional_t<
        is_const,
        Value const,
        Value>;



    //! @brief The type of the proxy used.
    using proxy
    = std::tuple<index_type const &, value_type &>;

  private:
    index_type const *indexes_;
    value_type       *values_;

  public:
    constexpr
    generic_iterator()
    = default;

    constexpr
    generic_iterator(generic_iterator const &)
    = default;

    constexpr
    generic_iterator(generic_iterator &&)
    noexcept
    = default;

    constexpr
    generic_iterator(
        index_type const *indexes,
        value_type       *values)
      : indexes_{indexes},
        values_ {values}
    { }


    constexpr
    ~generic_iterator()
    noexcept
    = default;


    constexpr
    generic_iterator &operator=(generic_iterator const &)
    = default;

    constexpr
    generic_iterator &operator=(generic_iterator &&)
    noexcept
    = default;



    constexpr
    proxy operator*() const
    noexcept
    {
      return proxy{
          *indexes_,
          *values_};
    }



    constexpr
    generic_iterator &operator++()
    noexcept
    {
      ++indexes_;
      ++values_;

      return *this;
    }



    constexpr
    bool operator==(generic_iterator const &other) const
    noexcept
    {
      return indexes_ == other.indexes_;
    }

  };

  //! @endcond

public:
  //! @brief The regular iterator type.
  using iterator       = generic_iterator<false>;
  //! @brief The const iterator type.
  using const_iterator = generic_iterator<true>;

  //! @brief The proxy type of the regular iterator.
  using proxy       = typename iterator::proxy;
  //! @brief The proxy type of the const iterator.
  using const_proxy = typename const_iterator::proxy;

  //! @cond INTERNAL
private:
  //! The type of the pages.
  using page_type
  = std::array<size_type, page_size>;

  //! The position to represent a non-existing index.
  constexpr
  static size_type null_position_
  = std::numeric_limits<size_type>::max();

  //! @endcond
  //! @cond INTERNAL
private:
  std::vector<std::unique_ptr<page_type>> pages_;

  std::vector<index_type>                       indexes_;
  std::vector<value_type, value_allocator_type> values_;



  /*!
   * @brief Checks if the given page contains no valid positions.
   *
   * @param page The page to check for positions.
   * @returns @c true if the given page contains no valid positions, @c false
   *   otherwise.
   */
  constexpr
  static bool is_blank(page_type const &page)
  noexcept
  {
    for (auto const pos : page)
    {
      if (pos != null_position_)
        return false;
    }
    return true;
  }


  /*!
   * @brief The page number of the given index.
   *
   * @param idx The index to get the page number of.
   * @returns The page number of the given index.
   */
  constexpr
  static size_type page_number(index_type const idx)
  noexcept
  {
    return idx / page_size;
  }


  /*!
   * @brief The line number (the index in its page) of the given index.
   *
   * @param idx The index to get the line number of.
   * @returns The line number of the given index.
   */
  constexpr
  static size_type line_number(index_type const idx)
  noexcept
  {
    return idx % page_size;
  }


  /*!
   * @brief The position in the dense containers of the given index.
   *
   * @param idx The index to get the position of.
   * @returns The position of the given index.
   */
  constexpr
  size_type  position(index_type const idx) const
  noexcept
  {
    return (*pages_[page_number(idx)])[line_number(idx)];
  }

  /*!
   * @brief The position in the dense containers of the given index.
   *
   * @param idx The index to get the position of.
   * @returns The position of the given index.
   */
  constexpr
  size_type &position(index_type const idx)
  noexcept
  {
    return (*pages_[page_number(idx)])[line_number(idx)];
  }

  //! @endcond

public:
  constexpr
  index_map()
  = default;

  constexpr
  index_map(index_map const &other)
    : pages_  {},
      indexes_{other.indexes_},
      values_ {other.values_}
  {
    pages_.reserve(other.pages_.capacity());
    for (auto const &page_uptr : other.pages_)
    {
      if (page_uptr)
        pages_.emplace_back(std::make_unique<page_type>(*page_uptr));
      else
        pages_.emplace_back(nullptr);
    }
  }

  constexpr
  index_map(index_map &&other)
  noexcept
  = default;


  constexpr
  ~index_map()
  noexcept
  = default;


  constexpr
  index_map &operator=(index_map const &other)
  {
    indexes_ = other.indexes_;
    values_  = other.values_;

    std::vector<std::unique_ptr<page_type>> pages;

    pages.reserve(other.pages_.capacity());
    for (auto const &page_uptr : other.pages_)
    {
      if (page_uptr)
        pages.emplace_back(std::make_unique<page_type>(*page_uptr));
      else
        pages.emplace_back(nullptr);
    }
    pages_ = std::move(pages);

    return *this;
  }

  constexpr
  index_map &operator=(index_map &&other)
  noexcept
  = default;



  /*!
   * @brief The number of pairs in the index_map.
   *
   * @returns The number of pairs in the index_map.
   */
  [[nodiscard]]
  constexpr
  size_type size() const
  noexcept
  {
    return indexes_.size();
  }


  /*!
   * @brief Checks whether the index_map is empty or not.
   *
   * @returns @c true if the index_map is empty, @c false otherwise.
   */
  [[nodiscard]]
  constexpr
  bool empty() const
  noexcept
  {
    return indexes_.empty();
  }


  /*!
   * @brief The maximum number of pairs the index_map can contain.
   *
   * @returns The maximum number of pairs the index_map can contain.
   */
  [[nodiscard]]
  constexpr
  size_type max_size() const
  noexcept
  {
    return indexes_.max_size();
  }


  /*!
   * @brief The maximum number of pairs the index_map can contain without
   *   requiring a reallocation.
   *
   * @returns The maximum number of pairs the index_map can contain without
   *   requiring a reallocation.
   */
  [[nodiscard]]
  constexpr
  size_type capacity() const
  noexcept
  {
    return indexes_.capacity();
  }


  /*!
   * @brief Increases the capacity of the index_map to at least the given
   *   number, reallocating memory if needed.
   *
   * @param n The capacity to reach.
   */
  constexpr
  void reserve(size_type const n)
  {
    indexes_.reserve(n);
    values_ .reserve(n);
  }


  /*!
   * @brief Shrinks the capacity of the index_map to its size, reallocating
   *   memory if needed.
   */
  constexpr
  void shrink_to_fit()
  {
    indexes_.shrink_to_fit();

    try
    {
      values_.shrink_to_fit();
    }
    catch(...)
    {
      indexes_.reserve(values_.capacity());
      throw;
    }

    for (auto &page_uptr : pages_)
    {
      if (page_uptr && is_blank(*page_uptr))
        page_uptr.reset();
    }
    while (!pages_.empty() && !pages_.back())
      pages_.pop_back();
  }



  /*!
   * @brief The iterator to the first pair of the index_map.
   *
   * @returns The iterator to the first pair of the index_map.
   */
  [[nodiscard]]
  constexpr
  iterator       begin()
  noexcept
  {
    return iterator{
        indexes_.data(),
        values_ .data()};
  }

  /*!
   * @brief The const iterator to the first pair of the index_map.
   *
   * @returns The const iterator to the first pair of the index_map.
   */
  [[nodiscard]]
  constexpr
  const_iterator begin() const
  noexcept
  {
    return const_iterator{
        indexes_.data(),
        values_ .data()};
  }


  /*!
   * @brief The iterator to past the last pair of the index_map.
   * @warning This iterator is not to be dereferenced.
   *
   * @returns The iterator to past the last pair of the index_map.
   */
  [[nodiscard]]
  constexpr
  iterator       end()
  noexcept
  {
    return iterator{
      indexes_.data() + size(),
      values_ .data() + size()};
  }

  /*!
   * @brief The const iterator to past the last pair of the index_map.
   * @warning This iterator is not to be dereferenced.
   *
   * @returns The const iterator to past the last pair of the index_map.
   */
  [[nodiscard]]
  constexpr
  const_iterator end() const
  noexcept
  {
    return const_iterator{
      indexes_.data() + size(),
      values_ .data() + size()};
  }


  /*!
   * @brief The const iterator to the first pair of the index_map.
   *
   * @returns The const iterator to the first pair of the index_map.
   */
  [[nodiscard]]
  constexpr
  const_iterator cbegin() const
  noexcept
  {
    return const_iterator{
      indexes_.data(),
      values_ .data()};
  }


  /*!
   * @brief The const iterator to past the last pair of the index_map.
   * @warning This iterator is not to be dereferenced.
   *
   * @returns The const iterator to past the last pair of the index_map.
   */
  [[nodiscard]]
  constexpr
  const_iterator cend() const
  noexcept
  {
    return const_iterator{
      indexes_.data() + size(),
      values_ .data() + size()};
  }



  /*!
   * @brief Checks if the given index is in the index_map.
   *
   * @param idx The index to check for.
   * @returns @c true if the given index is in the index_map, @c false
   *   otherwise.
   */
  [[nodiscard]]
  constexpr
  bool contains(index_type const idx) const
  noexcept
  {
    return page_number(idx) < pages_.size()
        && pages_[page_number(idx)]
        && position(idx) != null_position_
        && indexes_[position(idx)] == idx;
  }


  /*!
   * @brief The iterator to the given index, or end() if the index is not
   *   in the index_map.
   *
   * @param idx The index to find.
   * @returns The iterator to the given index, or end() if the index is not
   *   in the index_map.
   */
  [[nodiscard]]
  constexpr
  iterator       find(index_type const idx)
  noexcept
  {
    if (!contains(idx))
      return end();

    return iterator{
        indexes_.data() + position(idx),
        values_ .data() + position(idx)};
  }

  /*!
   * @brief The const iterator to the given index, or end() if the index is
   *   not in the index_map.
   *
   * @param idx The index to find.
   * @returns The const iterator to the given index, or end() if the index is
   *   not in the index_map.
   */
  [[nodiscard]]
  constexpr
  const_iterator find(index_type const idx) const
  noexcept
  {
    if (!contains(idx))
      return end();

    return const_iterator{
      indexes_.data() + position(idx),
      values_ .data() + position(idx)};
  }


  /*!
   * @brief The value paired to the given index.
   *
   * @param idx The index to get the paired value of.
   * @returns The paired value of the given index.
   */
  [[nodiscard]]
  constexpr
  value_type       &operator[](index_type const idx)
  noexcept
  {
    return values_[position(idx)];
  }

  /*!
   * @brief The value paired to the given index.
   *
   * @param idx The index to get the paired value of.
   * @returns The paired value of the given index.
   */
  [[nodiscard]]
  constexpr
  value_type const &operator[](index_type const idx) const
  noexcept
  {
    return values_[position(idx)];
  }


  /*!
   * @brief The value paired to the given index.
   *
   * @param idx The index to get the paired value of.
   * @returns The paired value of the given index.
   * @exception std::out_of_range if the index is not in the index_map.
   */
  [[nodiscard]]
  constexpr
  value_type       &at(index_type const idx)
  {
    if (!contains(idx))
    {
      throw std::out_of_range{
          "heim::index_map::at(index_type): out_of_range"};
    }
    return values_[position(idx)];
  }

  /*!
   * @brief The value paired to the given index.
   *
   * @param idx The index to get the paired value of.
   * @returns The paired value of the given index.
   * @exception std::out_of_range if the index is not in the index_map.
   */
  [[nodiscard]]
  constexpr
  value_type const &at(index_type const idx) const
  {
    if (!contains(idx))
    {
      throw std::out_of_range{
        "heim::index_map::at(index_type): out_of_range"};
    }
    return values_[position(idx)];
  }


  /*!
   * @brief Clears all indexes and erases all positions from its pages.
   */
  constexpr
  void clear()
  noexcept
  {
    for (auto &page_uptr : pages_)
    {
      if (page_uptr)
        page_uptr->fill(null_position_);
    }

    indexes_.clear();
    values_ .clear();
  }


  /*!
   * @brief Emplaces at the back of the index_map the given index, with
   *   its paired value constructed using the given arguments.
   *
   * @param idx The index to emplace a value for.
   * @param args The arguments to construct the value.
   */
  template<typename ...Args>
  constexpr
  void emplace_back(index_type const idx, Args &&...args)
  {
    if (contains(idx))
      return;

    if (page_number(idx) >= pages_.size())
      pages_.resize(page_number(idx) + 1);
    if (!pages_[page_number(idx)])
    {
      pages_[page_number(idx)] = std::make_unique<page_type>();
      pages_[page_number(idx)] ->fill(null_position_);
    }

    values_.emplace_back(std::forward<Args>(args)...);
    try
    {
      indexes_.emplace_back(idx);
    }
    catch (...)
    {
      values_.pop_back();
      throw;
    }
    position(idx) = size() - 1;
  }


  /*!
   * @brief Removes the given index and its paired value from the index_map,
   *   using the swap-and-pop method.
   *
   * @param idx The index to remove.
   */
  constexpr
  void pop_swap(index_type const idx)
  noexcept
  {
    if (!contains(idx))
      return;

    if (idx != indexes_.back())
    {
      index_type const e_back = indexes_.back();

      indexes_[position(idx)] = e_back;
      values_ [position(idx)] = std::move(values_.back());

      position(e_back) = position(idx);
    }

    indexes_.pop_back();
    values_ .pop_back();

    position(idx) = null_position_;
  }


  /*!
   * @brief Swaps the contents of @c *this and the given index_map.
   *
   * @param other The other index_map whose contents to swap.
   */
  constexpr
  void swap(index_map &other)
  noexcept
  {
    pages_  .swap(other.pages_);
    indexes_.swap(other.indexes_);
    values_ .swap(other.values_);
  }

  /*!
   * @brief Swaps the given indexes and their paired value.
   *
   * @param idxa The first index whose place to swap.
   * @param idxb The second index whose place to swap.
   */
  constexpr
  void swap(index_type const idxa, index_type const idxb)
  {
    if (!contains(idxa) || !contains(idxb))
      return;

    std::swap(indexes_[position(idxa)], indexes_[position(idxb)]);
    std::swap(values_ [position(idxa)], values_ [position(idxb)]);

    std::swap(position(idxa), position(idxb));
  }

};

/*!
 * @brief Swaps the contents of the given index_maps.
 *
 * @param lhs The first index_map whose contents to swap.
 * @param rhs The second index_map whose contents to swap.
 */
template<
    typename    Index,
    typename    Value,
    std::size_t PageSize,
    typename    Allocator>
constexpr
void swap(
    index_map<Index, Value, PageSize, Allocator> &lhs,
    index_map<Index, Value, PageSize, Allocator> &rhs)
noexcept
{
  lhs.swap(rhs);
}


namespace detail
{
template<typename>
struct is_index_map
  : std::false_type
{ };

template<
    typename    Index,
    typename    Value,
    std::size_t PageSize,
    typename    Allocator>
struct is_index_map<
    index_map<Index, Value, PageSize, Allocator>>
  : std::true_type
{ };


}


template<typename T>
struct is_index_map
{
  constexpr
  static bool value
  = detail::is_index_map<std::remove_cvref_t<T>>::value;

};

template<typename T>
constexpr
inline bool is_index_map_v
= is_index_map<T>::value;


}

#endif // HEIM_INDEX_MAP_HPP
