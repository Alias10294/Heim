#ifndef HEIM_COMPOSITION_COMPOSITION_HPP
#define HEIM_COMPOSITION_COMPOSITION_HPP

#include <array>
#include <concepts>
#include <cstddef>
#include <memory>
#include <numeric>
#include <tuple>
#include <type_traits>
#include <vector>
#include "core/component.hpp"
#include "core/entity.hpp"

namespace heim
{
/**
 * @brief A specialized unordered associative container for entities and their
 *     components.
 *
 * @tparam Entity             The type of the entities to contain.
 * @tparam Component          The type of the components to contain.
 * @tparam PageSize           The size of each page of the sparse container.
 * @tparam ComponentAllocator The type of the allocator for components.
 */
template<typename    Entity,
         typename    Component,
         std::size_t PageSize = 4096,
         typename    ComponentAllocator = std::allocator<Component>>
requires  core::entity<Entity>
      &&  core::component<Component>
      && (PageSize > 0)
      &&  std::same_as<
              typename std::allocator_traits<ComponentAllocator>::value_type,
              Component>
class composition
{
public:
  using entity_type              = Entity;
  using component_type           = Component;
  using component_allocator_type = ComponentAllocator;

  constexpr
  static std::size_t page_size = PageSize;

private:
  /// @cond INTERNAL

  /**
   * @brief The sparse container of the composition.
   */
  class sparse_container
  {
  private:
    using page_type           = std::array<std::size_t, page_size>;
    using page_container_type = std::vector<std::unique_ptr<page_type>>;

    using iterator       = typename page_container_type::iterator;
    using const_iterator = typename page_container_type::const_iterator;

  public:
    /// @brief The index used to signify a non-existing element.
    constexpr
    static std::size_t null_idx = std::numeric_limits<std::size_t>::max();

  public:
    constexpr
    sparse_container()
    noexcept
    = default;
    constexpr
    sparse_container(sparse_container const &other)
      : pages_{}
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
    sparse_container(sparse_container &&)
    noexcept
    = default;

    constexpr
    ~sparse_container()
    noexcept
    = default;


    [[nodiscard]]
    constexpr
    sparse_container &operator=(sparse_container const &other)
    {
      if (this == &other)
        return *this;

      page_container_type pages;

      pages.reserve(other.pages_.capacity());
      for (auto const &page_uptr : other.pages_)
      {
        if (page_uptr)
          pages_.emplace_back(std::make_unique<page_type>(*page_uptr));
        else
          pages_.emplace_back(nullptr);
      }
      pages_ = std::move(pages);
      return *this;
    }
    [[nodiscard]]
    constexpr
    sparse_container &operator=(sparse_container &&)
    noexcept
    = default;



    /**
     * @brief Tidies the sparse container by destroying its empty pages and
     *     reducing its size as much as possible.
     */
    constexpr
    void tidy()
    noexcept
    {
      for (auto &page_uptr : pages_)
      {
        if (page_uptr && is_blank(*page_uptr))
          page_uptr.reset();
      }

      while (!pages_.empty() && !pages_.back())
        pages_.pop_back();
    }

    /**
     * @brief Requests the deallocation of unused memory.
     */
    constexpr
    void shrink_to_fit()
    {
      pages_.shrink_to_fit();
    }



    /**
     * @return An iterator to the first page in the container.
     */
    [[nodiscard]]
    constexpr
    iterator       begin()
    noexcept
    {
      return pages_.begin();
    }
    /**
     * @return A const_iterator to the first page in the container.
     */
    [[nodiscard]]
    constexpr
    const_iterator begin() const
    noexcept
    {
      return pages_.begin();
    }

    /**
     * @return An iterator to after the last page in the container.
     *
     * @warning This iterator only acts as a sentinel, and is not to be
     *     dereferenced.
     */
    [[nodiscard]]
    constexpr
    iterator       end()
    noexcept
    {
      return pages_.end();
    }
    /**
     * @return A const iterator to after the last page in the container.
     *
     * @warning This returned const_iterator only acts as a sentinel. It is not
     *     to be dereferenced.
     */
    [[nodiscard]]
    constexpr
    const_iterator end() const
    noexcept
    {
      return pages_.end();
    }


