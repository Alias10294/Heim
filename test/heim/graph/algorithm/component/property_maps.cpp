#include "../derived/support.hpp"

using namespace heim_test::graph_algorithm::derived;

TEST_CASE("Connected components accept an externally owned component map", "[graph][cc][maps]")
{
  using G = graph<false>;
  using V = vertex<G>;
  G g;
  make_vertices(g, 5);
  add_edges(g, {{0, 1}, {2, 3}});
  std::unordered_map<V, std::size_t> labels;
  auto result = hg::cc(g, external_map<V, std::size_t>{&labels});
  REQUIRE(result.component_count() == 3);
  REQUIRE(labels.size() == 5);
  REQUIRE(labels.at(at<G>(0)) == labels.at(at<G>(1)));
  REQUIRE(labels.at(at<G>(2)) == labels.at(at<G>(3)));
  REQUIRE(labels.at(at<G>(1)) != labels.at(at<G>(2)));
}

TEST_CASE("Connected components can fill a supplied result with an external map", "[graph][cc][maps]")
{
  using G = graph<false>;
  using V = vertex<G>;
  G g;
  make_vertices(g, 4);
  add_edges(g, {{0, 1}, {1, 2}});
  std::unordered_map<V, std::size_t> labels;
  using M = external_map<V, std::size_t>;
  hg::cc_result<G, M> result{std::in_place_type<G>, M{&labels}};
  hg::cc(g, result);
  REQUIRE(result.component_count() == 2);
  REQUIRE(labels.size() == 4);
  REQUIRE(result.connected(at<G>(0), at<G>(2)));
}

TEST_CASE("Connected components accept custom integral component identifiers", "[graph][cc][maps]")
{
  using G = graph<false>;
  using V = vertex<G>;
  G g;
  make_vertices(g, 5);
  add_edges(g, {{0, 1}, {3, 4}});
  std::unordered_map<V, unsigned short> ids;
  auto result = hg::cc(g, external_map<V, unsigned short>{&ids});
  STATIC_REQUIRE(std::same_as<decltype(result.component_count()), unsigned short>);
  REQUIRE(result.component_count() == 3);
  REQUIRE(ids.size() == 5);
}

TEST_CASE("Connected components initialize and overwrite external labels", "[graph][cc][maps]")
{
  using G = graph<false>;
  using V = vertex<G>;
  G g;
  make_vertices(g, 4);
  add_edges(g, {{0, 1}, {1, 2}});
  std::unordered_map<V, std::size_t> labels;

  for (unsigned id = 0; id < 4; ++id)
    labels.insert_or_assign(at<G>(id), 900u);

  auto result = hg::cc(g, external_map<V, std::size_t>{&labels});
  REQUIRE(result.component_count() == 2);
  REQUIRE(labels.size() == 4);
  REQUIRE(labels.at(at<G>(0)) != 900u);
  REQUIRE(labels.at(at<G>(3)) != 900u);
}

TEST_CASE("Connected components accept an external color map", "[graph][cc][maps]")
{
  using G = graph<false>;
  using V = vertex<G>;
  G g;
  make_vertices(g, 4);
  add_edges(g, {{0, 1}, {2, 3}});
  std::unordered_map<V, custom_color> colors;
  auto result = hg::cc(g, hg::default_property_map, external_map<V, custom_color>{&colors});
  REQUIRE(result.component_count() == 2);
  REQUIRE(colors.size() == 4);

  for (auto const &[v, clr] : colors)
  {
    (void)v;
    REQUIRE(clr == custom_color::black);
  }
}

TEST_CASE("Connected components preserve custom colors in provided result overload", "[graph][cc][maps]")
{
  using G = graph<false>;
  using V = vertex<G>;
  G g;
  make_vertices(g, 3);
  add_edges(g, {{0, 1}});
  std::unordered_map<V, custom_color> colors;
  hg::cc_result<G> result{std::in_place_type<G>};
  hg::cc(g, result, external_map<V, custom_color>{&colors});
  REQUIRE(result.component_count() == 2);
  REQUIRE(colors.at(at<G>(0)) == custom_color::black);
  REQUIRE(colors.at(at<G>(1)) == custom_color::black);
  REQUIRE(colors.at(at<G>(2)) == custom_color::black);
}
