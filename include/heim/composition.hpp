#pragma once

#include <vector>
#include <limits>
#include <cstddef>
#include <algorithm>
#include <numeric>
#include <memory>
#include <functional>

#include "entity.hpp"

namespace heim
{
template<typename Comp>
using predicate = std::function<bool(
    const std::remove_cvref_t<Comp>&,
    const std::remove_cvref_t<Comp>&)>;

using predicate_handle = std::function<bool(const void*, const void*)>;

/**
 * @brief An optimised associative container for the storing and access of
 * components.
 *
 * Bases its implementation off of sparse sets, providing great
 * cache-friendliness and promising O(1) insertion, deletion and access to
 * components.
 * This comes at the cost of memory overhead, having to contain a large vector
 * of indexes, though will some day be optimised maybe with pagination.
 *
 * @tparam Component The component type to hold.
 */
template<typename Component>
class composition
{
public:
  /// @brief A placeholder for null ids in the sparse vector.
  static constexpr std::size_t null_idx = std::numeric_limits<
    std::size_t>::max();


  /**
   * @brief Emplaces the component in the composition.
   *
   * @tparam Args The type of the arguments needed for the creation of the
   * component.
   * @param e The entity to emplace a component for.
   * @param a The arguments needed for the creation of the component.
   *
   * @note Uses emplace_back, but components may be rearranged when deletions
   * occur.
   */
  template<typename... Args>
  constexpr void emplace(const entity e, Args&&... a)
  {
    if (contains(e))
      return;
    if (e >= sparse_.size())
      reserve(std::max(e + 1ULL, sparse_.size() * 2ULL + 1ULL));

    sparse_.at(e) = entities_.size();
    entities_.emplace_back(e);
    components_.emplace_back(std::forward<Args>(a)...);
  }

  /**
   * @brief Erases the component of the given entity.
   *
   * Uses the swap-and-pop method, thus rearranging components in the
   * composition.
   *
   * @param e The entity to erase a component to.
   */
  constexpr void erase(const entity e) noexcept
  {
    if (!contains(e))
      return;

    entities_[sparse_[e]]     = entities_.back();
    components_[sparse_[e]]   = components_.back();
    sparse_[entities_.back()] = sparse_[e];

    sparse_[e] = null_idx;
    entities_.pop_back();
    components_.pop_back();
  }


  /**
   * @brief Retrieves the index in the dense vector of the given entity.
   *
   * @param e The entity to retrieve the index of.
   * @return The index of the given entity.
   */
  [[nodiscard]] constexpr std::size_t index(const entity e) const
  {
    return sparse_[e];
  }

  /**
   * @brief Retrieves the entity associated to the given index.
   *
   * @param idx The index to retrieve the corresponding entity of.
   * @return The entity corresponding to the given index.
   */
  [[nodiscard]] constexpr entity composed(std::size_t idx) const noexcept
  {
    return entities_[idx];
  }

  /**
   * @brief Retrieves the component associated to the given entity.
   *
   * @param e The entity to retrieve the component of.
   * @return The componenet of the given entity.
   */
  [[nodiscard]] constexpr Component& get(const entity e) noexcept
  {
    return components_[sparse_[e]];
  }

  /**
   * @brief Retrieves the component associated to the given entity.
   *
   * @param e The entity to retrieve the component of.
   * @return The componenet of the given entity.
   */
  [[nodiscard]] constexpr const Component& get(const entity e) const noexcept
  {
    return components_[sparse_[e]];
  }

  /**
   * @brief Returns the number of components in the composition.
   */
  [[nodiscard]] constexpr std::size_t size() const noexcept
  {
    return components_.size();
  }

  /**
   * @brief Increases the capacity of the composition, allocating memory
   * for a given amount of components.
   *
   * @param n The new capacity of the composition.
   */
  constexpr void reserve(std::size_t n)
  {
    sparse_.resize(n, null_idx);
    entities_.reserve(n);
    components_.reserve(n);
  }


  /**
   * @brief Checks if the entity has a component in the composition.
   *
   * @param e The entity to search for.
   * @return @c true if the entity has a component, @c false otherwise.
   */
  [[nodiscard]] constexpr bool contains(const entity e) const noexcept
  {
    return e < sparse_.size() &&
           sparse_[e] < entities_.size() &&
           entities_[sparse_[e]] == e;
  }


