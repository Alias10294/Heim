#include "../derived/support.hpp"

using namespace heim_test::graph_algorithm::derived;

TEST_CASE("Connected components result can be constructed for a graph", "[graph][cc][result]")
{
  using G = graph<false>;
  hg::cc_result<G> result{std::in_place_type<G>};
  REQUIRE(result.component_count() == 0);
  REQUIRE(result.connected());
}

TEST_CASE("Connected components fill an existing result", "[graph][cc][result]")
{
  using G = graph<false>;
  G g;
  make_vertices(g, 5);
  add_edges(g, {{0, 1}, {3, 4}});
  hg::cc_result<G> result{std::in_place_type<G>};
  hg::cc(g, result);
  REQUIRE(result.component_count() == 3);
  REQUIRE(result.connected(at<G>(0), at<G>(1)));
  REQUIRE_FALSE(result.connected(at<G>(0), at<G>(2)));
}

TEST_CASE("Connected components result can be moved after computation", "[graph][cc][result]")
{
  using G = graph<false>;
  G g;
  make_vertices(g, 3);
  add_edges(g, {{0, 1}});
  auto first = hg::cc(g);
  auto moved = std::move(first);
  REQUIRE(moved.component_count() == 2);
  REQUIRE(moved.connected(at<G>(0), at<G>(1)));
  REQUIRE_FALSE(moved.connected(at<G>(1), at<G>(2)));
}

TEST_CASE("Connected components independent results own independent maps", "[graph][cc][result]")
{
  using G = graph<false>;
  G first;
  G second;
  make_vertices(first, 4);
  make_vertices(second, 4);
  add_edges(first, {{0, 1}, {1, 2}});
  add_edges(second, {{0, 3}});
  auto left = hg::cc(first);
  auto right = hg::cc(second);
  REQUIRE(left.component_count() == 2);
  REQUIRE(right.component_count() == 3);
  REQUIRE(left.connected(at<G>(0), at<G>(2)));
  REQUIRE_FALSE(right.connected(at<G>(0), at<G>(2)));
  REQUIRE(right.connected(at<G>(0), at<G>(3)));
}

TEST_CASE("Connected components expose the same result type through their alias", "[graph][cc][result]")
{
  using G = graph<false>;
  STATIC_REQUIRE(std::same_as<hg::cc_result<G>, hg::connected_components_result<G>>);
  STATIC_REQUIRE(std::same_as<decltype(hg::cc), decltype(hg::connected_components)>);
}
