#include "support.hpp"

using namespace heim_test::graph_algorithm::bfs;

namespace
{
struct user_failure : std::runtime_error
{
  user_failure() : std::runtime_error{"visitor failure"} { }
};
}

TEST_CASE("BFS visit propagates exceptions from discovery callbacks", "[graph][bfs][exception]")
{
  external_graph g{3};
  g.add_edge(0, 1);
  struct visitor
  {
    void discover_vertex(external_graph const &, int v) const
    {
      if (v == 1)
        throw user_failure{};
    }
  };

  REQUIRE_THROWS_AS(hg::breadth_first_visit(g, 0, visitor{}), user_failure);
}

TEST_CASE("BFS visit propagates exceptions from edge callbacks", "[graph][bfs][exception]")
{
  external_graph g{2};
  g.add_edge(0, 1);
  struct visitor
  {
    void examine_edge(external_graph const &, int) const { throw user_failure{}; }
  };

  REQUIRE_THROWS_AS(hg::breadth_first_visit(g, 0, visitor{}), user_failure);
}

TEST_CASE("BFS search propagates exceptions from caller-supplied predecessor maps", "[graph][bfs][exception]")
{
  external_graph g{2};
  g.add_edge(0, 1);
  struct failing_predecessors
  {
    int get(int) const { return 0; }
    void set(int, int) const { throw user_failure{}; }
  };

  REQUIRE_THROWS_AS(hg::bfs(g, 0, failing_predecessors{}), user_failure);
}
