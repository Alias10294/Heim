#include "model.hpp"
#include "visitors.hpp"

using namespace heim_test::graph_algorithm::dfs;

TEST_CASE("DFS visits an external graph using only its CPO-compatible operations", "[graph][dfs][generic]")
{
  auto g = make_external_graph();
  std::vector<int> discovered;
  struct visitor
  {
    std::vector<int> *values;
    void discover_vertex(external_graph const &, int v) const { values->push_back(v); }
  } vis{&discovered};
  REQUIRE(hg::depth_first_visit(g, 0, vis));
  REQUIRE((discovered == std::vector<int>{0, 1, 2, 3}));
}

TEST_CASE("DFS search on an external graph yields correct predecessor chains", "[graph][dfs][generic]")
{
  auto g = make_external_graph();
  auto result = hg::dfs(g, 0);
  REQUIRE(result.reached(3));
  REQUIRE(runtime_predecessor_of(result, 3) == 2);
  std::vector<int> actual;
  for (int v : runtime_path_from(result, 3))
    actual.push_back(v);
  REQUIRE((actual == std::vector<int>{3, 2, 1, 0}));
}

TEST_CASE("DFS_to on external graph stops at the first discovery of its target", "[graph][dfs][generic]")
{
  auto g = make_external_graph();
  auto result = hg::dfs_to(g, 0, 2);
  REQUIRE(result.reached());
  REQUIRE(result.discovery_time() == 3);
  std::vector<int> actual;
  for (int v : runtime_target_path(result))
    actual.push_back(v);
  REQUIRE((actual == std::vector<int>{2, 1, 0}));
}

TEST_CASE("DFS results support an external graph with isolated vertices", "[graph][dfs][generic]")
{
  external_graph g{5};
  g.add_edge(0, 1);
  g.add_edge(3, 4);
  std::array const roots{0, 3};
  auto result = hg::dfs(g, roots);
  REQUIRE(result.reached(0));
  REQUIRE(result.reached(1));
  REQUIRE_FALSE(result.reached(2));
  REQUIRE(result.reached(4));
}

TEST_CASE("DFS reference result on external graph has nested discovery intervals", "[graph][dfs][generic]")
{
  auto g = make_external_graph();
  auto result = hg::dfs(g, 0);
  for (int id = 1; id <= 3; ++id)
  {
    int parent = runtime_predecessor_of(result, id);
    REQUIRE(result.discovery_time_of(parent) < result.discovery_time_of(id));
    REQUIRE(result.finish_time_of(id) < result.finish_time_of(parent));
  }
}
