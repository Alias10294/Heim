#ifndef HEIM_COMPOSITION_COMPOSER_HPP
#define HEIM_COMPOSITION_COMPOSER_HPP

#include <cstddef>
#include <typeindex>
#include <unordered_map>
#include <vector>
#include "core/component.hpp"
#include "core/entity.hpp"
#include "any_composition.hpp"

namespace heim
{
/**
 * @brief A generic container for compositions of all types.
 */
class composer
{
public:
  /**
   * @return @c true if the composer has no compositions, @c false otherwise.
   */
  [[nodiscard]]
  constexpr
  bool empty() const
  noexcept
  {
    return compositions_.empty();
  }

  /**
   * @return The number of held compositions.
   */
  [[nodiscard]]
  constexpr
  std::size_t size() const
  noexcept
  {
    return compositions_.size();
  }



  /**
   * @return An iterator to the first type-erased composition of the composer.
   */
  [[nodiscard]]
  constexpr
  std::vector<any_composition>::iterator       begin()
  noexcept
  {
    return compositions_.begin();
  }
  /**
   * @return A const iterator to the first type-erased composition of the
   *     composer.
   */
  [[nodiscard]]
  constexpr
  std::vector<any_composition>::const_iterator begin() const
  noexcept
  {
    return compositions_.begin();
  }

  /**
   * @return An iterator to after the last type-erased composition of the
   *     composer.
   *
   * @note This returned iterator only acts as a sentinel, and is not to be
   *     dereferenced.
   */
  [[nodiscard]]
  constexpr
  std::vector<any_composition>::iterator       end()
  noexcept
  {
    return compositions_.end();
  }
  /**
   * @return A const iterator to after the last type-erased composition of the
   *     composer.
   *
   * @note This returned const iterator only acts as a sentinel, and is not to
   *     be dereferenced.
   */
  [[nodiscard]]
  constexpr
  std::vector<any_composition>::const_iterator end() const
  noexcept
  {
    return compositions_.end();
  }


  /**
   * @return A const iterator to the first type-erased composition of the
   *     composer.
   */
  [[nodiscard]]
  constexpr
  std::vector<any_composition>::const_iterator cbegin() const
  noexcept
  {
    return compositions_.cbegin();
  }

  /**
   * @return A const iterator to after the last type-erased composition of the
   *     composer.
   *
   * @note This returned const iterator only acts as a sentinel, and is not to
   *     be dereferenced.
   */
  [[nodiscard]]
  constexpr
  std::vector<any_composition>::const_iterator cend() const
  noexcept
  {
    return compositions_.cend();
  }



  /**
   * @tparam Entity             The type of entities of the composition.
   * @tparam Component          The type of components of the composition.
   * @tparam PageSize           The size of the pages in the sparse container
   *     of the composition.
   * @tparam ComponentAllocator The type of allocator for the components of the
   *     composition.
   * @return The type index of a composition with such template arguments.
   */
  template<typename    Entity,
           typename    Component,
           std::size_t PageSize           = 4096,
           typename    ComponentAllocator = std::allocator<Component>>
  requires  core::entity<Entity>
        &&  core::component<Component>
        && (PageSize > 0)
        &&  std::same_as<
                typename std::allocator_traits<ComponentAllocator>::value_type,
                Component>
  [[nodiscard]]
  constexpr
  static std::type_index type()
  noexcept
  {
    return std::type_index{
        typeid(composition<Entity, Component, PageSize, ComponentAllocator>)};
  }

  /**
   * @tparam Entity             The type of entities of the composition.
   * @tparam Component          The type of components of the composition.
   * @tparam PageSize           The size of the pages in the sparse container
   *     of the composition.
   * @tparam ComponentAllocator The type of allocator for the components of the
   *     composition.
   * @return The index in the composer of the composition with such template
   *     arguments.
   */
  template<typename    Entity,
           typename    Component,
           std::size_t PageSize           = 4096,
           typename    ComponentAllocator = std::allocator<Component>>
  requires  core::entity<Entity>
        &&  core::component<Component>
        && (PageSize > 0)
        &&  std::same_as<
                typename std::allocator_traits<ComponentAllocator>::value_type,
                Component>
  [[nodiscard]]
  constexpr
  std::size_t index() const
  noexcept
  {
    return indexes_.find(
        type<Entity, Component, PageSize, ComponentAllocator>{})->second;
  }


  /**
   * @tparam Entity             The type of entities of the composition.
   * @tparam Component          The type of components of the composition.
   * @tparam PageSize           The size of the pages in the sparse container
   *     of the composition.
   * @tparam ComponentAllocator The type of allocator for the components of the
   *     composition.
   * @return @c true if the composer holds a composition with such template
   *     arguments.
   */
  template<typename    Entity,
           typename    Component,
           std::size_t PageSize           = 4096,
           typename    ComponentAllocator = std::allocator<Component>>
  requires  core::entity<Entity>
        &&  core::component<Component>
        && (PageSize > 0)
        &&  std::same_as<
                typename std::allocator_traits<ComponentAllocator>::value_type,
                Component>
  [[nodiscard]]
  constexpr
  bool holds() const
  noexcept
  {
    return indexes_.contains(
        type<Entity, Component, PageSize, ComponentAllocator>());
  }


