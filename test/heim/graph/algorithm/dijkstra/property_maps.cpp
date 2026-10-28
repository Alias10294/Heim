#include "reference.hpp"

using namespace heim_test::graph_algorithm::dijkstra;

TEST_CASE("Dijkstra search accepts externally owned predecessor map", "[graph][dijkstra][maps]")
{
  using G = graph<>;
  using V = vertex<G>;
  weighted_graph<G> f{3};
  f.add(0, 1, 9);
  f.add(1, 2, 2);
  std::unordered_map<V, V> parents;
  using PM = external_map<V, V>;
  auto result = hg::dijkstra(f.g, at<G>(0), f.map(), PM{&parents});
  REQUIRE(parents.at(at<G>(0)) == at<G>(0));
  REQUIRE(parents.at(at<G>(1)) == at<G>(0));
  REQUIRE(parents.at(at<G>(2)) == at<G>(1));
  verify_search<G>(result, f.reference, {0});
}

TEST_CASE("Dijkstra search accepts externally owned distance map", "[graph][dijkstra][maps]")
{
  using G = graph<>;
  using V = vertex<G>;
  weighted_graph<G> f{3};
  f.add(0, 1, 4);
  f.add(1, 2, 3);
  std::unordered_map<V, unsigned> distances;
  using DM = external_map<V, unsigned>;
  auto result = hg::dijkstra(f.g, at<G>(0), f.map(), hg::default_property_map, DM{&distances});
  REQUIRE(distances.at(at<G>(2)) == 7);
  verify_search<G>(result, f.reference, {0});
}

TEST_CASE("Dijkstra search accepts both external result maps", "[graph][dijkstra][maps]")
{
  using G = graph<>;
  using V = vertex<G>;
  weighted_graph<G> f{4};
  f.add(0, 1, 9);
  f.add(0, 2, 1);
  f.add(2, 1, 2);
  std::unordered_map<V, V> parents;
  std::unordered_map<V, unsigned> distances;
  using PM = external_map<V, V>;
  using DM = external_map<V, unsigned>;
  auto result = hg::dijkstra(f.g, at<G>(0), f.map(), PM{&parents}, DM{&distances});
  REQUIRE(parents.at(at<G>(1)) == at<G>(2));
  REQUIRE(distances.at(at<G>(1)) == 3);
  verify_search<G>(result, f.reference, {0});
}

TEST_CASE("Dijkstra_to accepts external predecessor and distance maps", "[graph][dijkstra][maps]")
{
  using G = graph<>;
  using V = vertex<G>;
  weighted_graph<G> f{4};
  f.add(0, 1, 2);
  f.add(1, 2, 3);
  std::unordered_map<V, V> parents;
  std::unordered_map<V, unsigned> distances;
  using PM = external_map<V, V>;
  using DM = external_map<V, unsigned>;
  auto result = hg::dijkstra_to(f.g, at<G>(0), at<G>(2), f.map(), PM{&parents}, DM{&distances});
  REQUIRE(parents.at(at<G>(2)) == at<G>(1));
  REQUIRE(distances.at(at<G>(2)) == 5);
  verify_target<G>(result, f.reference, {0}, 2);
}

TEST_CASE("Dijkstra search works with externally owned color map", "[graph][dijkstra][maps]")
{
  using G = graph<>;
  using V = vertex<G>;
  weighted_graph<G> f{3};
  f.add(0, 1, 2);
  f.add(1, 2, 3);
  std::unordered_map<V, hg::color> colors;
  using CM = external_map<V, hg::color>;
  auto result = hg::dijkstra(f.g, at<G>(0), f.map(),
                             hg::default_property_map, hg::default_property_map,
                             std::numeric_limits<unsigned>::max(), 0u,
                             std::less<>{}, std::plus<>{}, CM{&colors});
  REQUIRE(colors.size() == 3);
  REQUIRE(colors.at(at<G>(0)) == hg::color::black);
  REQUIRE(colors.at(at<G>(2)) == hg::color::black);
  verify_search<G>(result, f.reference, {0});
}

TEST_CASE("Dijkstra search accepts user-defined color values", "[graph][dijkstra][maps]")
{
  using G = graph<>;
  using V = vertex<G>;
  weighted_graph<G> f{3};
  f.add(0, 1, 2);
  std::unordered_map<V, custom_color> colors;
  using CM = external_map<V, custom_color>;
  auto result = hg::dijkstra(f.g, at<G>(0), f.map(),
                             hg::default_property_map, hg::default_property_map,
                             std::numeric_limits<unsigned>::max(), 0u,
                             std::less<>{}, std::plus<>{}, CM{&colors});
  REQUIRE(colors.at(at<G>(0)) == custom_color::black);
  REQUIRE(colors.at(at<G>(1)) == custom_color::black);
  REQUIRE(colors.at(at<G>(2)) == custom_color::white);
  verify_search<G>(result, f.reference, {0});
}

TEST_CASE("Dijkstra accepts externally owned heap index map", "[graph][dijkstra][maps]")
{
  using G = graph<>;
  using V = vertex<G>;
  weighted_graph<G> f{4};
  f.add(0, 1, 10);
  f.add(0, 2, 1);
  f.add(2, 1, 1);
  f.add(1, 3, 1);
  std::unordered_map<V, std::size_t> indices;
  using IM = external_map<V, std::size_t>;
  auto result = hg::dijkstra(f.g, at<G>(0), f.map(),
                             hg::default_property_map, hg::default_property_map,
                             std::numeric_limits<unsigned>::max(), 0u,
                             std::less<>{}, std::plus<>{}, hg::default_property_map,
                             std::vector<V>{}, IM{&indices});
  verify_search<G>(result, f.reference, {0});
  REQUIRE(indices.contains(at<G>(1)));
  REQUIRE(indices.contains(at<G>(2)));
}

TEST_CASE("Dijkstra_visit supports all three externally owned maps", "[graph][dijkstra][maps]")
{
  using G = graph<>;
  using V = vertex<G>;
  weighted_graph<G> f{3};
  f.add(0, 1, 2);
  f.add(1, 2, 3);
  std::unordered_map<V, unsigned> distances;
  std::unordered_map<V, custom_color> colors;
  std::unordered_map<V, std::size_t> indices;
  using DM = external_map<V, unsigned>;
  using CM = external_map<V, custom_color>;
  using IM = external_map<V, std::size_t>;
  REQUIRE(runtime_visit(f.g, at<G>(0), f.map(),
                             []{}, DM{&distances},
                             std::numeric_limits<unsigned>::max(), 0u,
                             std::less<>{}, std::plus<>{}, CM{&colors},
                             std::vector<V>{}, IM{&indices}));
  REQUIRE(distances.at(at<G>(2)) == 5);
  REQUIRE(colors.at(at<G>(2)) == custom_color::black);
  REQUIRE(indices.contains(at<G>(2)));
}
