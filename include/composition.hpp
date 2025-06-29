#ifndef HEIM_COMPOSITION_HPP
#define HEIM_COMPOSITION_HPP

#include <algorithm>
#include <array>
#include <functional>
#include <iterator>
#include <numeric>
#include <vector>


#include "entity.hpp"

namespace heim
{
/**
 * @brief The base class for the %composition type.
 *
 * Made to hold without type-erasure the specialized compositions.
 */
class basic_composition
{
public:
  virtual ~basic_composition() = default;

};


/**
 * @brief An optimised associative container for the storing and access of
 * components.
 *
 * Bases its implementation off of sparse sets, providing great
 * cache-friendliness and promising O(1) insertion, deletion and access to
 * components.
 * The sparse array is here optimised through pagination, avoiding critical
 * memory overhead.
 *
 * @tparam Component The component type to hold.
 */
template<typename Component>
class composition final : public basic_composition
{
  static_assert(std::is_move_constructible_v<Component>,
      "heim::composition: Component must be MoveConstructible");
  static_assert(std::is_move_assignable_v<Component>,
      "heim::composition: Component must be MoveAssignable");

public:
  using entity_type     = entity;
  using component_type  = Component;
  using size_type       = std::size_t;
  using difference_type = std::ptrdiff_t;

  /// @brief A placeholder for null ids in the sparse vector.
  static constexpr size_type null_idx =
      std::numeric_limits<size_type>::max();
  /// @brief The size of a sparse array page.
  static constexpr size_type page_size = 4096;

  using component_pointer       = component_type*;
  using const_component_pointer = const component_type*;

  using component_reference       = component_type&;
  using const_component_reference = const component_type&;




  /**
   * @brief The iterator for the %composition type.
   *
   * Allows the use of the %composition with range-based for-loops and the use
   * of most of the STL algorithms that make sense, including ranges.
   *
   * @tparam Composition The %composition type (const or non-const)
   */
  template<typename Composition>
  class basic_iterator
  {
  public:
    /**
     * @brief The exposed proxy type that allows map-like behaviour for the
     * iterator.
     */
    struct proxy
    {
      entity_type         entity;
      component_reference component;
    };

    /**
     * @brief The special proxy type for the -> operator.
     *
     * Exploits the "recursiveness" of the -> operator to keep stability on the
     * proxy.
     */
    class arrow_proxy
    {
    public:
      explicit arrow_proxy(proxy p)
          noexcept :
        proxy_{std::move(p)}
      {}


      proxy* operator->()
      { return std::addressof(proxy_); }

    private:
      proxy proxy_;

    };


    using iterator_category = std::random_access_iterator_tag;
    using value_type        = proxy;
    using size_type         = std::size_t;
    using difference_type   = std::ptrdiff_t;
    using pointer           = arrow_proxy;
    using reference         = value_type;

    using composition_type  = Composition;



    explicit constexpr basic_iterator(Composition* c, const std::size_t idx)
      noexcept :
      composition_(c),
      idx_(idx)
    { }



    constexpr reference operator*() const
    {
      return proxy
      { composition_->entities_[idx_], composition_->components_[idx_] };
    }

    constexpr pointer operator->() const
    { return pointer{ **this }; }


    constexpr basic_iterator& operator++()
    {
      ++idx_;
      return *this;
    }
    constexpr basic_iterator  operator++(int)
    {
      auto tmp = *this;
      ++*this;
      return tmp;
    }

    constexpr basic_iterator& operator--()
    {
      --idx_;
      return *this;
    }
    constexpr basic_iterator  operator--(int)
    {
      auto tmp = *this;
      --*this;
      return tmp;
    }


    constexpr basic_iterator  operator+(const std::ptrdiff_t n) const
    { return basic_iterator(composition_, idx_ + n); }

    constexpr basic_iterator  operator-(const std::ptrdiff_t n) const
    { return basic_iterator(composition_, idx_ - n); }
    constexpr difference_type operator-(const basic_iterator& other) const
    { return idx_ - other.idx_; }


    constexpr bool operator==(const basic_iterator& other) const
    { return idx_ == other.idx_; }
    constexpr bool operator!=(const basic_iterator& other) const
    { return !(*this == other); }


    constexpr bool operator<(const basic_iterator& other) const
    { return idx_ < other.idx_; }
    constexpr bool operator>(const basic_iterator& other) const
    { return idx_ > other.idx_; }


