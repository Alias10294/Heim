#include "reference.hpp"

using namespace heim_test::graph_algorithm::dijkstra;

TEST_CASE("Dijkstra accepts explicit infinity and zero", "[graph][dijkstra][custom]")
{
  using G = graph<>;
  weighted_graph<G> f{3};
  f.add(0, 1, 7);
  auto result = hg::dijkstra(f.g, at<G>(0), f.map(),
                             hg::default_property_map, hg::default_property_map,
                             1000u, 0u);
  REQUIRE(runtime_distance_of(result, at<G>(0)) == 0);
  REQUIRE(runtime_distance_of(result, at<G>(1)) == 7);
  REQUIRE(runtime_distance_of(result, at<G>(2)) == 1000);
  REQUIRE_FALSE(result.reached(at<G>(2)));
}

TEST_CASE("Dijkstra honors a nonzero origin cost", "[graph][dijkstra][custom]")
{
  using G = graph<>;
  weighted_graph<G> f{3};
  f.add(0, 1, 7);
  f.add(1, 2, 4);
  auto result = hg::dijkstra(f.g, at<G>(0), f.map(),
                             hg::default_property_map, hg::default_property_map,
                             1000u, 10u);
  REQUIRE(runtime_distance_of(result, at<G>(0)) == 10);
  REQUIRE(runtime_distance_of(result, at<G>(1)) == 17);
  REQUIRE(runtime_distance_of(result, at<G>(2)) == 21);
}

TEST_CASE("Dijkstra accepts a custom combine operation for bottleneck minimization", "[graph][dijkstra][custom]")
{
  using G = graph<>;
  weighted_graph<G> f{4};
  f.add(0, 1, 8);
  f.add(0, 2, 3);
  f.add(2, 1, 4);
  f.add(1, 3, 2);
  auto combine = [](unsigned a, unsigned b){ return std::max(a, b); };
  auto result = hg::dijkstra(f.g, at<G>(0), f.map(),
                             hg::default_property_map, hg::default_property_map,
                             std::numeric_limits<unsigned>::max(), 0u,
                             std::less<>{}, combine);
  REQUIRE(runtime_distance_of(result, at<G>(1)) == 4);
  REQUIRE(runtime_distance_of(result, at<G>(3)) == 4);
}

TEST_CASE("Dijkstra accepts a custom greater comparator and min-combine", "[graph][dijkstra][custom]")
{
  using G = graph<>;
  weighted_graph<G> f{4};
  f.add(0, 1, 8);
  f.add(0, 2, 4);
  f.add(1, 3, 5);
  f.add(2, 3, 4);
  auto combine = [](unsigned a, unsigned b){ return std::min(a, b); };
  auto result = hg::dijkstra(f.g, at<G>(0), f.map(),
                             hg::default_property_map, hg::default_property_map,
                             0u, std::numeric_limits<unsigned>::max(),
                             std::greater<>{}, combine);
  REQUIRE(result.reached(at<G>(3)));
  REQUIRE(runtime_distance_of(result, at<G>(3)) == 5);
}

TEST_CASE("Dijkstra_to accepts comparator and combine for widest path", "[graph][dijkstra][custom]")
{
  using G = graph<>;
  weighted_graph<G> f{4};
  f.add(0, 1, 8);
  f.add(0, 2, 4);
  f.add(1, 3, 5);
  f.add(2, 3, 4);
  auto combine = [](unsigned a, unsigned b){ return std::min(a, b); };
  auto result = hg::dijkstra_to(f.g, at<G>(0), at<G>(3), f.map(),
                                hg::default_property_map, hg::default_property_map,
                                0u, std::numeric_limits<unsigned>::max(),
                                std::greater<>{}, combine);
  REQUIRE(result.reached());
  REQUIRE(runtime_target_distance(result) == 5);
}

TEST_CASE("Dijkstra accepts non-floating signed distances", "[graph][dijkstra][custom]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 3);
  auto a = g.place_edge(at<G>(0), at<G>(1)).first;
  auto b = g.place_edge(at<G>(1), at<G>(2)).first;
  std::vector<int> weights(std::max(edge_index(a), edge_index(b)) + 1);
  weights[edge_index(a)] = 3;
  weights[edge_index(b)] = 4;
  auto wm = weight_map<edge<G>, int>{&weights};
  auto result = hg::dijkstra(g, at<G>(0), wm,
                             hg::default_property_map, hg::default_property_map,
                             100, -10);
  REQUIRE(runtime_distance_of(result, at<G>(0)) == -10);
  REQUIRE(runtime_distance_of(result, at<G>(2)) == -3);
}