  /**
   * @brief Sorts the components of the composition using the given predicate.
   *
   * @param cmp The predicate given to order the components.
   */
  constexpr void sort(predicate<Component> cmp)
  {
    std::size_t n = components_.size();

    std::vector<std::size_t> order(n);
    std::iota(order.begin(), order.end(), 0ULL);
    std::sort(
        order.begin(),
        order.end(),
        [&](std::size_t lhs, std::size_t rhs)
        {
          return cmp(components_[lhs], components_[rhs]);
        });

    std::vector<entity> entities;
    entities.reserve(n);
    std::vector<Component> components;
    components.reserve(n);
    for (std::size_t idx : order)
    {
      entities.push_back(entities_[idx]);
      components.push_back(std::move(components_[idx]));
    }

    entities_   = std::move(entities);
    components_ = std::move(components);

    for (std::size_t i      = 0ULL; i < n; ++i)
      sparse_[entities_[i]] = i;
  }

private:
  std::vector<std::size_t> sparse_;
  std::vector<entity>      entities_;
  std::vector<Component>   components_;
};


/**
 * @brief The type-erased version of the composition.
 *
 * Allows for generalized and dynamic containment of all the compositions
 * in the composer.
 */
struct composition_handle
{
  std::unique_ptr<void, void(*)(void*)> ptr;

  /**
   * @brief Erases the component of the given entity.
   *
   * Uses the swap-and-pop method, thus rearranging components in the composition.
   *
   * @param e The entity to erase a component to.
   */
  std::function<void (const entity)> erase;

  /**
   * @brief Retrieves the index in the dense vector of the given entity.
   *
   * @param e The entity to retrieve the index of.
   * @return The index of the given entity.
   */
  std::function<std::size_t (const entity)> index;
  /**
   * @brief Retrieves the entity associated to the given index.
   *
   * @param idx The index to retrieve the corresponding entity of.
   * @return The entity corresponding to the given index.
   */
  std::function<entity (const std::size_t)> composed;
  /**
   * @brief Retrieves the component associated to the given entity.
   *
   * @param e The entity to retrieve the component of.
   * @return The componenet of the given entity.
   */
  std::function<void*(const entity)> get;
  /**
   * @brief Returns the number of components in the composition.
   */
  std::function<std::size_t ()> size;
  /**
   * @brief Increases the capacity of the composition, allocating memory
   * for a given amount of components.
   *
   * @param n The new capacity of the composition.
   */
  std::function<void (const std::size_t)> reserve;

  /**
   * @brief Checks if the entity has a component in the composition.
   *
   * @param e The entity to search for.
   * @return @c true if the entity has a component, @c false otherwise.
   */
  std::function<bool (const entity)> contains;

  /**
   * @brief Sorts the components of the composition using the given predicate.
   *
   * @tparam predicate The type of predicate given to order the components.
   * @param cmp The predicate given to order the components.
   */
  std::function<void (predicate_handle)> sort;
};

/**
 * @brief Returns a new type-erased composition.
 *
 * @tparam Component The composition to make a composition for.
 * @return The new type-erased composition for the given component.
 */
template<typename Component>
constexpr composition_handle make_handle() noexcept
{
  using Composition = composition<Component>;
  auto* rptr = new Composition();

  return composition_handle
  {
    std::unique_ptr<void, void(*)(void*)>{
      static_cast<void*>(rptr),
      [](void* p) { delete static_cast<Composition*>(p); }
    },
      [rptr](const entity e)
      {
        rptr->erase(e);
      },
      [rptr](const entity e)
      {
        return rptr->index(e);
      },
      [rptr](const std::size_t idx)
      {
        return rptr->composed(idx);
      },
      [rptr](const entity e)
      {
        return static_cast<void*>(std::addressof(rptr->get(e)));
      },
      [rptr]()
      {
        return rptr->size();
      },
      [rptr](const std::size_t n)
      {
        rptr->reserve(n);
      },

      [rptr](const entity e)
      {
        return rptr->contains(e);
      },
      [rptr](const predicate_handle& cmp)
      {
        rptr->sort(
            [cmp](const Component& lc, const Component& rc)
            {
              return cmp(&lc, &rc);
            });
      }
  };
}
}
