#ifndef HEIM_COMPONENT_COMPOSER_HPP
#define HEIM_COMPONENT_COMPOSER_HPP

#include <concepts>
#include <cstddef>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>
#include "any_composition.hpp"

namespace heim
{
/**
 * @brief Manages unique instances of any type of composition of components.
 *
 * @tparam Entity The type of entities for all compositions of this composer.
 */
template<typename Entity>
requires std::unsigned_integral<Entity>
class composer
{
private:
  using entity_type = Entity;

  using composition_container_type = std::vector<any_composition>;
  using index_container_type       =
      std::unordered_map<std::type_index, std::size_t>;

public:
  using iterator       = composition_container_type::iterator;
  using const_iterator = composition_container_type::const_iterator;

public:
  /**
   * @return @c true if the container contains no composition, @c false
   *     otherwise.
   */
  [[nodiscard]]
  constexpr
  bool empty() const
  noexcept
  {
    return compositions_.empty();
  }

  /**
   * @return The number of compositions contained.
   */
  [[nodiscard]]
  constexpr
  std::size_t size() const
  noexcept
  {
    return compositions_.size();
  }



  /**
   * @return An iterator to the first type-erased composition in the container.
   */
  [[nodiscard]]
  constexpr
  iterator       begin()
  noexcept
  {
    return compositions_.begin();
  }
  /**
   * @return A const iterator to the first type-erased composition in the
   *     container.
   */
  [[nodiscard]]
  constexpr
  const_iterator begin() const
  noexcept
  {
    return compositions_.begin();
  }

  /**
   * @return An iterator to after the last type-erased composition in the
   *     container.
   *
   * @warning This iterator only acts as a sentinel, and is not to be
   *     dereferenced.
   */
  [[nodiscard]]
  constexpr
  iterator       end()
  noexcept
  {
    return compositions_.end();
  }
  /**
   * @return A const iterator to after the last type-erased composition in the
   *     container.
   *
   * @warning This const iterator only acts as a sentinel, and is not to be
   *     dereferenced.
   */
  [[nodiscard]]
  constexpr
  const_iterator end() const
  noexcept
  {
    return compositions_.end();
  }


  /**
   * @return A const iterator to the first type-erased composition in the
   *     container.
   */
  [[nodiscard]]
  constexpr
  const_iterator cbegin() const
  noexcept
  {
    return compositions_.cbegin();
  }

  /**
   * @return A const iterator to after the last type-erased composition in the
   *     container.
   *
   * @warning This const iterator only acts as a sentinel, and is not to be
   *     dereferenced.
   */
  [[nodiscard]]
  constexpr
  const_iterator cend() const
  noexcept
  {
    return compositions_.cend();
  }


  /**
   * @tparam Component          The type of component of the composition.
   * @tparam PageSize           The size of each page in the sparse container
   *     of the composition.
   * @tparam ComponentAllocator The allocator for the components of the
   *     composition.
   * @return A type_index of the composition with these template arguments.
   */
  template<typename    Component,
           std::size_t PageSize           = 4096,
           typename    ComponentAllocator = std::allocator<Component>>
  requires  std::is_copy_constructible_v<Component>
        &&  std::is_copy_assignable_v   <Component>
        && (PageSize > 0)
  [[nodiscard]]
  constexpr
  static std::type_index type_index()
  noexcept
  {
    return std::type_index{typeid(
        composition<entity_type, Component, PageSize, ComponentAllocator>)};
  }

  /**
   * @tparam Component          The type of component of the composition.
   * @tparam PageSize           The size of each page in the sparse container
   *     of the composition.
   * @tparam ComponentAllocator The allocator for the components of the
   *     composition.
   * @return The index of the composition with given template arguments in the
   *     composer.
   */
  template<typename    Component,
           std::size_t PageSize           = 4096,
           typename    ComponentAllocator = std::allocator<Component>>
  requires  std::is_copy_constructible_v<Component>
        &&  std::is_copy_assignable_v   <Component>
        && (PageSize > 0)
  [[nodiscard]]
  constexpr
  std::size_t index() const
  noexcept
  {
    return indexes_.find(
        type_index<Component, PageSize, ComponentAllocator>())->second;
  }

  /**
   * @tparam Component          The type of component of the composition.
   * @tparam PageSize           The size of each page in the sparse container
   *     of the composition.
   * @tparam ComponentAllocator The allocator for the components of the
   *     composition.
   * @return @c true if a composition with such template arguments is
   *     contained, @c false otherwise.
   */
  template<typename    Component,
           std::size_t PageSize           = 4096,
           typename    ComponentAllocator = std::allocator<Component>>
  requires  std::is_copy_constructible_v<Component>
        &&  std::is_copy_assignable_v   <Component>
        && (PageSize > 0)
  [[nodiscard]]
  constexpr
  bool holds() const
  noexcept
  {
    return indexes_.contains(
        type_index<Component, PageSize, ComponentAllocator>());
  }


