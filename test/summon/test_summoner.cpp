#include <cstdint>
#include "../doctest.h"
#include "../../include/summon/summoner.hpp"

TEST_CASE("heim::summoner")
{
  heim::summoner<std::uint32_t> summoner;

  CHECK(summoner.summon() == 0);
  CHECK(summoner.summon() == 1);
  CHECK(summoner.summon() == 2);
  CHECK(summoner.summon() == 3);

  summoner.banish(0);
  CHECK(summoner.summon() == 0);
  CHECK(summoner.summon() == 4);

  summoner.banish(0);
  summoner.banish(1);
  summoner.banish(2);
  summoner.banish(3);
  summoner.banish(4);

  CHECK(summoner.summon() == 4);
  CHECK(summoner.summon() == 3);
  CHECK(summoner.summon() == 2);
  CHECK(summoner.summon() == 1);
  CHECK(summoner.summon() == 0);

  summoner.banish(2);
  CHECK(summoner.summon() == 2);
  CHECK(summoner.summon() == 5);
}