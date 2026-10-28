#include <memory>

#include <catch2/catch_test_macros.hpp>

#include <heim/common.hpp>
#include <heim/common/common.hpp>

TEST_CASE("common aggregate headers expose sparse containers and allocated pointers", "[common][header]")
{
  heim::sparse_set<unsigned> keys;
  REQUIRE(keys.insert(7u).second);
  REQUIRE(keys.contains(7u));

  heim::sparse_map<unsigned, int> values;
  REQUIRE(values.try_emplace(7u, 14).second);
  REQUIRE(values.at(7u) == 14);

  auto pointer = heim::allocate_unique<int>(std::allocator<int>{}, 21);
  REQUIRE(*pointer == 21);
}