TEST_CASE("Dijkstra works with a custom strong distance type", "[graph][dijkstra][custom]")
{
  using G = graph<>;
  struct cost
  {
    unsigned value{};
    auto operator<=>(cost const &) const = default;
    cost operator+(cost rhs) const { return {value + rhs.value}; }
  };
  G g;
  make_vertices(g, 3);
  auto a = g.place_edge(at<G>(0), at<G>(1)).first;
  auto b = g.place_edge(at<G>(1), at<G>(2)).first;
  std::vector<cost> weights(std::max(edge_index(a), edge_index(b)) + 1);
  weights[edge_index(a)] = {2};
  weights[edge_index(b)] = {3};
  auto wm = weight_map<edge<G>, cost>{&weights};
  auto result = hg::dijkstra(g, at<G>(0), wm,
                             hg::default_property_map, hg::default_property_map,
                             cost{1000}, cost{0});
  REQUIRE(runtime_distance_of(result, at<G>(2)).value == 5);
}

TEST_CASE("Dijkstra invokes supplied comparison object", "[graph][dijkstra][custom]")
{
  using G = graph<>;
  weighted_graph<G> f{3};
  f.add(0, 1, 2);
  f.add(1, 2, 4);
  unsigned comparisons{};
  auto cmp = [&comparisons](unsigned a, unsigned b)
  {
    ++comparisons;
    return a < b;
  };
  auto result = hg::dijkstra(f.g, at<G>(0), f.map(),
                             hg::default_property_map, hg::default_property_map,
                             std::numeric_limits<unsigned>::max(), 0u, cmp);
  REQUIRE(runtime_distance_of(result, at<G>(2)) == 6);
  REQUIRE(comparisons > 0);
}

TEST_CASE("Dijkstra invokes supplied combination object", "[graph][dijkstra][custom]")
{
  using G = graph<>;
  weighted_graph<G> f{3};
  f.add(0, 1, 2);
  f.add(1, 2, 4);
  unsigned combinations{};
  auto combine = [&combinations](unsigned a, unsigned b)
  {
    ++combinations;
    return a + b;
  };
  auto result = hg::dijkstra(f.g, at<G>(0), f.map(),
                             hg::default_property_map, hg::default_property_map,
                             std::numeric_limits<unsigned>::max(), 0u,
                             std::less<>{}, combine);
  REQUIRE(runtime_distance_of(result, at<G>(2)) == 6);
  REQUIRE(combinations == 2);
}

TEST_CASE("Dijkstra supports deque as heap container", "[graph][dijkstra][custom]")
{
  using G = graph<>;
  using V = vertex<G>;
  weighted_graph<G> f{4};
  f.add(0, 1, 10);
  f.add(0, 2, 1);
  f.add(2, 1, 1);
  f.add(1, 3, 2);
  std::deque<V> heap;
  auto result = hg::dijkstra(f.g, at<G>(0), f.map(),
                             hg::default_property_map, hg::default_property_map,
                             std::numeric_limits<unsigned>::max(), 0u,
                             std::less<>{}, std::plus<>{}, hg::default_property_map, heap);
  REQUIRE(heap.empty());
  verify_search<G>(result, f.reference, {0});
}

TEST_CASE("Dijkstra supports externally supplied vector heap container", "[graph][dijkstra][custom]")
{
  using G = graph<>;
  using V = vertex<G>;
  weighted_graph<G> f{4};
  f.add(0, 1, 10);
  f.add(0, 2, 1);
  f.add(2, 1, 1);
  f.add(1, 3, 2);
  std::vector<V> heap;
  auto result = hg::dijkstra(f.g, at<G>(0), f.map(),
                             hg::default_property_map, hg::default_property_map,
                             std::numeric_limits<unsigned>::max(), 0u,
                             std::less<>{}, std::plus<>{}, hg::default_property_map, heap);
  REQUIRE(heap.empty());
  verify_search<G>(result, f.reference, {0});
}

TEST_CASE("Dijkstra_to supports deque as heap container", "[graph][dijkstra][custom]")
{
  using G = graph<>;
  using V = vertex<G>;
  weighted_graph<G> f{4};
  f.add(0, 1, 10);
  f.add(0, 2, 1);
  f.add(2, 1, 1);
  f.add(1, 3, 2);
  std::deque<V> heap;
  auto result = hg::dijkstra_to(f.g, at<G>(0), at<G>(3), f.map(),
                                hg::default_property_map, hg::default_property_map,
                                std::numeric_limits<unsigned>::max(), 0u,
                                std::less<>{}, std::plus<>{}, hg::default_property_map, heap);
  REQUIRE(result.reached());
  REQUIRE(runtime_target_distance(result) == 4);
}