  /**
   * @tparam Entity             The type of entities of the composition.
   * @tparam Component          The type of components of the composition.
   * @tparam PageSize           The size of the pages in the sparse container
   *     of the composition.
   * @tparam ComponentAllocator The type of allocator for the components of the
   *     composition.
   * @return A reference to the held composition with such template arguments.
   */
  template<typename    Entity,
           typename    Component,
           std::size_t PageSize           = 4096,
           typename    ComponentAllocator = std::allocator<Component>>
  requires  core::entity<Entity>
        &&  core::component<Component>
        && (PageSize > 0)
        &&  std::same_as<
                typename std::allocator_traits<ComponentAllocator>::value_type,
                Component>
  [[nodiscard]]
  constexpr
  composition<Entity, Component, PageSize, ComponentAllocator>
      &get()
  noexcept
  {
    return compositions_[
        index<Entity, Component, PageSize, ComponentAllocator>()].template
            get<Entity, Component, PageSize, ComponentAllocator>();
  }
  /**
   * @tparam Entity             The type of entities of the composition.
   * @tparam Component          The type of components of the composition.
   * @tparam PageSize           The size of the pages in the sparse container
   *     of the composition.
   * @tparam ComponentAllocator The type of allocator for the components of the
   *     composition.
   * @return A const reference to the held compositions with such template
   *     arguments.
   */
  template<typename    Entity,
           typename    Component,
           std::size_t PageSize           = 4096,
           typename    ComponentAllocator = std::allocator<Component>>
  requires  core::entity<Entity>
        &&  core::component<Component>
        && (PageSize > 0)
        &&  std::same_as<
                typename std::allocator_traits<ComponentAllocator>::value_type,
                Component>
  [[nodiscard]]
  constexpr
  composition<Entity, Component, PageSize, ComponentAllocator> const
      &get() const
  noexcept
  {
    return compositions_[
        index<Entity, Component, PageSize, ComponentAllocator>()].template
            get<Entity, Component, PageSize, ComponentAllocator>();
  }



  /**
   * @brief Composes the composer of a new composition with such template
   *     arguments.
   *
   * @tparam Entity             The type of entities of the composition.
   * @tparam Component          The type of components of the composition.
   * @tparam PageSize           The size of the pages in the sparse container
   *     of the composition.
   * @tparam ComponentAllocator The type of allocator for the components of the
   *     composition.
   * @tparam Args               The type of arguments to construct the
   *     composition.
   * @param args The arguments to construct the composition.
   * @return @c true if the composer is composed of the new composition,
   *     @c false otherwise.
   *
   * @note If an exception is thrown for any reason, this function has no
   *     effect (strong exception safety guarantee).
   */
  template<typename    Entity,
           typename    Component,
           std::size_t PageSize           = 4096,
           typename    ComponentAllocator = std::allocator<Component>,
           typename ...Args>
  requires  core::entity<Entity>
        &&  core::component<Component>
        && (PageSize > 0)
        &&  std::same_as<
                typename std::allocator_traits<ComponentAllocator>::value_type,
                Component>
  constexpr
  bool compose(Args &&...args)
  {
    if (holds<Entity, Component, PageSize, ComponentAllocator>())
      return false;

    indexes_.emplace(
        type<Entity, Component, PageSize, ComponentAllocator>(),
        compositions_.size());
    try
    {
      compositions_.emplace_back(
          std::in_place_type_t<
              composition<Entity, Component, PageSize, ComponentAllocator>>{},
          std::forward<Args>(args)...);
    }
    catch (...)
    {
      indexes_.erase(type<Entity, Component, PageSize, ComponentAllocator>());
      throw;
    }
    return true;
  }


  /**
   * @brief Removes the composition with such template arguments from the
   *     composer, if there is any.
   *
   * @tparam Entity             The type of entities of the composition.
   * @tparam Component          The type of components of the composition.
   * @tparam PageSize           The size of the pages in the sparse container
   *     of the composition.
   * @tparam ComponentAllocator The type of allocator for the components of the
   *     composition.
   * @return @c true if a composition has been removed, @c false otherwise.
   *
   * @warning This method uses a "swap-and-pop" algorithm to avoid moving
   *     multiple compositions. If some part of your program relies on the
   *     index of the last composition, you will have to update it (using the
   *     index method).
   */
  template<typename    Entity,
           typename    Component,
           std::size_t PageSize           = 4096,
           typename    ComponentAllocator = std::allocator<Component>>
  requires  core::entity<Entity>
        &&  core::component<Component>
        && (PageSize > 0)
        &&  std::same_as<
                typename std::allocator_traits<ComponentAllocator>::value_type,
                Component>
  constexpr
  bool decompose()
  noexcept
  {
    if (!holds<Entity, Component, PageSize, ComponentAllocator>())
      return false;

    std::size_t idx = index<Entity, Component, PageSize, ComponentAllocator>();
    if (idx != compositions_.size() - 1)
    {
      compositions_[idx] = std::move(compositions_.back());
      indexes_[std::type_index{compositions_.back().type()}] = idx;
    }

    compositions_.pop_back();
    indexes_.erase(type<Entity, Component, PageSize, ComponentAllocator>());

    return true;
  }


  /**
   * @brief Swaps the contents of @p *this and @code other@endcode.
   *
   * @param other The other composer whose contents to swap.
   */
  constexpr
  void swap(composer &other)
  noexcept
  {
    indexes_     .swap(other.indexes_     );
    compositions_.swap(other.compositions_);
  }

private:
  std::unordered_map<std::type_index, std::size_t> indexes_;
  std::vector<any_composition>                     compositions_;

};


/**
 * @brief Swaps the contents of @code lhs@endcode and @code rhs@endcode.
 *
 * @param lhs The first  composer whose contents to swap.
 * @param rhs The second composer whose contents to swap.
 */
constexpr
void swap(composer &lhs, composer &rhs)
noexcept
{
  lhs.swap(rhs);
}

}

#endif // HEIM_COMPOSITION_COMPOSER_HPP
