#include "model.hpp"
#include "visitors.hpp"

#include <sstream>

using namespace heim_test::graph_algorithm::dfs;

TEST_CASE("DFS search and target search handle deleted vertices and edges", "[graph][dfs][regression]")
{
  using G = graph<true, true>;
  G g;
  make_vertices(g, 9);
  add_edges(g, {{0, 1}, {1, 2}, {0, 3}, {3, 4}, {4, 6}, {6, 8}, {0, 7}, {7, 8}});
  g.remove_vertex(at<G>(1));
  g.remove_vertex(at<G>(5));
  g.remove_edges(at<G>(0), at<G>(3));
  add_edges(g, {{0, 4}});

  auto result = hg::dfs(g, at<G>(0));
  for (unsigned id : {0u, 4u, 6u, 7u, 8u})
  {
    auto v = at<G>(id);
    REQUIRE(result.reached(v));
    REQUIRE(result.discovery_time_of(v) < result.finish_time_of(v));
  }
  REQUIRE_FALSE(result.reached(at<G>(2)));
  REQUIRE_FALSE(result.reached(at<G>(3)));

  auto to_result = hg::dfs_to(g, at<G>(0), at<G>(8));
  REQUIRE(to_result.reached());
  auto path = path_ids<G>(runtime_target_path(to_result));
  REQUIRE(path.front() == 8);
  REQUIRE(path.back() == 0);
  REQUIRE(path.size() <= g.vertex_count());
}

TEST_CASE("DFS consumes genuine single-pass input roots", "[graph][dfs][regression]")
{
  external_graph g{5};
  g.add_edge(0, 1);
  g.add_edge(3, 4);
  std::istringstream source{"0 3"};
  auto roots = std::ranges::istream_view<int>(source);
  static_assert(std::ranges::input_range<decltype(roots)>);
  static_assert(!std::ranges::forward_range<decltype(roots)>);

  auto result = hg::dfs(g, roots);
  REQUIRE(result.reached(0));
  REQUIRE(result.reached(1));
  REQUIRE(result.reached(3));
  REQUIRE(result.reached(4));
  REQUIRE_FALSE(result.reached(2));
}

TEST_CASE("DFS_to accepts genuine single-pass roots", "[graph][dfs][regression]")
{
  external_graph g{5};
  g.add_edge(0, 1);
  g.add_edge(3, 4);
  std::istringstream source{"0 3"};
  auto roots = std::ranges::istream_view<int>(source);
  auto result = hg::dfs_to(g, roots, 4);
  REQUIRE(result.reached());
  std::vector<int> path;
  for (int v : runtime_target_path(result))
    path.push_back(v);
  REQUIRE((path == std::vector<int>{4, 3}));
}

TEST_CASE("DFS classifies a cross edge into an earlier finished tree", "[graph][dfs][regression]")
{
  external_graph g{3};
  g.add_edge(0, 1);
  g.add_edge(2, 1);
  std::array roots{0, 2};
  std::vector<std::string> events;
  trace_visitor visitor{&events};
  REQUIRE(hg::depth_first_visit(g, roots, visitor));
  auto const finish_first = std::ranges::find(events, std::string{"F1"});
  auto const start_second = std::ranges::find(events, std::string{"S2"});
  auto const cross_edge = std::ranges::find(events, std::string{"X1"});
  REQUIRE(finish_first != events.end());
  REQUIRE(start_second != events.end());
  REQUIRE(cross_edge != events.end());
  REQUIRE(finish_first < start_second);
  REQUIRE(start_second < cross_edge);
}