    /**
     * @return A const_iterator to the first page in the container.
     */
    [[nodiscard]]
    constexpr
    const_iterator cbegin() const
      noexcept
    {
      return pages_.cbegin();
    }

    /**
     * @return A const iterator to the after last page of the container.
     *
     * @warning This const_iterator only acts as a sentinel, and is not to be
     *     dereferenced.
     */
    [[nodiscard]]
    constexpr
    const_iterator cend() const
    noexcept
    {
      return pages_.cend();
    }



    /**
     * @param e The entity to search an index for.
     * @return @c true if @code e@endcode has an index in the container,
     *     @c false otherwise.
     */
    [[nodiscard]]
    constexpr
    bool contains(entity_type const e) const
    noexcept
    {
      return page_index(e) < pages_.size()
          && pages_[page_index(e)]
          && operator[](e) != null_idx;
    }


    /**
     * @param e The entity to search an index for.
     * @return A reference to the index of @code e@endcode.
     */
    [[nodiscard]]
    constexpr
    std::size_t &operator[](entity_type const e)
    noexcept
    {
      return (*pages_[page_index(e)])[line_index(e)];
    }
    /**
     * @param e The entity to search an index for.
     * @return A const reference to the index of @code e@endcode.
     */
    [[nodiscard]]
    constexpr
    std::size_t  operator[](entity_type const e) const
    noexcept
    {
      return (*pages_[page_index(e)])[line_index(e)];
    }

    /**
     * @param e The entity to search an index for.
     * @return A reference to the index of @code e@endcode.
     * @throw std::out_of_range if @code e@endcode does not have an index in
     *     the container.
     */
    [[nodiscard]]
    constexpr
    std::size_t &at(entity_type const e)
    {
      if (!contains(e))
        throw std::out_of_range("heim::component::sparse_container::at");

      return operator[](e);
    }
    /**
     * @param e The entity to search an index for.
     * @return A const reference to the index of @code e@endcode.
     * @throw std::out_of_range if @code e@endcode does not have an index in
     *     the container.
     */
    [[nodiscard]]
    constexpr
    std::size_t  at(entity_type const e) const
    noexcept
    {
      if (!contains(e))
        throw std::out_of_range("heim::component::sparse_container::at");

      return operator[](e);
    }



    /**
     * @brief Ensures @code e@endcode can be indexed in the container,
     *     allocating a new empty page if needed.
     *
     * @param e The entity to ensure the indexing of.
     */
    constexpr
    void ensure(entity_type const e)
    {
      if (page_index(e) >= pages_.size())
        pages_.resize(page_index(e) + 1);

      if (!pages_[page_index(e)])
      {
        pages_[page_index(e)] = std::make_unique<page_type>();
        pages_[page_index(e)] ->fill(null_idx);
      }
    }


    /**
     * @brief Blanks all pages in the container.
     */
    constexpr
    void blank()
    noexcept
    {
      for (auto &page_uptr : pages_)
      {
        if (page_uptr)
          page_uptr->fill(null_idx);
      }
    }

    /**
     * @brief Clears the container of all its pages.
     */
    constexpr
    void clear()
    noexcept
    {
      pages_.clear();
    }



    /**
     * @brief Swaps the contents of @p *this and @code other@endcode.
     *
     * @param other The other container whose contents to swap.
     */
    constexpr
    void swap(sparse_container &other)
    noexcept
    {
      pages_.swap(other.pages_);
    }
    /**
     * @brief Swaps the indices of @code a@endcode and @code b@endcode in the
     *     container.
     *
     * @param a The first entity whose index to swap.
     * @param b The second entity whose index to swap.
     */
    constexpr
    void swap(entity_type const a, entity_type const b)
    noexcept
    {
      std::size_t const tmp = sparse_[a];
      sparse_[a]            = sparse_[b];
      sparse_[b]            = tmp;
    }

  private:
    /**
     * @param page The page to check the blankness (i.e. emptiness) of.
     * @return @c true if the page is blank (i.e. has no entities indexed),
     *     @c false otherwise.
     */
    [[nodiscard]]
    constexpr
    bool is_blank(page_type const& page)
    noexcept
    {
      for (auto const idx : page)
      {
        if (idx != null_idx)
          return false;
      }
      return true;
    }