  /**
   * @tparam Component          The type of component of the composition.
   * @tparam PageSize           The size of each page in the sparse container
   *     of the composition.
   * @tparam ComponentAllocator The allocator for the components of the
   *     composition.
   * @return A reference to the contained composition with such template
   *     arguments.
   */
  template<typename    Component,
           std::size_t PageSize           = 4096,
           typename    ComponentAllocator = std::allocator<Component>>
  requires  std::is_copy_constructible_v<Component>
        &&  std::is_copy_assignable_v   <Component>
        && (PageSize > 0)
  [[nodiscard]]
  constexpr
  composition<entity_type, Component, PageSize, ComponentAllocator>
      &get()
  noexcept
  {
    return compositions_[
        index<Component, PageSize, ComponentAllocator>()].template
            get<entity_type, Component, PageSize, ComponentAllocator>();
  }
  /**
   * @tparam Component          The type of component of the composition.
   * @tparam PageSize           The size of each page in the sparse container
   *     of the composition.
   * @tparam ComponentAllocator The allocator for the components of the
   *     composition.
   * @return A const reference to the contained composition with such template
   *     arguments.
   */
  template<typename    Component,
           std::size_t PageSize           = 4096,
           typename    ComponentAllocator = std::allocator<Component>>
  requires  std::is_copy_constructible_v<Component>
        &&  std::is_copy_assignable_v   <Component>
        && (PageSize > 0)
  [[nodiscard]]
  constexpr
  composition<entity_type, Component, PageSize, ComponentAllocator> const
      &get() const
  noexcept
  {
    return compositions_[
        index<Component, PageSize, ComponentAllocator>()].template
            get<entity_type, Component, PageSize, ComponentAllocator>();
  }



  /**
   * @brief Compose a new composition with the given template arguments in the
   *     composer, if one does not already exist, and forwarding the arguments
   *     @code args@endcode to the constructor of the composition as
   *     @code std::forward<Args>(args)...@endcode.
   *
   * @tparam Component          The type of component of the composition.
   * @tparam PageSize           The size of each page in the sparse container
   *     of the composition.
   * @tparam ComponentAllocator The allocator for the components of the
   *     composition.
   * @tparam Args               The type of arguments to construct the
   *     composition.
   * @param args The arguments to construct the composition.
   * @return @c true if the composition has been added, @c false otherwise.
   */
  template<typename    Component,
           std::size_t PageSize           = 4096,
           typename    ComponentAllocator = std::allocator<Component>,
           typename ...Args>
  requires  std::is_copy_constructible_v<Component>
        &&  std::is_copy_assignable_v   <Component>
        && (PageSize > 0)
  constexpr
  bool compose(Args &&...args)
  {
    std::type_index const type_idx =
        type_index<Component, PageSize, ComponentAllocator>();

    if (!indexes_.try_emplace(type_idx, compositions_.size()).second)
      return false;

    try
    {
      compositions_.emplace_back(std::in_place_type_t<
          composition<entity_type, Component, PageSize, ComponentAllocator>>{},
          std::forward<Args>(args)...);
    }
    catch (...)
    {
      indexes_.erase(type_idx);
      throw;
    }
    return true;
  }


  /**
   * @brief Erases the contained composition with given template arguments, if
   *     one is contained.
   *
   * @tparam Component          The type of component of the composition.
   * @tparam PageSize           The size of each page in the sparse container
   *     of the composition.
   * @tparam ComponentAllocator The allocator for the components of the
   *     composition.
   * @return @c true if a composition has been erased, @c false otherwise.
   *
   * @warning This method uses a @a swap-and-pop algorithm to induce
   *     constant-time complexity for the operation. This comes at the cost of
   *     breaking the current order of the elements in the container, and
   *     invalidating the index of the previous last composition. Thus, every
   *     entity's signature in the world needs to be updated as well.
   */
  template<typename    Component,
           std::size_t PageSize           = 4096,
           typename    ComponentAllocator = std::allocator<Component>>
  requires  std::is_copy_constructible_v<Component>
        &&  std::is_copy_assignable_v   <Component>
        && (PageSize > 0)
  constexpr
  bool remove()
  noexcept
  {
    if (!holds<Component, PageSize, ComponentAllocator>())
      return false;

    std::size_t const idx =
        index<Component, PageSize, ComponentAllocator>();

    if (idx != compositions_.size() - 1)
    {
      indexes_[std::type_index{compositions_.back().type()}] = idx;
      compositions_[idx] = std::move(compositions_.back());
    }

    indexes_.erase(
        type_index<Component, PageSize, ComponentAllocator>());
    compositions_.pop_back();

    return true;
  }



  /**
   * @brief Swaps the contents of @p *this and @code other@endcode.
   *
   * @param other The other composer to swap the contents of.
   */
  constexpr
  void swap(composer &other)
  noexcept
  {
    compositions_.swap(other.compositions_);
    indexes_     .swap(other.indexes_);
  }

private:
  composition_container_type compositions_;
  index_container_type       indexes_;

};


/**
 * @brief Swaps the contents of @code lhs@endcode and @code rhs@endcode.
 *
 * @tparam Entity The type of entities of the composers.
 * @param lhs The first  composer to swap the contents of.
 * @param rhs The second composer to swap the contents of.
 */
template<typename Entity>
constexpr
void swap(composer<Entity> &lhs, composer<Entity> &rhs)
noexcept
{
  lhs.swap(rhs);
}

}

#endif // HEIM_COMPONENT_COMPOSER_HPP
