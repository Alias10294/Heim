#include "reference.hpp"

using namespace heim_test::graph_algorithm::dijkstra;

TEST_CASE("Dijkstra propagates exceptions from weight map", "[graph][dijkstra][exception]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 2);
  (void)g.place_edge(at<G>(0), at<G>(1));
  struct throwing_map
  {
    unsigned get(edge<G>) const
    { throw std::runtime_error{"weight lookup failed"}; }
  };
  REQUIRE_THROWS_AS(hg::dijkstra(g, at<G>(0), throwing_map{}), std::runtime_error);
}

TEST_CASE("Dijkstra_visit propagates visitor exceptions", "[graph][dijkstra][exception]")
{
  using G = graph<>;
  weighted_graph<G> f{2};
  f.add(0, 1, 1);
  struct visitor
  {
    void examine_vertex(G const &, vertex<G>) const
    { throw std::runtime_error{"visitor failed"}; }
  };
  REQUIRE_THROWS_AS(runtime_visit(f.g, at<G>(0), f.map(), visitor{}), std::runtime_error);
}

TEST_CASE("Dijkstra propagates exceptions from combine operation", "[graph][dijkstra][exception]")
{
  using G = graph<>;
  weighted_graph<G> f{2};
  f.add(0, 1, 1);
  auto combine = [](unsigned, unsigned) -> unsigned
  { throw std::runtime_error{"combine failed"}; };
  REQUIRE_THROWS_AS((hg::dijkstra(f.g, at<G>(0), f.map(),
                                  hg::default_property_map, hg::default_property_map,
                                  std::numeric_limits<unsigned>::max(), 0u,
                                  std::less<>{}, combine)), std::runtime_error);
}

TEST_CASE("Dijkstra propagates exceptions from comparator", "[graph][dijkstra][exception]")
{
  using G = graph<>;
  weighted_graph<G> f{2};
  f.add(0, 1, 1);
  auto compare = [](unsigned, unsigned) -> bool
  { throw std::runtime_error{"comparison failed"}; };
  REQUIRE_THROWS_AS((hg::dijkstra(f.g, at<G>(0), f.map(),
                                  hg::default_property_map, hg::default_property_map,
                                  std::numeric_limits<unsigned>::max(), 0u,
                                  compare)), std::runtime_error);
}

TEST_CASE("Dijkstra_to propagates exceptions from weight map", "[graph][dijkstra][exception]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 2);
  (void)g.place_edge(at<G>(0), at<G>(1));
  struct throwing_map
  {
    unsigned get(edge<G>) const
    { throw std::runtime_error{"weight lookup failed"}; }
  };
  REQUIRE_THROWS_AS(hg::dijkstra_to(g, at<G>(0), at<G>(1), throwing_map{}), std::runtime_error);
}
