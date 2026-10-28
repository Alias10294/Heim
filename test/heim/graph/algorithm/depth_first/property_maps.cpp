#include "support.hpp"

using namespace heim_test::graph_algorithm::dfs;

TEST_CASE("DFS visit initializes every external color-map entry", "[graph][dfs][maps]")
{
  using G = graph<>;
  using V = vertex<G>;
  G g;
  make_vertices(g, 4);
  add_edges(g, {{0, 1}, {1, 2}});

  std::unordered_map<V, hg::color> colors;
  auto cmap = external_map<V, hg::color>{&colors};
  REQUIRE(hg::depth_first_visit(g, at<G>(0), std::monostate{}, cmap));
  REQUIRE(colors.size() == 4);
  for (unsigned id = 0; id < 4; ++id)
    REQUIRE(colors.at(at<G>(id)) == (id == 3 ? hg::color::white : hg::color::black));
}

TEST_CASE("DFS visit supports a custom enum with white black gray enumerators", "[graph][dfs][maps]")
{
  using G = graph<>;
  using V = vertex<G>;
  G g;
  make_vertices(g, 3);
  add_edges(g, {{0, 1}});
  std::unordered_map<V, custom_color> colors;
  REQUIRE(hg::depth_first_visit(g, at<G>(0), std::monostate{},
                                external_map<V, custom_color>{&colors}));
  REQUIRE(colors.at(at<G>(0)) == custom_color::black);
  REQUIRE(colors.at(at<G>(1)) == custom_color::black);
  REQUIRE(colors.at(at<G>(2)) == custom_color::white);
}

TEST_CASE("DFS writes predecessor discovery and finish maps supplied by the caller", "[graph][dfs][maps]")
{
  using G = graph<>;
  using V = vertex<G>;
  G g;
  make_vertices(g, 4);
  add_edges(g, {{0, 1}, {1, 2}});
  std::unordered_map<V, V> parents;
  std::unordered_map<V, std::size_t> discovers;
  std::unordered_map<V, std::size_t> finishes;
  using PM = external_map<V, V>;
  using TM = external_map<V, std::size_t>;

  auto result = hg::dfs(g, at<G>(0), PM{&parents}, TM{&discovers}, TM{&finishes});
  REQUIRE(result.reached(at<G>(2)));
  REQUIRE(parents.at(at<G>(2)) == at<G>(1));
  REQUIRE(discovers.at(at<G>(0)) == 1);
  REQUIRE(finishes.at(at<G>(0)) == 6);
  REQUIRE(discovers.at(at<G>(3)) == std::numeric_limits<std::size_t>::max());
  REQUIRE(finishes.at(at<G>(3)) == std::numeric_limits<std::size_t>::max());
}

TEST_CASE("DFS result accepts a separate external color map", "[graph][dfs][maps]")
{
  using G = graph<>;
  using V = vertex<G>;
  G g;
  make_vertices(g, 3);
  add_edges(g, {{0, 1}, {1, 2}});
  std::unordered_map<V, hg::color> colors;
  auto result = hg::dfs(g, at<G>(0), hg::default_property_map,
                        hg::default_property_map, hg::default_property_map,
                        external_map<V, hg::color>{&colors});
  REQUIRE(result.reached(at<G>(2)));
  REQUIRE(colors.size() == 3);
  REQUIRE(colors.at(at<G>(2)) == hg::color::black);
}

TEST_CASE("DFS_to returns a result with external predecessor and discovery maps", "[graph][dfs][maps]")
{
  using G = graph<>;
  using V = vertex<G>;
  G g;
  make_vertices(g, 4);
  add_edges(g, {{0, 1}, {1, 2}, {2, 3}});
  std::unordered_map<V, V> parents;
  std::unordered_map<V, std::size_t> discovers;
  using PM = external_map<V, V>;
  using DM = external_map<V, std::size_t>;
  auto result = hg::dfs_to(g, at<G>(0), at<G>(2), PM{&parents}, DM{&discovers});
  REQUIRE(result.reached());
  REQUIRE(parents.at(at<G>(2)) == at<G>(1));
  REQUIRE(discovers.at(at<G>(2)) == 3);
  REQUIRE(discovers.at(at<G>(3)) == std::numeric_limits<std::size_t>::max());
}

TEST_CASE("DFS search accepts custom maps in the output-result overload", "[graph][dfs][maps]")
{
  using G = graph<>;
  using V = vertex<G>;
  G g;
  make_vertices(g, 3);
  add_edges(g, {{0, 1}, {1, 2}});
  std::unordered_map<V, V> parents;
  std::unordered_map<V, std::size_t> discovers;
  std::unordered_map<V, std::size_t> finishes;
  using PM = external_map<V, V>;
  using TM = external_map<V, std::size_t>;
  hg::dfs_result<G, PM, TM, TM> result{
      std::in_place_type<G>, PM{&parents}, TM{&discovers}, TM{&finishes}};
  hg::depth_first_search(g, result, at<G>(0));
  REQUIRE(result.reached(at<G>(2)));
  REQUIRE(discovers.at(at<G>(2)) == 3);
  REQUIRE(finishes.at(at<G>(0)) == 6);
}

TEST_CASE("DFS_to output-result overload accepts external maps", "[graph][dfs][maps]")
{
  using G = graph<>;
  using V = vertex<G>;
  G g;
  make_vertices(g, 3);
  add_edges(g, {{0, 1}, {1, 2}});
  std::unordered_map<V, V> parents;
  std::unordered_map<V, std::size_t> times;
  using PM = external_map<V, V>;
  using TM = external_map<V, std::size_t>;
  hg::dfs_to_result<G, PM, TM> result{
      std::in_place_type<G>, at<G>(2), PM{&parents}, TM{&times}};
  hg::dfs_to(g, result, at<G>(0));
  REQUIRE(result.reached());
  REQUIRE(times.at(at<G>(2)) == 3);
}
