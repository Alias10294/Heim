#include <catch2/catch_test_macros.hpp>
#include <heim/meta.hpp>
#include <tuple>
#include <type_traits>

TEST_CASE("type_sequence selection and erasure preserve selected index order", "[meta][compile]")
{
  using s = heim::type_sequence<char, short, int, long>;
  static_assert(std::is_same_v<s::select<3, 1>, heim::type_sequence<long, short>>);
  static_assert(std::is_same_v<s::erase<1, 3>, heim::type_sequence<char, int>>);
  static_assert(std::is_same_v<s::slice<1, 3>, heim::type_sequence<short, int>>);
  static_assert(std::is_same_v<s::insert<2, bool>, heim::type_sequence<char, short, bool, int, long>>);
  static_assert(std::is_same_v<s::replace<1, bool>, heim::type_sequence<char, bool, int, long>>);
  REQUIRE(true);
}
