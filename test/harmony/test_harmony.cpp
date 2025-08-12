#include <cstdint>

#include "../doctest.h"
#include "../../include/harmony/harmony.hpp"
#include "../../include/composition/composition.hpp"

TEST_CASE("heim::harmony: ...")
{
  heim::composition<std::uint32_t, int>   c1;
  heim::composition<std::uint32_t, float> c2;

  heim::harmony<
      std::uint32_t,
      heim::composition<std::uint32_t, int>,
      heim::composition<std::uint32_t, float>> h{c1, c2};
  CHECK(h.length() == 0);

  for (std::size_t i = 0; i < 10; i++)
    c1.emplace(i * 2, i);
  for (std::size_t i = 0; i < 10; i++)
    c2.emplace(i * 3, i);

  h.include(0);
  h.include(6);
  h.include(12);
  h.include(18);
  CHECK(h.length() == 4);

  std::size_t cpt = 0;
  for (auto [e, c] : c1)
    if (cpt < 4) CHECK(e == cpt++ * 6);

  h.exclude(18);
  CHECK(h.length() == 3);
  for (auto [e, c] : c1)
    if (cpt < 3) CHECK(e == cpt++ * 6);

  h.include(18);
  CHECK(h.length() == 4);
  for (auto [e, c] : c1)
    if (cpt < 4) CHECK(e == cpt++ * 6);

  h.exclude(0);
  CHECK(h.length() == 3);
  for (auto [e, c] : c1)
  {
    switch (cpt++)
    {
    case 0:
      CHECK(e == 18);
      break;
    case 1:
      CHECK(e == 6);
      break;
    case 2:
      CHECK(e == 12);
      break;
    default:
      break;
    }
  }
}