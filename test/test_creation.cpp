#include "doctest.h"
#include "creation.hpp"

TEST_CASE("heim::creation")
{
  using creation
  = heim::creation<std::size_t>
      ::component<int>
      ::component<float>
      ::sync<int, float>;

  creation c;
}