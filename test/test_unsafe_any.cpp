#include "doctest.h"
#include "unsafe_any.hpp"
#include <memory>

TEST_CASE("heim::unsafe_any")
{
  heim::unsafe_any a{std::in_place_type<int>, 0};
  CHECK_EQ(*heim::unsafe_any_cast<int>(a), 0);

  a = 0.0f;
  CHECK_EQ(*heim::unsafe_any_cast<float>(a), 0.0f);

  a = std::make_unique<int>(0);
  CHECK_EQ(**heim::unsafe_any_cast<std::unique_ptr<int>>(a), 0);
}