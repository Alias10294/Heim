#include <cstdint>
#include "../doctest.h"
#include "../../include-old/composition/composition.hpp"
#include "../../include-old/harmony/harmonizer.hpp"

TEST_CASE("heim::harmonizer")
{
  heim::harmonizer harmonizer;
  CHECK(harmonizer.harmonize(heim::harmony<
      std::uint32_t,
      heim::composition<std::uint32_t, int>,
      heim::composition<std::uint32_t, float>>{}));
  CHECK_FALSE(harmonizer.harmonize(heim::harmony<
      std::uint32_t,
      heim::composition<std::uint32_t, int>,
      heim::composition<std::uint32_t, float>>{}));
  CHECK(harmonizer.harmonizes<heim::harmony<
      std::uint32_t,
      heim::composition<std::uint32_t, int>,
      heim::composition<std::uint32_t, float>>>());
  CHECK(harmonizer.harmonizes<heim::composition<std::uint32_t, int>>());
  CHECK(harmonizer.harmonizes<heim::composition<std::uint32_t, float>>());

  CHECK(harmonizer.index<heim::harmony<
      std::uint32_t,
      heim::composition<std::uint32_t, int>,
      heim::composition<std::uint32_t, float>>>() == 0);
  CHECK(harmonizer.index<heim::composition<std::uint32_t, int>>() == 0);
  CHECK(harmonizer.index<heim::composition<std::uint32_t, float>>() == 1);

  CHECK(harmonizer.index<heim::composition<std::uint32_t, unsigned>>() == 2);
  CHECK_FALSE(harmonizer.harmonizes<
      heim::composition<std::uint32_t, unsigned>>());

  auto &h = harmonizer.get<heim::harmony<
      std::uint32_t,
      heim::composition<std::uint32_t, int>,
      heim::composition<std::uint32_t, float>>>();
  CHECK(h.length() == 0);

  auto const &ch = harmonizer.get<heim::harmony<
      std::uint32_t,
      heim::composition<std::uint32_t, int>,
      heim::composition<std::uint32_t, float>>>();
  CHECK(ch.length() == 0);

  std::size_t cpt = 0;
  for (auto &ah : harmonizer)
    cpt++;
  CHECK(cpt == 1);
}