    constexpr bool operator<=(const basic_iterator& other) const
    { return !(*this > other); }
    constexpr bool operator>=(const basic_iterator& other) const
    { return !(*this < other); }

  private:
    composition_type* composition_;
    size_type         idx_;

  };

  using iterator       = basic_iterator<composition>;
  using const_iterator = basic_iterator<const composition>;

  using reverse_iterator       = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  using page_type = std::array<size_type, page_size>;



  /**
   * @brief Gets the component of the given entity.
   *
   * @param e The entity to get the component of
   * @return The component of the given entity.
   * @throw std::out_of_range if the entity has no component in the
   * %composition.
   * @note This is the non-const version of this method.
   */
  component_reference       at(const entity_type e)
  {
    if (!contains(e))
      throw std::out_of_range("composition::at: entity out of range");
    return components_.at(sparse_.at(e / page_size).at(e % page_size));
  }
  /**
   * @brief Gets the component of the given entity.
   *
   * @param e The entity to get the component of
   * @return The component of the given entity.
   * @throw std::out_of_range if the entity has no component in the
   * %composition.
   * @note This is the const version of this method.
   */
  const component_reference at(const entity_type e) const
  {
    if (!contains(e))
      throw std::out_of_range("composition::at: entity out of range");
    return components_.at(sparse_.at(e / page_size).at(e % page_size));
  }

  /**
   * @brief Gets the component of the given entity.
   *
   * @param e The entity to get the component of
   * @return The component of the given entity.
   * @note This is the non-const version of this method.
   */
  constexpr component_reference       operator[](const entity_type e)
      noexcept
  { return components_[sparse_[e / page_size][e % page_size]]; }
  /**
   * @brief Gets the component of the given entity.
   *
   * @param e The entity to get the component of
   * @return The component of the given entity.
   * @note This is the const version of this method.
   */
  constexpr const_component_reference operator[](const entity_type e) const
      noexcept
  { return components_[sparse_[e / page_size][e % page_size]]; }

  /**
   * @brief Finds the entity in the %composition.
   *
   * @param e The entity to find in the %composition
   * @return An iterator pointing to the entity.
   * @note If the entity is not in the %composition, the method returns the end
   * iterator.
   * @note This is the non-const version of this method.
   */
  constexpr iterator       find(const entity_type e)
      noexcept
  {
    if (!contains(e))
      return end();
    size_type idx = sparse_[e / page_size][e % page_size];
    return iterator(this, idx);
  }
  /**
   * @brief Finds the entity in the %composition.
   *
   * @param e The entity to find in the %composition
   * @return An iterator pointing to the entity.
   * @note If the entity is not in the %composition, the method returns the end
   * iterator.
   * @note This is the const version of this method.
   */
  constexpr const_iterator find(const entity_type e) const
      noexcept
  {
    if (!contains(e))
      return end();
    size_type idx = sparse_[e / page_size][e % page_size];
    return const_iterator(this, idx);
  }

  /**
   * @brief Checks if the entity is in the %composition.
   *
   * @param e The entity to check for
   * @return @c true if the entity is in the %composition, @c false otherwise.
   */
  [[nodiscard]]
  constexpr bool contains(const entity_type e) const
      noexcept
  {
    return e / page_size < sparse_.size()
        && sparse_[e / page_size][e % page_size] < entities_.size()
        && entities_[sparse_[e / page_size][e % page_size]] == e;
  }



  /**
   * @brief Returns the iterator corresponding to the first element in the
   * %composition.
   *
   * @return The iterator corresponding to the first element in the
   * %composition.
   * @note This is the non-const version of this method.
   */
  [[nodiscard]]
  constexpr iterator       begin()
      noexcept
  { return iterator(this, 0); }
  /**
   * @brief Returns the iterator corresponding to the first element in the
   * %composition.
   *
   * @return The iterator corresponding to the first element in the
   * %composition.
   * @note This is the const version of this method.
   */
  [[nodiscard]]
  constexpr const_iterator begin() const
      noexcept
  { return const_iterator(this, 0); }

  /**
   * @brief Returns the iterator corresponding to the first element in the
   * %composition.
   *
   * @return The iterator corresponding to the first element in the
   * %composition.
   */
  [[nodiscard]]
  constexpr const_iterator cbegin() const
      noexcept
  { return begin(); }


