#include <cstdint>
#include "../doctest.h"
#include "../../include/harmony/any_harmony.hpp"
#include "composition/composition.hpp"

TEST_CASE("heim::any_harmony: ...")
{
  heim::any_harmony ah;
  CHECK      (ah.type() == typeid(void));
  CHECK_FALSE(ah.has_value());

  heim::composition<std::uint32_t, int>   c1;
  for (std::size_t i = 0; i < 10; i++)
    c1.emplace(i * 2, i);

  heim::composition<std::uint32_t, float> c2;
  for (std::size_t i = 0; i < 10; i++)
    c2.emplace(i * 3, i);

  ah.emplace<heim::harmony<
      std::uint32_t,
      heim::composition<std::uint32_t, int>,
      heim::composition<std::uint32_t, float>>>(c1, c2);
  CHECK(ah.has_value());
  CHECK(ah.type() == typeid(heim::harmony<
      std::uint32_t,
      heim::composition<std::uint32_t, int>,
      heim::composition<std::uint32_t, float>>));

  auto &h = ah.get<heim::harmony<
      std::uint32_t,
      heim::composition<std::uint32_t, int>,
      heim::composition<std::uint32_t, float>>>();
  CHECK(h.length() == 0);

  ah.reset();
  CHECK      (ah.type() == typeid(void));
  CHECK_FALSE(ah.has_value());

  heim::harmony<
      std::uint32_t,
      heim::composition<std::uint32_t, int>,
      heim::composition<std::uint32_t, float>> h2{c1, c2};
  h2.include(0);
  ah.emplace<decltype(h2)>(h2);
  CHECK(ah.get<decltype(h2)>().length() == 1);

  auto ah2 = ah;
  CHECK(ah2.get<decltype(h2)>().length() == 1);

  heim::swap(ah, ah2);
}