    /**
     * @param e The entity to get the page index of.
     * @return The page index of @code e@endcode.
     */
    [[nodiscard]]
    constexpr
    static std::size_t page_index(entity_type const e)
    noexcept
    {
      return e / page_size;
    }

    /**
     * @param e The entity to get the index in its page of.
     * @return The index in its page of @code e@endcode.
     */
    [[nodiscard]]
    constexpr
    static std::size_t line_index(entity_type const e)
    noexcept
    {
      return e % page_size;
    }

  private:
    page_container_type pages_;

  };



  template<bool IsConst>
  using proxy_type = std::tuple<
      entity_type const,
      std::conditional_t<IsConst,
          component_type const &,
          component_type &>>;

  /**
   * @brief The dense container of elements of the composition.
   */
  class dense_container
  {
  private:
    using entity_container_type    = std::vector<entity_type>;
    using component_container_type = std::vector<
        component_type,
        component_allocator_type>;



    /**
     * @brief The custom iterator of the dense container.
     *
     * @warning This iterator uses a proxy to access each element of the dense
     *     container, so modifications on the container can not be done
     *     through the iterators. Instead, use its custom algorithms.
     */
    template<bool IsConst>
    class generic_iterator
    {
    public:
      constexpr
      static bool is_const = IsConst;

    private:
      using entity_pointer_type    = entity_type const *;
      using component_pointer_type = std::conditional_t<is_const,
          component_type const *,
          component_type *>;

    public:
      using proxy_type = proxy_type<IsConst>;


      using difference_type   = std::ptrdiff_t;
      using value_type        = proxy_type;
      using pointer           = void;
      using reference         = proxy_type;
      using iterator_category = void;
      using iterator_concept  = std::random_access_iterator_tag;

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
      generic_iterator(
          entity_pointer_type const entities,
          component_pointer_type    components)
      noexcept
        : entities_  {entities},
          components_{components}
      { }

      constexpr
      ~generic_iterator()
      noexcept
      = default;


      constexpr
      generic_iterator &operator=(generic_iterator const &other)
      = default;
      constexpr
      generic_iterator &operator=(generic_iterator &&other)
      noexcept
      = default;



      [[nodiscard]]
      constexpr
      proxy_type operator*() const
      noexcept
      {
        return proxy_type{*entities_, *components_};
      }



      constexpr
      generic_iterator &operator++()
      noexcept
      {
        ++entities_;
        ++components_;
        return *this;
      }

      constexpr
      generic_iterator &operator--()
      noexcept
      {
        --entities_;
        --components_;
        return *this;
      }


      [[nodiscard]]
      constexpr
      generic_iterator operator+(std::ptrdiff_t const dist) const
      noexcept
      {
        return generic_iterator{
            entities_   + dist,
            components_ + dist};
      }



      [[nodiscard]]
      constexpr
      bool operator==(generic_iterator const &other) const
      noexcept
      {
        return entities_ == other.entities_;
      }

      [[nodiscard]]
      constexpr
      auto operator<=>(generic_iterator const &other) const
      noexcept
      {
        return entities_ <=> other.entities_;
      }



      constexpr
      void swap(generic_iterator &other)
      noexcept
      {
        std::swap(entities_  , other.entities_  );
        std::swap(components_, other.components_);
      }

    private:
      entity_pointer_type    entities_  {nullptr};
      component_pointer_type components_{nullptr};

    };

  public:
    using iterator       = generic_iterator<false>;
    using const_iterator = generic_iterator<true>;

    using proxy_type       = typename iterator::proxy_type;
    using const_proxy_type = typename const_iterator::proxy_type;

  public:
    /**
     * @return @c true if the container has no elements, @c false
     *     otherwise.
     */
    [[nodiscard]]
    constexpr
    bool empty() const
    noexcept
    {
      return entities_.empty();
    }

    /**
     * @return The number of elements in the container.
     */
    [[nodiscard]]
    constexpr
    std::size_t size() const
    noexcept
    {
      return entities_.size();
    }

