#include "doctest.h"
#include "unsafe_any.hpp"
#include <string>
#include <memory>

TEST_CASE("heim::unsafe_any")
{
  heim::unsafe_any a{std::in_place_type<int>, 0};
  CHECK_EQ(*heim::unsafe_any_cast<int>(a), 0);
  CHECK_EQ(a.has_value(), true);

  a.reset();
  CHECK_EQ(a.has_value(), false);

  a = 0.0f;
  CHECK_EQ(*heim::unsafe_any_cast<float>(a), 0.0f);
  CHECK_EQ(a.has_value(), true);

  a = std::make_unique<int>(0);
  CHECK_EQ(**heim::unsafe_any_cast<std::unique_ptr<int>>(a), 0);
  CHECK_EQ(a.has_value(), true);

  a.emplace<std::string>("test");
  CHECK_EQ(*heim::unsafe_any_cast<std::string>(a), "test");
  CHECK_EQ(a.has_value(), true);

  *heim::unsafe_any_cast<std::string>(a) = "some large string name";
  CHECK_EQ(*heim::unsafe_any_cast<std::string>(a), "some large string name");
}