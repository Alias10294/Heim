#include "../derived/support.hpp"

using namespace heim_test::graph_algorithm::derived;

namespace
{
using Undirected = graph<false>;
using Directed = graph<true>;
using ExternalUndirected = external_graph<false>;
using ExternalDirected = external_graph<true>;
}

TEST_CASE("Connected components require an undirected graph", "[graph][cc][traits]")
{
  STATIC_REQUIRE(std::invocable<decltype(hg::cc), Undirected const &>);
  STATIC_REQUIRE(std::invocable<decltype(hg::cc), ExternalUndirected const &>);
  STATIC_REQUIRE_FALSE(std::invocable<decltype(hg::cc), Directed const &>);
  STATIC_REQUIRE_FALSE(std::invocable<decltype(hg::cc), ExternalDirected const &>);
}

TEST_CASE("Connected components aliases accept the same valid graph", "[graph][cc][traits]")
{
  STATIC_REQUIRE(std::invocable<decltype(hg::connected_components), Undirected const &>);
  STATIC_REQUIRE(std::same_as<decltype(hg::cc), decltype(hg::connected_components)>);
}