    /**
     * @return The maximum number of elements the container can contain.
     */
    [[nodiscard]]
    constexpr
    std::size_t max_size() const
    noexcept
    {
      return entities_.max_size();
    }


    /**
     * @return The maximum number of elements the container can contain without
     *     requiring a reallocation.
     */
    [[nodiscard]]
    constexpr
    std::size_t capacity() const
    noexcept
    {
      return entities_.capacity();
    }

    /**
     * @brief Requests the deallocation of unused memory, shrinking the
     *     capacity of the dense container to fit its size.
     */
    constexpr
    void shrink_to_fit()
    {
      entities_  .shrink_to_fit();
      components_.shrink_to_fit();
    }

    /**
     * @brief Reallocates to increases the number of elements it can contain to
     *     @code new_cap@endcode.
     *
     * @param new_cap The new capacity of the container, in number of
     *     elements.
     * @throw std::length_error if @code new_cap > max_size()@endcode.
     */
    constexpr
    void reserve(std::size_t const new_cap)
    {
      entities_  .reserve(new_cap);
      components_.reserve(new_cap);
    }



    /**
     * @return An iterator to the first element in the container.
     */
    [[nodiscard]]
    constexpr
    iterator       begin()
    noexcept
    {
      return iterator{
          entities_  .data(),
          components_.data()};
    }

    /**
     * @return A const iterator to the first element in the container.
     */
    [[nodiscard]]
    constexpr
    const_iterator begin() const
    noexcept
    {
      return const_iterator{
          entities_  .data(),
          components_.data()};
    }

    /**
     * @return An iterator to after the last element in the container.
     *
     * @warning This iterator only acts as a sentinel, and is not to be
     *     dereferenced.
     */
    [[nodiscard]]
    constexpr
    iterator       end()
    noexcept
    {
      return iterator{
          entities_  .data() + size(),
          components_.data() + size()};
    }

    /**
     * @return A const iterator to after the last element in the container.
     *
     * @warning This iterator only acts as a sentinel, and is not to be
     *     dereferenced.
     */
    [[nodiscard]]
    constexpr
    const_iterator end() const
    noexcept
    {
      return const_iterator{
          entities_  .data() + size(),
          components_.data() + size()};
    }


    /**
     * @return A const iterator to the first element in the container.
     */
    [[nodiscard]]
    constexpr
    const_iterator cbegin() const
    noexcept
    {
      return begin();
    }

    /**
     * @return A const iterator to after the last element in the container.
     *
     * @warning This iterator only acts as a sentinel, and is not to be
     *     dereferenced.
     */
    [[nodiscard]]
    constexpr
    const_iterator cend() const
      noexcept
    {
      return end();
    }



    /**
     * @param idx The index to get an element proxy of.
     * @return A proxy to the element at position @code idx@endcode.
     */
    [[nodiscard]]
    constexpr
    proxy_type operator[](std::size_t const idx)
    noexcept
    {
      return proxy_type{entities_[idx], components_[idx]};
    }

    /**
     * @param idx The index to get an element proxy of.
     * @return A const proxy to the element at location @code idx@endcode.
     */
    [[nodiscard]]
    constexpr
    const_proxy_type operator[](std::size_t const idx) const
    noexcept
    {
      return const_proxy_type{entities_[idx], components_[idx]};
    }

    /**
     * @param idx The index to get an element proxy of.
     * @return A proxy to the elements at location @code idx@endcode.
     * @throw std::out_of_range if @code idx >= size()@endcode.
     */
    [[nodiscard]]
    constexpr
    proxy_type at(std::size_t const idx)
    {
      if (idx >= size())
        throw std::out_of_range("heim::component::dense_container::at");

      return operator[](idx);
    }

    /**
     * @param idx The index to get an element proxy of.
     * @return A const proxy to the elements at location @code idx@endcode.
     * @throw std::out_of_range if @code idx >= size()@endcode.
     */
    [[nodiscard]]
    constexpr
    const_proxy_type at(std::size_t const idx) const
    {
      if (idx >= size())
        throw std::out_of_range("heim::component::dense_container::at");

      return operator[](idx);
    }


