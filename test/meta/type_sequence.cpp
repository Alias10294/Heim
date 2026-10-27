#include <catch2/catch_test_macros.hpp>
#include <heim/meta.hpp>
#include <type_traits>

TEST_CASE("type_sequence operations preserve types", "[meta]")
{
  using input = heim::type_sequence<int, long, int>;
  static_assert(heim::type_sequence_size_v<input> == 3);
  static_assert(std::is_same_v<input::unique, heim::type_sequence<int, long>>);
  REQUIRE(true);
}
