#include "support.hpp"

#include <heim/property/property_map.hpp>

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <compare>
#include <concepts>
#include <map>
#include <ranges>
#include <vector>

namespace
{
struct vertex
{
  int id{};
  auto operator<=>(vertex const &) const = default;
};

struct vertex_predecessor_map
{
  std::array<vertex, 4> const *parents{};

  vertex get(vertex v) const { return parents->at(static_cast<std::size_t>(v.id)); }
};

using vertex_view = heim::graphs::predecessor_view<vertex_predecessor_map, vertex>;
static_assert(std::ranges::input_range<vertex_view>);
static_assert(std::same_as<std::ranges::range_value_t<vertex_view>, vertex>);
}

TEST_CASE("predecessor_view supports value-semantic custom vertex descriptors",
          "[graph][algorithm][predecessor_view][customization]")
{
  std::array<vertex, 4> const parents{vertex{0}, vertex{0}, vertex{1}, vertex{2}};
  vertex_view path{vertex_predecessor_map{&parents}, vertex{3}};

  std::vector<vertex> collected;
  for (auto v : path)
    collected.push_back(v);

  REQUIRE((collected == std::vector<vertex>{vertex{3}, vertex{2}, vertex{1}, vertex{0}}));
}

TEST_CASE("predecessor_view accepts a property map made from a standard associative container",
          "[graph][algorithm][predecessor_view][customization]")
{
  std::map<int, int> predecessors{{0, 0}, {1, 0}, {2, 1}, {3, 2}};
  auto map = heim::make_property_map(predecessors);
  heim::graphs::predecessor_view<decltype(map), int> path{map, 3};

  std::vector<int> collected;
  for (int v : path)
    collected.push_back(v);

  REQUIRE((collected == std::vector<int>{3, 2, 1, 0}));
}
