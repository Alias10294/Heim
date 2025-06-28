#include "doctest.h"

#include "summoner.hpp"

TEST_CASE("summoner: creates entities")
{
  heim::summoner s;
  heim::entity e1 = s.summon();
  heim::entity e2 = s.summon();
  CHECK(e1 == 0);
  CHECK(e2 == 1);
}

TEST_CASE("summoner: recycles entities")
{
  heim::summoner s;
  heim::entity e1 = s.summon();
  s.banish(e1);
  heim::entity e2 = s.summon();
  CHECK(e1 == e2);
}