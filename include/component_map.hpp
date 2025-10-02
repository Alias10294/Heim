#ifndef HEIM_COMPONENT_MAP_HPP
#define HEIM_COMPONENT_MAP_HPP

#include <array>
#include <cstddef>
#include <format>
#include <limits>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>
#include "component.hpp"
#include "entity.hpp"

namespace heim
{
/*!
 * @brief The associative container for entities and components of Heim.
 *
 * @tparam Entity The type of entities (keys) to hold.
 * @tparam Component The type of components (values) to hold.
 * @tparam PageSize The size of each page of indexes.
 * @tparam Allocator The type of allocator for components.
 *
 * @details Implements a customised sparse set, providing constant-time
 *   complexity on insertion, deletion and search of element. Uses pagination
 *   to limit memory usage on the sparse array.
 */
template<
    typename    Entity,
    typename    Component,
    std::size_t PageSize,
    typename    Allocator>
class component_map
{
  static_assert(
      is_entity_v<Entity>,
      "heim::component_map: Entity must satisfy: "
          "is_entity_v<Entity> ");
  static_assert(
      is_component_v<Component>,
      "heim::component_map: Component must satisfy: "
          "is_component_v<Component> ");
  static_assert(
      is_component_allocator_v<Component, Allocator>,
      "heim::component_map: Allocator must satisfy: "
          "is_component_allocator_v<Component, Allocator> ");

public:
  //! @brief The size integer type.
  using size_type
  = std::size_t;



  //! @brief The type of entities (keys).
  using entity_type
  = Entity;

  //! @brief The type of components (values).
  using component_type
  = Component;

  //! @brief The size of each page of indexes.
  constexpr
  static size_type page_size
  = PageSize;

  //! @brief The type of allocator for components.
  using component_allocator_type
  = Allocator;

  //! @cond INTERNAL
private:
  /*!
   * @brief The iterator type of the component_map.
   *
   * @tparam IsConst Whether the iterator is to be used in a const setting or
   *   not.
   *
   * @details Because of the data structure of the component_map, this iterator
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


    //! @brief The type of entities (keys).
    using entity_type
    = Entity;

    //! @brief The type of components (values).
    using component_type
    = std::conditional_t<
        is_const,
        Component const,
        Component>;



    //! @brief The type of the proxy used.
    using proxy
    = std::tuple<entity_type const &, component_type &>;

  private:
    entity_type const *entities_;
    component_type    *components_;

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
        entity_type const *entities,
        component_type    *components)
      : entities_  {entities},
        components_{components}
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
          *entities_,
          *components_};
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
    bool operator==(generic_iterator const &other) const
    noexcept
    {
      return entities_ == other.entities_;
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
  //! The type of the pages of the sparse container.
  using page_type
  = std::array<size_type, page_size>;

  //! The index to represent a non-existing entity.
  constexpr
  static size_type null_index_
  = std::numeric_limits<size_type>::max();

  //! @endcond
  //! @cond INTERNAL
private:
  std::vector<std::unique_ptr<page_type>> pages_;

  std::vector<entity_type>                              entities_;
  std::vector<component_type, component_allocator_type> components_;



  /*!
   * @brief Checks if the given page directs to no entities.
   *
   * @param page The page to check for entities.
   * @returns @c true if the given page directs to no entities, @c false
   *   otherwise.
   */
  constexpr
  static bool is_blank(page_type const &page)
  noexcept
  {
    for (auto const idx : page)
    {
      if (idx != null_index_)
        return false;
    }
    return true;
  }


  /*!
   * @brief The page index of the given entity.
   *
   * @param e The entity to get the page index of.
   * @returns The page index of the given entity.
   */
  constexpr
  static size_type page_index(entity_type const e)
  noexcept
  {
    return e / page_size;
  }


  /*!
   * @brief The line index (the index in its page) of the given entity.
   *
   * @param e The entity to get the line index of.
   * @returns The line index of the given entity.
   */
  constexpr
  static size_type line_index(entity_type const e)
  noexcept
  {
    return e % page_size;
  }


  /*!
   * @brief The index in the dense containers of the given entity.
   *
   * @param e The entity to get the index of.
   * @returns The index of the given entity.
   */
  constexpr
  size_type  index(entity_type const e) const
  noexcept
  {
    return (*pages_[page_index(e)])[line_index(e)];
  }

  /*!
   * @brief The index in the dense containers of the given entity.
   *
   * @param e The entity to get the index of.
   * @returns The index of the given entity.
   */
  constexpr
  size_type &index(entity_type const e)
  noexcept
  {
    return (*pages_[page_index(e)])[line_index(e)];
  }

  //! @endcond

public:
  constexpr
  component_map()
  = default;

  constexpr
  component_map(component_map const &other)
    : pages_     {},
      entities_  {other.entities_},
      components_{other.components_}
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
  component_map(component_map &&other)
  noexcept
  = default;


