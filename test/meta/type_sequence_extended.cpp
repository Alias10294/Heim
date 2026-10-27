#include <catch2/catch_test_macros.hpp>
#include <heim/meta.hpp>
#include <tuple>
#include <type_traits>

namespace {
template<class T> struct is_integral : std::is_integral<T> {};
template<class T> struct add_pointer { using type = T*; };
template<class A, class B> struct smaller_size : std::bool_constant<(sizeof(A) < sizeof(B))> {};
using seq = heim::type_sequence<char, int, char, long>;
}

TEST_CASE("type_sequence introspection is consistent", "[meta][compile]")
{
  static_assert(seq::size == 4);
  static_assert(!seq::empty);
  static_assert(seq::contains<int>);
  static_assert(!seq::contains<double>);
  static_assert(seq::count<char> == 2);
  static_assert(seq::count<int> == 1);
  static_assert(seq::count<double> == 0);
  static_assert(std::is_same_v<seq::front, char>);
  static_assert(std::is_same_v<seq::back, long>);
  static_assert(std::is_same_v<seq::get<1>, int>);
  static_assert(heim::type_sequence<>::empty);
  REQUIRE(true);
}

TEST_CASE("type_sequence structural transformations preserve ordering", "[meta][compile]")
{
  static_assert(std::is_same_v<seq::unique, heim::type_sequence<char, int, long>>);
  static_assert(std::is_same_v<seq::reverse, heim::type_sequence<long, char, int, char>>);
  static_assert(std::is_same_v<seq::pop_front, heim::type_sequence<int, char, long>>);
  static_assert(std::is_same_v<seq::pop_back, heim::type_sequence<char, int, char>>);
  static_assert(std::is_same_v<seq::take<2>, heim::type_sequence<char, int>>);
  static_assert(std::is_same_v<seq::drop<2>, heim::type_sequence<char, long>>);
  static_assert(std::is_same_v<seq::append<double>, heim::type_sequence<char, int, char, long, double>>);
  static_assert(std::is_same_v<seq::prepend<bool>, heim::type_sequence<bool, char, int, char, long>>);
  static_assert(std::is_same_v<seq::remove<char>, heim::type_sequence<int, long>>);
  REQUIRE(true);
}

TEST_CASE("type_sequence transform, filter and sort types at compile time", "[meta][compile]")
{
  using source = heim::type_sequence<long, char, int, short>;
  static_assert(std::is_same_v<source::transform<add_pointer>, heim::type_sequence<long*, char*, int*, short*>>);
  static_assert(std::is_same_v<heim::type_sequence<int, float, long, double>::filter<is_integral>,
                               heim::type_sequence<int, long>>);
  using sorted = source::sort<smaller_size>;
  static_assert(sorted::size == 4);
  static_assert(std::is_same_v<sorted::get<0>, char>);
  // int and long can have the same size (e.g. on MinGW64); the sort
  // comparator intentionally does not order equal-size types.
  static_assert(sizeof(sorted::get<0>) <= sizeof(sorted::get<1>));
  static_assert(sizeof(sorted::get<1>) <= sizeof(sorted::get<2>));
  static_assert(sizeof(sorted::get<2>) <= sizeof(sorted::get<3>));
  static_assert(sorted::contains<long>);
  static_assert(sorted::contains<int>);
  REQUIRE(true);
}
