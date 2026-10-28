#include "support.hpp"

#include <catch2/catch_test_macros.hpp>

#include <concepts>
#include <iterator>
#include <ranges>
#include <type_traits>

using namespace heim_test::graph_algorithm::predecessor;

static_assert(std::ranges::view<view>);
static_assert(std::ranges::input_range<view>);
static_assert(std::ranges::input_range<view const>);
static_assert(std::input_iterator<view::iterator>);
static_assert(std::sentinel_for<std::default_sentinel_t, view::iterator>);
static_assert(std::same_as<std::ranges::range_value_t<view>, int>);
static_assert(std::same_as<std::ranges::range_reference_t<view>, int>);
static_assert(std::same_as<std::ranges::range_difference_t<view>, std::ptrdiff_t>);
static_assert(std::same_as<decltype(std::declval<view const &>().end()), std::default_sentinel_t>);
static_assert(std::default_initializable<view::iterator>);
static_assert(std::copy_constructible<view::iterator>);
static_assert(std::default_initializable<view>);
static_assert(!std::ranges::sized_range<view>);

TEST_CASE("predecessor_view models an input view with default sentinel",
          "[graph][algorithm][predecessor_view][concepts]")
{
  auto parents = chain();
  view path{parent_map{&parents}, 7};
  auto it = path.begin();

  REQUIRE(*it == 7);
  REQUIRE(it != std::default_sentinel);
  REQUIRE(std::ranges::distance(path) == 5);
}

TEST_CASE("predecessor_view default iterator is an end iterator",
          "[graph][algorithm][predecessor_view][concepts]")
{
  view::iterator it;
  REQUIRE(it == std::default_sentinel);
}