  /**
   * @brief Returns the iterator corresponding to the last element in the
   * %composition.
   *
   * @return The iterator corresponding to the last element in the %composition.
   * @note This is the non-const version of this method.
   */
  [[nodiscard]]
  constexpr iterator       end()
      noexcept
  { return iterator(this, entities_.size()); }
  /**
   * @brief Returns the iterator corresponding to the last element in the
   * %composition.
   *
   * @return The iterator corresponding to the last element in the %composition.
   * @note This is the const version of this method.
   */
  [[nodiscard]]
  constexpr const_iterator end() const
      noexcept
  { return const_iterator(this, entities_.size()); }
  /**
   * @brief Returns the iterator corresponding to the last element in the
   * %composition.
   *
   * @return The iterator corresponding to the last element in the %composition.
   */
  [[nodiscard]]
  constexpr const_iterator cend() const
      noexcept
  { return end(); }


  /**
   * @brief Returns the reverse iterator corresponding to the first element in
   * the %composition.
   *
   * @return The reverse iterator corresponding to the first element in the
   * %composition.
   * @note This is the non-const version of this method.
   */
  [[nodiscard]]
  constexpr reverse_iterator       rbegin()
      noexcept
  { return reverse_iterator(end()); }
  /**
   * @brief Returns the reverse iterator corresponding to the first element in
   * the %composition.
   *
   * @return The reverse iterator corresponding to the first element in the
   * %composition.
   * @note This is the const version of this method.
   */
  [[nodiscard]]
  constexpr const_reverse_iterator rbegin() const
      noexcept
  { return const_reverse_iterator(end()); }

  /**
   * @brief Returns the reverse iterator corresponding to the first element in
   * the %composition.
   *
   * @return The reverse iterator corresponding to the first element in the
   * %composition.
   */
  [[nodiscard]]
  constexpr const_reverse_iterator crbegin() const
      noexcept
  { return rbegin(); }


  /**
   * @brief Returns the reverse iterator corresponding to the last element in
   * the %composition.
   *
   * @return The reverse iterator corresponding to the last element in the
   * %composition.
   * @note This is the non-const version of this method.
   */
  [[nodiscard]]
  constexpr reverse_iterator       rend()
      noexcept
  { return reverse_iterator(begin()); }
  /**
   * @brief Returns the reverse iterator corresponding to the last element in
   * the %composition.
   *
   * @return The reverse iterator corresponding to the last element in the
   * %composition.
   * @note This is the const version of this method.
   */
  [[nodiscard]]
  constexpr const_reverse_iterator rend() const
      noexcept
  { return const_reverse_iterator(begin()); }
  /**
   * @brief Returns the reverse iterator corresponding to the last element in
   * the %composition.
   *
   * @return The reverse iterator corresponding to the last element in the
   * %composition.
   */
  [[nodiscard]]
  constexpr const_reverse_iterator crend() const
      noexcept
  { return rend(); }



  /**
   * @brief Checks if the %composition is empty or not.
   *
   * @return @c true if the %composition is empty, @c false otherwise.
   */
  [[nodiscard]]
  constexpr bool empty() const
      noexcept
  { return entities_.empty(); }


  /**
   * @brief Returns the size (or number of components) of the %composition.
   *
   * @return The size of the %composition.
   */
  [[nodiscard]]
  constexpr size_type size()     const
      noexcept
  { return entities_.size(); }

  /**
   * @brief Returns the maximum size a %composition can have.
   *
   * @return The maximum size of the %composition.
   */
  [[nodiscard]]
  constexpr size_type max_size() const
      noexcept
  { return entities_.max_size(); }


  /**
   * @brief Tries to increase the capacity of the %composition.
   *
   * @param n The required new capacity for the %composition.
   * @throw std::length_error if @a n exceeds @c max_size().
   */
  constexpr void reserve(size_type n)
      noexcept(std::is_nothrow_move_constructible_v<component_type>)
  {
    entities_.reserve(n);
    components_.reserve(n);
  }

  /**
   * @brief Returns the number of components the %composition can hold before
   * needing to reallocate memory.
   *
   * @return The number of components the %composition can hold before needing
   * to reallocate memory.
   */
  [[nodiscard]]
  constexpr size_type capacity() const
      noexcept
  { return entities_.capacity(); }

  /**
   * Reduces the capacity of the %composition to match its current size.
   */
  constexpr void shrink_to_fit()
      noexcept(std::is_nothrow_move_constructible_v<component_type>)
  {
    entities_.shrink_to_fit();
    components_.shrink_to_fit();
  }



  /**
   * @brief Erases all the elements in the %composition.
   */
  constexpr void clear()
  {
    sparse_.clear();
    sparse_.shrink_to_fit();
    entities_.clear();
    components_.clear();
  }


