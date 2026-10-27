#include <heim/meta.hpp>
#include <heim/meta/meta.hpp>

#include <catch2/catch_test_macros.hpp>
#include <type_traits>

TEST_CASE("meta aggregate headers expose the type sequence interface", "[meta][header]")
{
  using sequence = heim::type_sequence<int, double>;
  STATIC_REQUIRE(heim::type_sequence_size_v<sequence> == 2);
  STATIC_REQUIRE((std::is_same_v<sequence::reverse, heim::type_sequence<double, int>>));
}
