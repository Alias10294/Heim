#include "doctest.h"
#include "creation.hpp"

TEST_CASE("heim::creation")
{
  using creation_t
  = heim::creation<std::size_t>
      ::component<int>
      ::component<float>
      ::sync<int, float>;

  creation_t my_creation;
}