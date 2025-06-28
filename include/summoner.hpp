#pragma once

#include <forward_list>

#include "entity.hpp"

namespace heim
{
/**
 * @brief Manages summoning and banished entities.
 *
 * Implements a lightweight freelist for banished entities to allow for
 * reuse of IDs, avoiding exhaustion.
 */
class summoner
{
public:
  /**
   * @brief Summons or unbanishes an entity for new use.
   *
   * Reuses a banished entity if it can, otherwise generate the next entity.
   *
   * @return The summoned entity.
   */
  entity summon()
  {
    if (banished_.empty())
      return next_++;

    entity e = banished_.front();
    banished_.pop_front();
    return e;
  }

  /**
   * @brief Banishes an entity for the world.
   *
   * Sends the entity in the banished freelist, waiting to be reused.
   *
   * @param e The entity to banish.
   */
  void banish(const entity e)
  {
    banished_.push_front(e);
  }

private:
  entity                    next_{ 0 };
  std::forward_list<entity> banished_{};
};
}
