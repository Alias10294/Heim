#include "../doctest.h"
#include "../../include/composition/composer.hpp"

TEST_CASE("heim::composer")
{
  heim::composer composer;

  CHECK      (composer.compose(heim::composition<std::uint32_t, int>{}));
  CHECK_FALSE(composer.compose(heim::composition<std::uint32_t, int>{}));
  CHECK(composer.composes<heim::composition<std::uint32_t, int>>());
  CHECK(composer.index<heim::composition<std::uint32_t, int>>() == 0);

  CHECK(composer.index<heim::composition<std::uint32_t, float>>() == 1);
  CHECK_FALSE(composer.composes<heim::composition<std::uint32_t, float>>());
  CHECK(composer.compose(heim::composition<std::uint32_t, float>{}));
  CHECK(composer.composes<heim::composition<std::uint32_t, float>>());

  auto &c1 = composer.get<heim::composition<std::uint32_t, int>>();
  auto &c2 = composer.get<heim::composition<std::uint32_t, float>>();
  CHECK(c1.empty());
  CHECK(c2.empty());

  std::size_t cpt = 0;
  for (auto &ac : composer)
    cpt++;
  CHECK(cpt == 2);
}