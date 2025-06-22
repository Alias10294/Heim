#pragma once

#include "entity.hpp"
#include "composer.hpp"
#include "glimpse.hpp"
#include "summoner.hpp"

namespace heim
{
/**
 * @brief The central object of Heim.
 *
 * Holds all the data of your game world.
 */
class world
{
public:
  /**
   * @brief Summons a new entity in the game.
   *
   * @return The new entity to be used.
   */
  entity summon()
  {
    return summoner_.summon();
  }

  /**
   * @brief Destroy the entity and all of its components.
   */
  void destroy(const entity e)
  {
    summoner_.banish(e);
    composer_.clear(e);
  }


  /**
   * @brief Compose an entity of a new component
   *
   * @tparam Component The type of the component to compose with.
   * @param cmp       The predicate used to establish order in the composition.
   */
  template<typename Component>
  constexpr void compose(predicate<Component> cmp = nullptr)
  {
    composer_.compose<Component>(cmp);
  }


  /**
   * @brief Compose an entity of a new component
   *
   * @tparam Component The type of the new component.
   * @tparam Args      The type of the arguments for the new component.
   * @param e The entity to compose.
   * @param a The arguments for the new component.
   */
  template<typename Component, typename... Args>
  constexpr void compose(const entity e, Args&&... a)
  {
    composer_.compose<Component>(e, std::forward<Args>(a)...);
  }

  /**
   * @brief Erase the component(s) of given type(s) of the given entity.
   *
   * @tparam Components The types of the components to erase.
   * @param e The entity to erase components of.
   */
  template<typename... Components>
  constexpr void erase(const entity e)
  {
    composer_.erase<Components...>(e);
  }


  /**
   * @brief Returns a glimpse of the world concerning the given component
   * types.
   *
   * @tparam Components The components to catch a glimpse at.
   * @return The glimpse of the components.
   */
  template<typename... Components>
  basic_glimpse<Components...> glimpse()
  {
    return composer_.glimpse<Components...>();
  }


  /**
   * @brief Retrieves the component of given type of the given entity.
   *
   * @tparam Component The type of the component to get.
   * @param e The entity to retrieve the component of.
   * @return The component of the entity.
   */
  template<typename Component>
  Component& get(const entity e)
  {
    return composer_.get<Component>(e);
  }

  /**
   * @brief Checks if the given entity has component(s) of given type(s).
   *
   * @tparam Components The type(s) of the component(s) to check.
   * @param e The entity to check.
   * @return true if the entity has all the components, false otherwise.
   */
  template<typename... Components>
  constexpr bool has(const entity e) const
  {
    return composer_.has<Components...>(e);
  }

private:
  summoner summoner_{};
  composer composer_{};
};
}
