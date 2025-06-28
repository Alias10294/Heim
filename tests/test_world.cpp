#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "world.hpp"
#include <vector>

using heim::world;
using heim::entity;

TEST_CASE("summon() renvoie des IDs uniques")
{
  world w;
  entity e1 = w.summon();
  entity e2 = w.summon();
  CHECK(e1 != e2);
}

TEST_CASE("destroy() réutilise l'ID")
{
  world w;
  entity e = w.summon();
  w.destroy(e);
  entity f = w.summon();
  CHECK(f == e);
}

TEST_CASE("compose<T>() enregistre le composant sans valeur")
{
  world w;
  w.compose<int>();               // on déclare qu'on veut un composant int
  entity e = w.summon();
  CHECK_FALSE(w.has<int>(e));     // aucun int attaché
}

TEST_CASE("compose<T>(e, value) / get<T>() / has<T>()")
{
  world w;
  w.compose<int>();
  entity e = w.summon();
  w.compose<int>(e, 123);
  REQUIRE(w.has<int>(e));
  CHECK(w.get<int>(e) == 123);
}

TEST_CASE("erase<T>() supprime un composant")
{
  world w;
  w.compose<float>();
  entity e = w.summon();
  w.compose<float>(e, 3.14f);
  REQUIRE(w.has<float>(e));
  w.erase<float>(e);
  CHECK_FALSE(w.has<float>(e));
}

TEST_CASE("glimpse<T>() itère correctement")
{
  world w;
  w.compose<int>();
  // on crée trois entités, mais on n'attache un int qu'à deux
  entity a = w.summon(); w.compose<int>(a, 10);
  w.summon();
  entity b = w.summon(); w.compose<int>(b, 20);

  auto gl = w.glimpse<int>();
  std::vector<entity> collected;
  for (auto id : gl)
    collected.push_back(id);

  CHECK(collected.size() == 2);
  CHECK(collected[0] == a);
  CHECK(collected[1] == b);
}
