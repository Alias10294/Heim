#include "doctest.h"
#include "registry.hpp"

TEST_CASE("heim::registry")
{
  using registry_t
  = heim::registry<std::size_t>
      ::component<int>
      ::component<float>
      ::component<short>
      ::sync<int, float>;

  registry_t my_registry;
}