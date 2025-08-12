#ifndef HEIM_ENTITY_SUMMONER_HPP
#define HEIM_ENTITY_SUMMONER_HPP

#include <cstddef>
#include <stdexcept>
#include <vector>
#include "entity.hpp"

namespace heim
{
/**
 * @brief Manages the summoning, banishing, and recycling of entities for the
 *     world.
 */
template<typename Entity>
requires core::entity<Entity>
class summoner
{
public:
  using entity_type = Entity;

public:
  /**
   * @brief Increases the number of banished entities the summoner can hold
   * without requiring reallocation.
   *
   * @param new_cap The new capacity of the container, in number of elements.
   * @throw std::length_error if the capacity exceeds the banished container's
   *     maximum size.
   */
  constexpr
  void reserve(std::size_t const new_cap)
  {
    banished_.reserve(new_cap);
  }



  /**
   * @brief Summons a newly generated entity, or unbanishes an entity.
   *
   * @return The summoned entity.
   */
  [[nodiscard]]
  constexpr
  entity_type summon()
  noexcept
  {
    if (banished_.empty())
      return generated_++;

    entity_type const e = banished_.back();
    banished_.pop_back();
    return e;
  }


  /**
   * @brief Banishes an entity, considering it removed from the world, and
   *     readies it to be summoned once again.
   *
   * @param e The entity to banish.
   */
  constexpr
  void banish(entity_type const e)
  noexcept
  {
    banished_.push_back(e);
  }

private:
  entity_type              generated_ = 0;
  std::vector<entity_type> banished_;

};

}

#endif // HEIM_ENTITY_SUMMONER_HPP