    /**
     * @return A proxy to the first element in the container.
     */
    [[nodiscard]]
    constexpr
    proxy_type       front()
    noexcept
    {
      return proxy_type{
          entities_  .front(),
          components_.front()};
    }

    /**
     * @return A const proxy to the first element in the container.
     */
    [[nodiscard]]
    constexpr
    const_proxy_type front() const
    noexcept
    {
      return const_proxy_type{
          entities_  .front(),
          components_.front()};
    }

    /**
     * @return A proxy to the last element in the container.
     */
    [[nodiscard]]
    constexpr
    proxy_type       back()
    noexcept
    {
      return proxy_type{
          entities_  .back(),
          components_.back()};
    }

    /**
     * @return A const proxy to the last element in the container.
     */
    [[nodiscard]]
    constexpr
    const_proxy_type back() const
    noexcept
    {
      return const_proxy_type{
          entities_  .back(),
          components_.back()};
    }



    /**
     * @brief Append a new element to the end of the container,
     *     forwarding the arguments @code args...@endcode to the constructor as
     *     @code std::forward<Args>(args)...@endcode.
     *
     * @param e The entity to append to the container.
     * @param args The arguments to construct the corresponding component in
     *     the container.
     * @return @c true if a new element has been emplaced, @c false otherwise.
     *
     * @note If an exception is thrown for any reason, this function has no
     *     effect (strong exception safety guarantee).
     */
    template<typename... Args>
    constexpr
    void emplace_back(entity_type const e, Args &&...args)
    {
      components_.emplace_back(std::forward<Args>(args)...);
      try
      {
        entities_.emplace_back(e);
      }
      catch (...)
      {
        components_.pop_back();
        throw;
      }
    }


    /**
     * @brief Erases the element at location @code idx@endcode.
     *
     * @param idx The index of the element to erase in the container.
     *
     * @warning This method uses a @a swap-and-pop algorithm to induce
     *     constant-time complexity for the operation. This comes at the cost
     *     of breaking the current order of elements in the container.
     * @note If an exception is thrown for any reason, this function has no
     *     effect (strong exception safety guarantee).
     */
    constexpr
    void erase(std::size_t const idx)
    noexcept(std::is_nothrow_move_assignable_v<Component>)
    {
      components_[idx] = std::move(components_.back());
      entities_  [idx] = std::move(entities_.back());

      components_.pop_back();
      entities_  .pop_back();
    }

    /**
     * @brief Erases all elements from the container.
     */
    constexpr
    void clear()
    noexcept
    {
      entities_  .clear();
      components_.clear();
    }



    /**
     * @brief Swaps the contents of @p *this and @code other@endcode.
     *
     * @param other The other container whose contents to swap.
     */
    constexpr
    void swap(dense_container &other)
    noexcept(noexcept(
        std::declval<component_container_type>().swap(
            std::declval<component_container_type>())))
    {
      components_.swap(other.components_);
      entities_  .swap(other.entities_);
    }

    /**
     * @brief Swaps the elements at @code idx_a@endcode and @code idx_b@endcode
     *     in the container.
     *
     * @param idx_a The first index whose element to swap.
     * @param idx_b The second index whose element to swap.
     */
    constexpr
    void swap(std::size_t const idx_a, std::size_t const idx_b)
      noexcept(noexcept(std::swap(
          std::declval<component_type>(),
          std::declval<component_type>())))
    {
      std::swap(entities_  [idx_a], entities_  [idx_b]);
      std::swap(components_[idx_a], components_[idx_b]);
    }

  private:
    entity_container_type    entities_;
    component_container_type components_;

  };

  /// @endcond

public:
  using iterator       = typename dense_container::iterator;
  using const_iterator = typename dense_container::const_iterator;

public:
  /**
   * @return @c true if the container has no elements, @c false otherwise.
   */
  [[nodiscard]]
  constexpr
  bool empty() const
  noexcept
  {
    return dense_.empty();
  }

  /**
   * @return The number of elements in the container.
   */
  [[nodiscard]]
  constexpr
  std::size_t size() const
  noexcept
  {
    return dense_.size();
  }

  /**
   * @return The maximum number of elements the container can contain.
   */
  [[nodiscard]]
  constexpr
  std::size_t max_size() const
  noexcept
  {
    return dense_.max_size();
  }


