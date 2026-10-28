#include "../derived/reference.hpp"

using namespace heim_test::graph_algorithm::derived;

// Result objects are one-shot reports. A new graph state gets a new result.
TEST_CASE("CC computes a changed graph with a fresh result", "[graph][cc][regression]")
{
  using G = graph<false>;
  G g;
  make_vertices(g, 5);
  add_edges(g, {{0, 1}, {3, 4}});

  {
    auto result = hg::cc(g);
    REQUIRE(result.component_count() == 3);
    REQUIRE_FALSE(result.connected());
  }

  add_edges(g, {{1, 2}, {2, 3}});

  {
    auto result = hg::cc(g);
    REQUIRE(result.component_count() == 1);
    REQUIRE(result.connected());
    REQUIRE(result.connected(at<G>(0), at<G>(4)));
  }
}

TEST_CASE("CC operates on surviving non-contiguous vertex identifiers", "[graph][cc][regression]")
{
  using G = graph<false, true>;
  G g;
  make_vertices(g, 8);
  add_edges(g, {{0, 1}, {1, 2}, {3, 4}, {5, 6}, {6, 7}});
  g.remove_vertex(at<G>(1));
  g.remove_edges(at<G>(3), at<G>(4));
  g.remove_vertex(at<G>(6));
  add_edges(g, {{0, 2}, {3, 5}, {5, 7}});

  auto result = hg::cc(g);
  REQUIRE(g.vertex_count() == 6);
  REQUIRE(result.component_count() == 3);
  REQUIRE(result.connected(at<G>(0), at<G>(2)));
  REQUIRE(result.connected(at<G>(3), at<G>(7)));
  REQUIRE_FALSE(result.connected(at<G>(3), at<G>(4)));
  REQUIRE_FALSE(result.connected(at<G>(0), at<G>(7)));
}
