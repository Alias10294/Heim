#include "../doctest.h"
#include "../../include/composition/any_composition.hpp"

TEST_CASE("heim::any_composition")
{
  heim::any_composition ac;

  CHECK      (ac.type() == typeid(void));
  CHECK_FALSE(ac.has_value());

  ac.emplace<heim::composition<std::uint32_t, int>>();
  CHECK(ac.has_value());
  CHECK(ac.type() == typeid(heim::composition<std::uint32_t, int>));

  auto &c = ac.get<heim::composition<std::uint32_t, int>>();
  CHECK(c.empty());

  ac.reset();
  CHECK      (ac.type() == typeid(void));
  CHECK_FALSE(ac.has_value());

  heim::composition<std::uint32_t, int> c2;
  c2.emplace(1, 2);
  ac.emplace<heim::composition<std::uint32_t, int>>(c2);
  CHECK(ac.get<heim::composition<std::uint32_t, int>>().contains(1));

  auto ac2 = ac;
  CHECK(ac2.get<heim::composition<std::uint32_t, int>>().contains(1));

  ac.do_erase<std::uint32_t>(1);
  CHECK_FALSE(ac.get<heim::composition<std::uint32_t, int>>().contains(1));
  CHECK(ac2.get<heim::composition<std::uint32_t, int>>().contains(1));

  heim::swap(ac, ac2);
  CHECK(ac.get<heim::composition<std::uint32_t, int>>().contains(1));
  CHECK_FALSE(ac2.get<heim::composition<std::uint32_t, int>>().contains(1));

  heim::any_composition ac3{
      std::in_place_type_t<heim::composition<std::uint32_t, int>>{},
      ac.get<heim::composition<std::uint32_t, int>>()};
  CHECK(ac3.get<heim::composition<std::uint32_t, int>>().contains(1));

}