  /**
   * @return The maximum number of elements the container can contain without
   *     requiring a reallocation.
   */
  [[nodiscard]]
  constexpr
  std::size_t capacity() const
  noexcept
  {
    return dense_.capacity();
  }

  /**
   * @brief Requests the deallocation of unused memory, shrinking the capacity
   *     of its dense container to fit its size and destroying the empty pages
   *     of its sparse container.
   */
  constexpr
  void shrink_to_fit()
  {
    sparse_.tidy();
    sparse_.shrink_to_fit();

    dense_ .shrink_to_fit();
  }

  /**
   * @brief Increases the number of elements the dense container can hold
   *     without requiring reallocation.
   *
   * @param new_cap The new capacity of the container, in number of elements.
   * @throw std::length_error if @code new_cap > max_size()@endcode.
   */
  constexpr
  void reserve(std::size_t const new_cap)
  {
    dense_.reserve(new_cap);
  }



  /**
   * @return An iterator to the first element in the container.
   */
  [[nodiscard]]
  constexpr
  iterator       begin()
  noexcept
  {
    return dense_.begin();
  }
  /**
   * @return A const iterator to the first element in the container.
   */
  [[nodiscard]]
  constexpr
  const_iterator begin() const
  noexcept
  {
    return dense_.begin();
  }

  /**
   * @return An iterator to after the last element in the container.
   *
   * @warning This iterator only acts as a sentinel, and is not to be
   *     dereferenced.
   */
  [[nodiscard]]
  constexpr
  iterator       end()
  noexcept
  {
    return dense_.end();
  }
  /**
   * @return A const iterator to the last element in the container.
   *
   * @warning This const iterator only acts as a sentinel, and is not to be
   *     dereferenced.
   */
  [[nodiscard]]
  constexpr
  const_iterator end() const
  noexcept
  {
    return dense_.end();
  }


  /**
   * @return A const iterator to the first element in the container.
   */
  [[nodiscard]]
  constexpr
  const_iterator cbegin() const
  noexcept
  {
    return dense_.cbegin();
  }

  /**
   * @return A const iterator to the last element in the container.
   *
   * @warning This const iterator only acts as a sentinel, and is not to be
   *     dereferenced.
   */
  [[nodiscard]]
  constexpr
  const_iterator cend() const
    noexcept
  {
    return dense_.cend();
  }



  /**
   * @param e The entity to search an element for.
   * @return @c true if @code e@endcode has a corresponding element in the
   *     container, @c false otherwise.
   */
  [[nodiscard]]
  constexpr
  bool contains(entity_type const e) const
  noexcept
  {
    return sparse_.contains(e)
        && std::get<0>(dense_[sparse_[e]]) == e;
  }


  /**
   * @param e The entity of the component to get.
   * @return A reference to the value that is mapped to @code e@endcode.
   *
   * @note Unlike @p std::unordered_map, if @code e@endcode does not already
   *     exist in the container, the behaviour is undefined.
   */
  [[nodiscard]]
  constexpr
  component_type       &operator[](entity_type const e)
  noexcept
  {
    return std::get<1>(dense_[sparse_[e]]);
  }
  /**
   * @param e The entity of the component to get.
   * @return A const reference to the value that is mapped to @code e@endcode.
   *
   * @note Unlike @p std::unordered_map, if @code e@endcode does not already
   *     exist in the container, the behaviour is undefined.
   */
  [[nodiscard]]
  constexpr
  component_type const &operator[](entity_type const e) const
  noexcept
  {
    return std::get<1>(dense_[sparse_[e]]);
  }

  /**
   * @param e The entity of the component to get.
   * @return A reference to the value that is mapped to @code e@endcode.
   * @throw std::out_of_range if @code e@endcode does not have an element in
   *     the container.
   */
  [[nodiscard]]
  constexpr
  component_type       &at(entity_type const e)
  {
    return std::get<1>(dense_.at(sparse_.at(e)));
  }
  /**
   * @param e The entity of the component to get.
   * @return A const reference to the value that is mapped to @code e@endcode.
   * @throw std::out_of_range if @code e@endcode does not have an element in
   *     the container.
   */
  [[nodiscard]]
  constexpr
  component_type const &at(entity_type const e) const
  {
    return std::get<1>(dense_.at(sparse_.at(e)));
  }