  /**
   * @brief Emplaces a component in the %composition.
   *
   * @tparam Args The argument types necessary to construct the component
   * @param e    The entity to emplace a new component for
   * @param args The arguments necessary to construct the component
   * @return @c true if a component has been emplaced, @c false otherwise.
   */
  template<typename... Args>
  constexpr bool emplace(const entity_type e, Args&&... args)
      noexcept(std::is_nothrow_constructible_v<component_type, Args&&...>)
  {
    if (contains(e))
      return false;

    const size_type page_idx = e / page_size;
    const size_type line_idx = e % page_size;

    if (const size_type old_size = sparse_.size();
        page_idx >= old_size)
    {
      const size_type new_size = page_idx + 1;
      sparse_.resize(new_size);
      for (size_type i = old_size; i < new_size; ++i)
        sparse_[i].fill(null_idx);
    }

    sparse_[page_idx][line_idx] = entities_.size();
    entities_.emplace_back(e);
    components_.emplace_back(std::forward<Args>(args)...);
    return true;
  }


  /**
   * @brief Erases the component of a given entity.
   *
   * @param e The entity to erase the component of
   */
  constexpr void     erase(const entity_type e)
      noexcept
  {
    if (!contains(e))
      return;

    const size_type page_idx = e / page_size;
    const size_type line_idx = e % page_size;
    size_type idx = sparse_[page_idx][line_idx];

    if (idx != entities_.size() - 1)
    {
      entities_[idx]   = entities_.back();
      components_[idx] = std::move(components_.back());

      sparse_[entities_.back() / page_size][entities_.back() % page_size] = idx;
    }

    sparse_[page_idx][line_idx] = null_idx;

    entities_.pop_back();
    components_.pop_back();
  }

  /**
   * @brief Erases the component pointed to by the given iterator.
   *
   * @param pos The iterator pointing to the component to erase
   * @return The iterator following the erased component.
   */
  constexpr iterator erase(iterator       pos)
      noexcept
  {
    erase((*pos).entity);
    return iterator(this, pos.idx_);
  }
  /**
   * @brief Erases the component pointed to by the given const_iterator.
   *
   * @param pos The const_iterator pointing to the component to erase
   * @return The iterator following the erased component.
   */
  constexpr iterator erase(const_iterator pos)
      noexcept
  {
    erase((*pos).entity);
    return iterator(this, pos.idx_);
  }

  /**
   * @brief Erases a range of components using the given iterators.
   *
   * @param first The first iterator of the range to erase.
   * @param last The last iterator of the range to erase.
   * @return The last iterator.
   */
  constexpr iterator erase(iterator       first, iterator       last)
      noexcept
  {
    while (first != last)
      first = erase(first);
    return first;
  }
  /**
   * @brief Erases a range of components using the given const_iterators.
   *
   * @param first The first const_iterator of the range to erase.
   * @param last The last const_iterator of the range to erase.
   * @return The last const_iterator.
   */
  constexpr iterator erase(const_iterator first, const_iterator last)
      noexcept
  {
    while (first != last)
      first = erase(first);
    return first;
  }


  /**
   * @brief Sorts the components of the %composition.
   *
   * @param cmp The predicate used to determine order for sorting.
   */
  constexpr void sort(
      std::function<bool(const Component&, const Component&)> cmp)
      noexcept(std::is_nothrow_move_assignable_v<component_type>)
  {
    const size_type n = entities_.size();
    if (n <= 1)
      return;

    std::vector<size_type> order(n);
    std::iota(order.begin(), order.end(), 0ULL);

    std::sort(order.begin(), order.end(),
        [&](size_type lhs, size_type rhs)
        { return cmp(components_[lhs], components_[rhs]); });

    for (size_type i = 0; i < n; ++i)
    {
      size_type j = i;
      while (order[j] != i)
      {
        std::swap(entities_[j], entities_[order[j]]);
        std::swap(components_[j], components_[order[j]]);
        sparse_[entities_[j] / page_size][entities_[j] % page_size] = j;

        const size_type next = order[j];
        order[j] = j;
        j = next;
      }
      sparse_[entities_[j] / page_size][entities_[j] % page_size] = j;
      order[j] = j;
    }
  }

private:
  std::vector<page_type>      sparse_;
  std::vector<entity_type>    entities_;
  std::vector<component_type> components_;

};
}

#endif // HEIM_COMPOSITION_HPP
