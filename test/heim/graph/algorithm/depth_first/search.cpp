#include "model.hpp"

using namespace heim_test::graph_algorithm::dfs;

TEST_CASE("DFS search assigns discovery and finish times on a chain", "[graph][dfs][search]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 5);
  add_edges(g, {{0, 1}, {1, 2}, {2, 3}, {3, 4}});
  auto result = hg::dfs(g, at<G>(0));
  for (unsigned i = 0; i < 5; ++i)
  {
    auto v = at<G>(i);
    REQUIRE(result.reached(v));
    REQUIRE(result.discovery_time_of(v) == i + 1);
    REQUIRE(result.finish_time_of(v) == 10 - i);
  }
  REQUIRE((path_ids<G>(runtime_path_from(result, at<G>(4))) ==
           std::vector<unsigned>{4, 3, 2, 1, 0}));
}

TEST_CASE("DFS search leaves unreachable vertices with invalid timestamps", "[graph][dfs][search]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 5);
  add_edges(g, {{0, 1}, {2, 3}});
  auto result = hg::dfs(g, at<G>(0));
  for (unsigned id : {2u, 3u, 4u})
  {
    REQUIRE_FALSE(result.reached(at<G>(id)));
    REQUIRE(result.discovery_time_of(at<G>(id)) == std::numeric_limits<std::size_t>::max());
    REQUIRE(result.finish_time_of(at<G>(id)) == std::numeric_limits<std::size_t>::max());
  }
}

TEST_CASE("DFS search accepts a single isolated vertex", "[graph][dfs][search]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 3);
  auto result = hg::dfs(g, at<G>(1));
  REQUIRE(result.discovery_time_of(at<G>(1)) == 1);
  REQUIRE(result.finish_time_of(at<G>(1)) == 2);
  REQUIRE(runtime_predecessor_of(result, at<G>(1)) == at<G>(1));
}

TEST_CASE("DFS search accepts an empty root range", "[graph][dfs][search]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 3);
  std::vector<vertex<G>> roots;
  auto result = hg::dfs(g, roots);
  for (unsigned id = 0; id < 3; ++id)
    REQUIRE_FALSE(result.reached(at<G>(id)));
}

TEST_CASE("DFS search accepts multiple disconnected roots", "[graph][dfs][search]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 6);
  add_edges(g, {{0, 1}, {1, 2}, {3, 4}});
  auto const roots = std::array{at<G>(0), at<G>(3)};
  auto result = hg::dfs(g, roots);
  for (unsigned id = 0; id < 5; ++id)
    REQUIRE(result.reached(at<G>(id)));
  REQUIRE_FALSE(result.reached(at<G>(5)));
  REQUIRE((path_ids<G>(runtime_path_from(result, at<G>(4))) == std::vector<unsigned>{4, 3}));
}

TEST_CASE("DFS search ignores repeated and previously discovered roots", "[graph][dfs][search]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 4);
  add_edges(g, {{0, 1}, {1, 2}});
  std::array const roots{at<G>(0), at<G>(1), at<G>(0), at<G>(3)};
  auto result = hg::dfs(g, roots);
  REQUIRE(result.discovery_time_of(at<G>(0)) == 1);
  REQUIRE(result.finish_time_of(at<G>(3)) == 8);
  REQUIRE(runtime_predecessor_of(result, at<G>(1)) == at<G>(0));
  REQUIRE(runtime_predecessor_of(result, at<G>(3)) == at<G>(3));
}

TEST_CASE("DFS search works on directed self-loops and parallel edges", "[graph][dfs][search]")
{
  using G = graph<true, true>;
  G g;
  make_vertices(g, 4);
  add_edges(g, {{0, 0}, {0, 1}, {0, 1}, {1, 2}, {2, 0}, {2, 3}});
  reference_graph model{4, true};
  for (auto [u, v] : std::initializer_list<std::pair<unsigned, unsigned>>{
           {0, 0}, {0, 1}, {0, 1}, {1, 2}, {2, 0}, {2, 3}})
    model.add(u, v);
  auto result = hg::dfs(g, at<G>(0));
  verify_search(g, result, model, {0});
}

TEST_CASE("DFS search works on undirected graphs with cycles", "[graph][dfs][search]")
{
  using G = graph<false>;
  G g;
  make_vertices(g, 5);
  add_edges(g, {{0, 1}, {1, 2}, {2, 0}, {3, 4}});
  reference_graph model{5, false};
  for (auto [u, v] : std::initializer_list<std::pair<unsigned, unsigned>>{
           {0, 1}, {1, 2}, {2, 0}, {3, 4}})
    model.add(u, v);
  auto result = hg::dfs(g, at<G>(0));
  verify_search(g, result, model, {0});
}

TEST_CASE("DFS search result aliases expose the same type", "[graph][dfs][search]")
{
  using G = graph<>;
  static_assert(std::same_as<hg::dfs_result<G>, hg::depth_first_search_result<G>>);
  G g;
  make_vertices(g, 2);
  add_edges(g, {{0, 1}});
  auto result = hg::depth_first_search(g, at<G>(0));
  REQUIRE(result.reached(at<G>(1)));
}