  /**
   * @brief Emplaces a new element for @code e@endcode, if it does not already
   *     have a corresponding element, forwarding the arguments
   *     @code args...@endcode to the constructor of the component as
   *     @code std::forward<Args>(args)...@endcode.
   *
   * @param e The entity to emplace in the container.
   * @param args The arguments to construct the corresponding component in the
   *     container.
   * @return @c true if a new element has been emplaced, @c false otherwise.
   *
   * @note If an exception is thrown for any reason, this function has no
   *     effect (strong exception safety guarantee).
   */
  template<typename ...Args>
  constexpr
  bool emplace(entity_type const e, Args &&...args)
  {
    if (contains(e))
      return false;

    sparse_.ensure(e);

    dense_.emplace_back(e, std::forward<Args>(args)...);
    sparse_[e] = dense_.size() - 1;

    return true;
  }


  /**
   * @brief Erases the element corresponding to @code e@endcode from the
   *     container.
   *
   * @param e The entity of the element to erase from the container.
   * @return @c true if an element has been erased, @c false otherwise.
   *
   * @warning This method uses a @a swap-and-pop algorithm to induce
   *     constant-time complexity for the operation. This comes at the cost of
   *     breaking the current order of the elements in the container.
   * @note If an exception is thrown for any reason, this function has no
   *     effect (strong exception safety guarantee).
   */
  constexpr
  bool erase(entity_type const e)
  noexcept(noexcept(std::declval<dense_container>().erase(0)))
  {
    if (!contains(e))
      return false;

    entity_type const back_e = std::get<0>(dense_.back());

    dense_.erase(sparse_[e]);

    sparse_[back_e] = sparse_[e];
    sparse_[e]      = sparse_.null_idx;

    return true;
  }

  /**
   * @brief Erases all the elements from its dense container and blanks all the
   *     pages of its sparse container.
   */
  constexpr
  void clear()
  noexcept
  {
    sparse_.blank();
    dense_ .clear();
  }


  /**
   * @brief Swaps the contents of @p *this and @code other@endcode.
   *
   * @param other The other composition whose contents to swap.
   */
  constexpr
  void swap(composition& other)
  noexcept(noexcept(std::declval<dense_container>().swap(
      std::declval<dense_container>())))
  {
    dense_ .swap(other.dense_ );
    sparse_.swap(other.sparse_);
  }

  /**
   * @brief Swaps the elements corresponding @code a@endcode and
   *     @code b@endcode in the composition.
   *
   * @param a The first  entity whose element to swap.
   * @param b The second entity whose element to swap.
   */
  constexpr
  void swap(entity_type const a, entity_type const b)
  noexcept(noexcept(std::declval<dense_container>().swap(0, 0)))
  {
    dense_ .swap(sparse_[a], sparse_[b]);
    sparse_.swap(a, b);
  }

private:
  sparse_container sparse_;
  dense_container  dense_;

};


/**
 * @brief Swaps the contents of @code lhs@endcode and @code rhs@endcode.
 *
 * @param lhs The first composition whose contents to swap.
 * @param rhs The second composition whose contents to swap.
 */
template<typename Entity,
         typename Component,
         std::size_t PageSize = 4096,
         typename ComponentAllocator = std::allocator<Component>>
requires  std::unsigned_integral<Entity>
      &&  std::is_move_constructible_v<Component>
      &&  std::is_move_assignable_v   <Component>
      &&  std::is_destructible_v      <Component>
      && (PageSize > 0)
constexpr
void swap(
    composition<Entity, Component, PageSize, ComponentAllocator>& lhs,
    composition<Entity, Component, PageSize, ComponentAllocator>& rhs)
noexcept(noexcept(std::declval<
    composition<Entity, Component, PageSize, ComponentAllocator>>().swap(
        std::declval<
            composition<Entity, Component, PageSize, ComponentAllocator>>())))
{
  lhs.swap(rhs);
}

}

#endif // HEIM_COMPOSITION_COMPOSITION_HPP