  constexpr
  ~component_map()
  noexcept
  = default;


  constexpr
  component_map &operator=(component_map const &other)
  {
    entities_   = other.entities_;
    components_ = other.components_;

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
  component_map &operator=(component_map &&other)
  noexcept
  = default;



  /*!
   * @brief The number of elements in the component_map.
   *
   * @returns The number of elements in the component_map.
   */
  [[nodiscard]]
  constexpr
  size_type size() const
  noexcept
  {
    return entities_.size();
  }


  /*!
   * @brief Checks whether the component_map is empty or not.
   *
   * @returns @c true if the component_map is empty, @c false otherwise.
   */
  [[nodiscard]]
  constexpr
  bool empty() const
  noexcept
  {
    return entities_.empty();
  }


  /*!
   * @brief The maximum number of entities the component_map can contain.
   *
   * @returns The maximum number of entities the component_map can contain.
   */
  [[nodiscard]]
  constexpr
  size_type max_size() const
  noexcept
  {
    return entities_.max_size();
  }


  /*!
   * @brief The maximum number of elements the component_map can contain without
   *   requiring a reallocation.
   *
   * @returns The maximum number of elements the component_map can contain without
   *   requiring a reallocation.
   */
  [[nodiscard]]
  constexpr
  size_type capacity() const
  noexcept
  {
    return entities_.capacity();
  }


  /*!
   * @brief Increases the capacity of the component_map to at least the given
   *   number, reallocating memory if needed.
   *
   * @param n The capacity to reach.
   */
  constexpr
  void reserve(size_type const n)
  {
    entities_  .reserve(n);
    components_.reserve(n);
  }


  /*!
   * @brief Shrinks the capacity of the component_map to its size, reallocating
   *   memory if needed.
   */
  constexpr
  void shrink_to_fit()
  {
    entities_.shrink_to_fit();

    try
    {
      components_.shrink_to_fit();
    }
    catch(...)
    {
      entities_.reserve(components_.capacity());
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
   * @brief The iterator to the first element of the component_map.
   *
   * @returns The iterator to the first element of the component_map.
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

  /*!
   * @brief The const iterator to the first element of the component_map.
   *
   * @returns The const iterator to the first element of the component_map.
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


  /*!
   * @brief The iterator to past the last element of the component_map.
   * @warning This iterator is not to be dereferenced.
   *
   * @returns The iterator to past the last element of the component_map.
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

  /*!
   * @brief The const iterator to past the last element of the component_map.
   * @warning This iterator is not to be dereferenced.
   *
   * @returns The const iterator to past the last element of the component_map.
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


  /*!
   * @brief The const iterator to the first element of the component_map.
   *
   * @returns The const iterator to the first element of the component_map.
   */
  [[nodiscard]]
  constexpr
  const_iterator cbegin() const
  noexcept
  {
    return const_iterator{
      entities_  .data(),
      components_.data()};
  }


  /*!
   * @brief The const iterator to past the last element of the component_map.
   * @warning This iterator is not to be dereferenced.
   *
   * @returns The const iterator to past the last element of the component_map.
   */
  [[nodiscard]]
  constexpr
  const_iterator cend() const
  noexcept
  {
    return const_iterator{
      entities_  .data() + size(),
      components_.data() + size()};
  }



  /*!
   * @brief Checks if the given entity is in the component_map.
   *
   * @param e The entity to check for.
   * @returns @c true if the given entity is in the component_map, @c false
   *   otherwise.
   */
  [[nodiscard]]
  constexpr
  bool contains(entity_type const e) const
  noexcept
  {
    return page_index(e) < pages_.size()
        && pages_[page_index(e)]
        && index(e) != null_index_
        && entities_[index(e)] == e;
  }


  /*!
   * @brief The iterator to the given entity, or end() if the entity is not
   *   in the component_map.
   *
   * @param e The entity to find.
   * @returns The iterator to the given entity, or end() if the entity is not
   *   in the component_map.
   */
  [[nodiscard]]
  constexpr
  iterator       find(entity_type const e)
  noexcept
  {
    if (!contains(e))
      return end();

    return iterator{
        entities_  .data() + index(e),
        components_.data() + index(e)};
  }

  /*!
   * @brief The const iterator to the given entity, or end() if the entity is
   *   not in the component_map.
   *
   * @param e The entity to find.
   * @returns The const iterator to the given entity, or end() if the entity is
   *   not in the component_map.
   */
  [[nodiscard]]
  constexpr
  const_iterator find(entity_type const e) const
  noexcept
  {
    if (!contains(e))
      return end();

    return const_iterator{
      entities_  .data() + index(e),
      components_.data() + index(e)};
  }


  /*!
   * @brief The component of the given entity.
   *
   * @param e The entity to get the component of.
   * @returns The component of the given entity.
   */
  [[nodiscard]]
  constexpr
  component_type       &operator[](entity_type const e)
  noexcept
  {
    return components_[index(e)];
  }

  /*!
   * @brief The component of the given entity.
   *
   * @param e The entity to get the component of.
   * @returns The component of the given entity.
   */
  [[nodiscard]]
  constexpr
  component_type const &operator[](entity_type const e) const
  noexcept
  {
    return components_[index(e)];
  }


  /*!
   * @brief The component of the given entity.
   *
   * @param e The entity to get the component of.
   * @returns The component of the given entity.
   * @exception std::out_of_range if the entity is not in the component_map.
   */
  [[nodiscard]]
  constexpr
  component_type       &at(entity_type const e)
  {
    if (!contains(e))
    {
      throw std::out_of_range{
          "heim::component_map::at(entity_type): out_of_range"};
    }
    return components_[index(e)];
  }

  /*!
   * @brief The component of the given entity.
   *
   * @param e The entity to get the component of.
   * @returns The component of the given entity.
   * @exception std::out_of_range if the entity is not in the component_map.
   */
  [[nodiscard]]
  constexpr
  component_type const &at(entity_type const e) const
  {
    if (!contains(e))
    {
      throw std::out_of_range{
        "heim::component_map::at(entity_type) const: out_of_range"};
    }
    return components_[index(e)];
  }


  /*!
   * @brief Clears all entities and erases all indexes from its pages.
   */
  constexpr
  void clear()
  noexcept
  {
    for (auto &page_uptr : pages_)
    {
      if (page_uptr)
        page_uptr->fill(null_index_);
    }

    entities_  .clear();
    components_.clear();
  }


  /*!
   * @brief Emplaces at the back of the component_map the given entity, with
   *   its component constructed using the given arguments.
   *
   * @param e The entity to emplace a component for.
   * @param args The arguments to construct the component.
   */
  template<typename ...Args>
  constexpr
  void emplace_back(entity_type const e, Args &&...args)
  {
    if (contains(e))
      return;

    if (page_index(e) >= pages_.size())
      pages_.resize(page_index(e) + 1);
    if (!pages_[page_index(e)])
    {
      pages_[page_index(e)] = std::make_unique<page_type>();
      pages_[page_index(e)] ->fill(null_index_);
    }

    components_.emplace_back(std::forward<Args>(args)...);
    try
    {
      entities_.emplace_back(e);
    }
    catch (...)
    {
      components_.pop_back();
    }
    index(e) = size() - 1;
  }


  /*!
   * @brief Removes the given entity and its component from the component_map,
   *   using the swap-and-pop method.
   *
   * @param e The entity to remove.
   */
  constexpr
  void pop_swap(entity_type const e)
  noexcept
  {
    if (!contains(e))
      return;

    if (e != entities_.back())
    {
      entity_type const e_back = entities_.back();

      entities_  [index(e)] = e_back;
      components_[index(e)] = std::move(components_.back());

      index(e_back) = index(e);
    }

    entities_  .pop_back();
    components_.pop_back();

    index(e) = null_index_;
  }


  /*!
   * @brief Swaps the contents of @c *this and the given component_map.
   *
   * @param other The other component_map whose contents to swap.
   */
  constexpr
  void swap(component_map &other)
  noexcept
  {
    pages_     .swap(other.pages_);
    entities_  .swap(other.entities_);
    components_.swap(other.components_);
  }

  /*!
   * @brief Swaps the given entities and their component.
   *
   * @param a The first entity whose place to swap.
   * @param b The second entity whose place to swap.
   */
  constexpr
  void swap(entity_type const a, entity_type const b)
  {
    if (!contains(a) || !contains(b))
      return;

    std::swap(entities_  [index(a)], entities_  [index(b)]);
    std::swap(components_[index(a)], components_[index(b)]);

    std::swap(index(a), index(b));
  }

};

/*!
 * @brief Swaps the contents of the given component_maps.
 *
 * @param lhs The first component_map whose contents to swap.
 * @param rhs The second component_map whose contents to swap.
 */
template<
    typename    Entity,
    typename    Component,
    std::size_t PageSize,
    typename    Allocator>
constexpr
void swap(
    component_map<Entity, Component, PageSize, Allocator> &lhs,
    component_map<Entity, Component, PageSize, Allocator> &rhs)
noexcept
{
  lhs.swap(rhs);
}


namespace detail
{
template<typename>
struct is_component_map
  : std::false_type
{ };

template<
    typename    Entity,
    typename    Component,
    std::size_t PageSize,
    typename    Allocator>
struct is_component_map<
    component_map<Entity, Component, PageSize, Allocator>>
  : std::true_type
{ };

}

template<typename T>
struct is_component_map
{
  constexpr
  static bool value
  = detail::is_component_map<std::remove_cvref_t<T>>::value;

};

template<typename T>
constexpr
inline bool is_component_map_v
= is_component_map<T>::value;

}

#endif // HEIM_COMPONENT_MAP_HPP
