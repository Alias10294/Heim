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

TEST_CASE("DFS can run a fresh search after a supplied map throws", "[graph][dfs][exception]")
{
  using G = graph<>;
  using V = vertex<G>;
  G g;
  make_vertices(g, 3);
  add_edges(g, {{0, 1}, {1, 2}});

  std::unordered_map<V, V> predecessors;
  struct failing_map
  {
    std::unordered_map<V, V> *storage;

    V get(V key) const { return storage->at(key); }

    void set(V key, V value) const
    {
      if (key == V{2})
        throw user_failure{};
      storage->insert_or_assign(key, value);
    }
  };

  {
    hg::dfs_result<G, failing_map> discarded{
        std::in_place_type<G>, failing_map{&predecessors}};
    REQUIRE_THROWS_AS(hg::dfs(g, discarded, at<G>(0)), user_failure);
    // The interrupted result is discarded, not reused or inspected.
  }

  auto fresh = hg::dfs(g, at<G>(0));
  REQUIRE(fresh.reached(at<G>(2)));
  REQUIRE(fresh.finish_time_of(at<G>(0)) == 6);
  REQUIRE((path_ids<G>(runtime_path_from(fresh, at<G>(2))) == std::vector<unsigned>{2, 1, 0}));
}
