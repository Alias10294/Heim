#include "doctest.h"
#include "raw_any.hpp"
#include <memory>

TEST_CASE("heim::raw_any")
{
  heim::raw_any a{std::in_place_type<int>, 0};
  CHECK_EQ(*heim::raw_any_cast<int>(a), 0);

  a = 0.0f;
  CHECK_EQ(*heim::raw_any_cast<float>(a), 0.0f);

  a = std::make_unique<int>(0);
  CHECK_EQ(**heim::raw_any_cast<std::unique_ptr<int>>(a), 0);
}