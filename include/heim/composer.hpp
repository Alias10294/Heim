#pragma once

#include <vector>
#include <unordered_map>
#include <typeindex>
#include <functional>
#include <cassert>

#include "entity.hpp"
#include "composition.hpp"
#include "glimpse.hpp"

namespace heim
{
/**
 * @brief Manages the components of the world.
 */
class composer
{
public:
  using composition_sorter = std::function<void()>;


  /**
   * @brief Compose with a new type of component.
   *
   * Also allows component to be automatically sorted using a given predicate.
   *
   * @tparam Comp The type of component to compose with.
   */
  template<typename Comp>
  constexpr void compose()
  {
    compose<Comp>(nullptr);
  }

  /**
   * @brief Compose with a new type of component.
   *
   * Also allows component to be automatically sorted using a given predicate.
   *
   * @tparam Comp The type of component to compose with.
   * @param cmp The predicate for automatic component sorting.
   */
  template<typename Comp>
  constexpr void compose(predicate<Comp> cmp)
  {
    using Component = std::remove_cvref_t<Comp>;

    if (composes<Component>())
    {
      std::size_t idx = index<Component>();

      sorters_[idx] = make_sorter<Component>(idx, cmp);
    }
    else
    {
      std::size_t idx = handles_.size();

      handles_.emplace_back(make_handle<Component>());

      if (cmp) sorters_.push_back(make_sorter<Component>(idx, cmp));
      else     sorters_.emplace_back(nullptr);

      indexes_[type_index<Component>()] = idx;
    }
  }


  /**
   * @brief Compose an entity of a new component
   *
   * @tparam Comp The type of the new component.
   * @tparam Args The type of the arguments for the new component.
   * @param e The entity to compose.
   * @param a The arguments for the new component.
   */
  template<typename Comp, typename... Args>
  constexpr void compose(const entity e, Args&&... a)
  {
    using Component = std::remove_cvref_t<Comp>;

    if (!composes<Component>())
      return;

    std::size_t idx = index<Component>();
    get_composition<Component>().emplace(e, std::forward<Args>(a)...);

    if (sorters_[idx])
      sorters_[idx]();
  }

  /**
   * @brief Erase the component(s) of given type(s) of the given entity.
   *
   * @tparam Comps The types of the components to erase.
   * @param e The entity to erase components of.
   */
  template<typename... Comps>
  // NOLINTNEXTLINE(clang-analyzer-optin.const)
  constexpr void erase(const entity e)
  {
    (erase_one<std::remove_cvref_t<Comps>>(e), ...);
  }

  /**
   * @brief Clears the given entity of any component.
   *
   * @param e The entity to clear.
   */
  constexpr void clear(const entity e)
  {
    for (std::size_t idx = 0ULL; idx < handles_.size(); ++idx)
    {
      handles_[idx].erase(e);
      if (sorters_[idx])
        sorters_[idx]();
    }
  }


  /**
   * @brief Returns a glimpse of the world concerning the given component types.
   *
   * @tparam Comps The components to catch a glimpse at.
   * @return The glimpse of the components.
   */
  template<typename... Comps>
  [[nodiscard]] basic_glimpse<Comps...> glimpse()
  {
    return basic_glimpse<Comps...>(
        get_composition<std::remove_cvref_t<Comps>>()...);
  }

  /**
   * @brief Retrieves the component of given type of the given entity.
   *
   * @tparam Comp The type of the component to get.
   * @param e The entity to retrieve the component of.
   * @return The component of the entity.
   */
  template<typename Comp>
  [[nodiscard]] Comp& get(const entity e)
  {
    using Component = std::remove_cvref_t<Comp>;

    composition_handle& ch = handles_[index<Component>()];
    return *static_cast<Component*>(ch.get(e));
  }


  /**
   * @brief Checks if the given entity has component(s) of given type(s).
   *
   * @tparam Comps The type(s) of the component(s) to check.
   * @param e The entity to check.
   * @return true if the entity has all the components, false otherwise.
   */
  template<typename... Comps>
  constexpr bool has(const entity e) const
  {
    return (has_one<std::remove_cvref_t<Comps>>(e) && ...);
  }

private:
  std::unordered_map<std::type_index, std::size_t> indexes_;

  std::vector<composition_handle> handles_;
  std::vector<composition_sorter> sorters_;


  /**
   * @brief Returns the type index of the given component type.
   *
   * @tparam Component The component to get the type index of.
   * @return The type index of the component.
   */
  template<typename Component>
  static constexpr std::type_index type_index() noexcept
  {
    return std::type_index(typeid(Component));
  }

  /**
   * @brief Returns the index of the composition of given component type.
   *
   * @tparam Component The component type of the composition to get the index
   * of.
   * @return The index of the concerned composition.
   */
  template<typename Component>
  constexpr std::size_t index() const
  {
    auto it = indexes_.find(type_index<Component>());
    assert(it != indexes_.end());
    return it->second;
  }


  /**
   * @brief Checks if the component of given type already has a composition.
   *
   * @tparam Component The component type to check for.
   * @returns true if the component already has a composition, false otherwise.
   */
  template<typename Component>
  constexpr bool composes() const
  {
    return indexes_.contains(type_index<Component>());
  }


  template<typename Component>
  constexpr composition_sorter make_sorter(
    std::size_t idx, predicate<Component> cmp) const noexcept
  {
    return [cmp, idx, this]()
    {
      handles_[idx].sort(
          [cmp](const void* lhsh, const void* rhsh) -> bool
          {
            const Component& lhs = *static_cast<const Component*>(lhsh);
            const Component& rhs = *static_cast<const Component*>(rhsh);
            return cmp(lhs, rhs);
          });
    };
  }


  /**
   * @brief Erase the component of given type of the given entity.
   *
   * @tparam Component The type of the component to erase.
   * @param e The entity to erase the component of.
   */
  template<typename Component>
  // NOLINTNEXTLINE(clang-analyzer-optin.const)
  constexpr void erase_one(const entity e)
  {
    if (!composes<Component>())
      return;

    std::size_t idx = index<Component>();
    handles_[idx].erase(e);

    if (sorters_[idx])
      sorters_[idx]();
  }

  /**
   * @brief Retrieves the composition of the given component type.
   *
   * @tparam Component The component type of the composition to retrieve.
   * @return The corresponding composition.
   */
  template<typename Component>
  constexpr composition<Component>& get_composition()
  {
    if (!composes<Component>())
      throw /*???*/;

    composition_handle& ch = handles_[index<Component>()];
    return *static_cast<composition<Component>*>(ch.ptr.get());
  }

  /**
   * @brief Checks if the given entity has a component of given type.
   *
   * @tparam Component The type of the component to check.
   * @param e The entity to check.
   * @return true if the entity has the component, false otherwise.
   */
  template<typename Component>
  constexpr bool has_one(const entity e) const
  {
    if (!composes<Component>())
      return false;

    const composition_handle& ch = handles_[index<Component>()];
    return ch.contains(e);
  }
};
}
