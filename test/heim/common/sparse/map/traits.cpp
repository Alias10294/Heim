#include <concepts>
#include <iterator>

#include "support.hpp"

using namespace heim_test::sparse_map;

static_assert(std::same_as<map<>::key_type, unsigned>);
static_assert(std::same_as<map<>::mapped_type, int>);
static_assert(std::same_as<map<>::value_type, std::pair<unsigned, int>>);
static_assert(std::same_as<map<>::reference, std::pair<unsigned const&, int&>>);
static_assert(std::same_as<map<>::const_reference, std::pair<unsigned const&, int const&>>);
static_assert(map<>::page_size == 8);
static_assert(map<unsigned, int, 0>::page_size == 0);
static_assert(std::is_copy_constructible_v<map<>>);
static_assert(std::is_move_constructible_v<map<>>);
static_assert(std::is_copy_assignable_v<map<>>);
static_assert(std::is_move_assignable_v<map<>>);
static_assert(std::same_as<decltype(*std::declval<map<>::iterator>()), map<>::reference>);
static_assert(std::same_as<decltype(*std::declval<map<>::const_iterator>()), map<>::const_reference>);
static_assert(std::is_constructible_v<map<>::const_iterator, map<>::iterator>);
static_assert(std::is_default_constructible_v<map<>::iterator>);

TEST_CASE("sparse_map advertised types and page sizes", "[common][sparse_map][traits]")
{
  STATIC_REQUIRE(map<>::page_size == 8);
  STATIC_REQUIRE(map<unsigned, int, 0>::page_size == 0);
  STATIC_REQUIRE(std::is_same_v<map<>::allocator_type, std::allocator<std::pair<unsigned, int>>>);
  STATIC_REQUIRE(std::is_same_v<std::iterator_traits<map<>::iterator>::difference_type, std::ptrdiff_t>);
}

TEST_CASE("sparse_map iterator exposes immutable key and mutable value", "[common][sparse_map][traits]")
{
  map<> m{{1u, 2}};
  static_assert(std::same_as<decltype((*m.begin()).first), unsigned const &>);
  static_assert(std::same_as<decltype((*m.begin()).second), int &>);
  static_assert(std::same_as<decltype((*std::as_const(m).begin()).second), int const &>);
  m.begin()->second = 5;
  REQUIRE(m.at(1u) == 5);
}

TEST_CASE("sparse_map mapped proxy retains address identity", "[common][sparse_map][traits]")
{
  map<> m{{4u, 40}, {8u, 80}};
  auto it = m.find(4u);
  REQUIRE(std::addressof(it->second) == std::addressof(m.at(4u)));
  it->second = 41;
  REQUIRE(m.at(4u) == 41);
}
