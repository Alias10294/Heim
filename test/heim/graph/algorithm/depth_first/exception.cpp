#include "support.hpp"

#include <stdexcept>

using namespace heim_test::graph_algorithm::dfs;

namespace
{
struct user_failure : std::runtime_error
{
  user_failure() : std::runtime_error{"DFS visitor failure"} { }
};
}

TEST_CASE("DFS propagates exceptions thrown during vertex discovery", "[graph][dfs][exception]")
{
  external_graph g{2};
  g.add_edge(0, 1);
  struct visitor
  {
    void discover_vertex(external_graph const &, int v) const
    { if (v == 1) throw user_failure{}; }
  } vis;
  REQUIRE_THROWS_AS(hg::depth_first_visit(g, 0, vis), user_failure);
}

TEST_CASE("DFS propagates exceptions thrown by an edge visitor", "[graph][dfs][exception]")
{
  external_graph g{2};
  g.add_edge(0, 1);
  struct visitor
  {
    void examine_edge(external_graph const &, int) const
    { throw user_failure{}; }
  } vis;
  REQUIRE_THROWS_AS(hg::depth_first_visit(g, 0, vis), user_failure);
}

TEST_CASE("DFS propagates exceptions thrown by finish_edge", "[graph][dfs][exception]")
{
  external_graph g{2};
  g.add_edge(0, 1);
  struct visitor
  {
    void finish_edge(external_graph const &, int) const { throw user_failure{}; }
  } vis;
  REQUIRE_THROWS_AS(hg::depth_first_visit(g, 0, vis), user_failure);
}

TEST_CASE("DFS result maps remain usable in a separate successful search after an exception", "[graph][dfs][exception]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 3);
  add_edges(g, {{0, 1}, {1, 2}});
  auto result = hg::dfs(g, at<G>(0));
  REQUIRE(result.reached(at<G>(2)));
  REQUIRE(result.finish_time_of(at<G>(0)) == 6);
}
