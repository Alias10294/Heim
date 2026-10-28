#include "reference.hpp"
#include "visitors.hpp"

using namespace heim_test::graph_algorithm::dijkstra;

TEST_CASE("Dijkstra visit accepts an empty graph and source range", "[graph][dijkstra][visit]")
{
  weighted_graph<graph<>> f{0};
  std::vector<vertex<graph<>>> roots;
  REQUIRE(runtime_visit(f.g, roots, f.map(), empty_visitor{}));
}

TEST_CASE("Dijkstra visit accepts an isolated source", "[graph][dijkstra][visit]")
{
  using G = graph<>;
  weighted_graph<G> f{2};
  REQUIRE(runtime_visit(f.g, at<G>(0), f.map(), empty_visitor{}));
}

TEST_CASE("Dijkstra visit accepts a single vertex and self-loop", "[graph][dijkstra][visit]")
{
  using G = graph<>;
  weighted_graph<G> f{1};
  f.add(0, 0, 0);
  REQUIRE(runtime_visit(f.g, at<G>(0), f.map(), empty_visitor{}));
}

TEST_CASE("Dijkstra visit accepts a complete directed graph", "[graph][dijkstra][visit]")
{
  using G = graph<true, true>;
  weighted_graph<G> f{5};
  for (unsigned i = 0; i < 5; ++i)
    for (unsigned j = 0; j < 5; ++j)
      f.add(i, j, i + j + 1);

  REQUIRE(runtime_visit(f.g, at<G>(0), f.map(), empty_visitor{}));
}

TEST_CASE("Dijkstra visit works with undirected adjacency", "[graph][dijkstra][visit]")
{
  using G = graph<false>;
  weighted_graph<G> f{4};
  f.add(0, 1, 2);
  f.add(1, 2, 3);
  f.add(2, 3, 4);
  REQUIRE(runtime_visit(f.g, at<G>(3), f.map(), empty_visitor{}));
}

TEST_CASE("Dijkstra visit processes a root range", "[graph][dijkstra][visit]")
{
  using G = graph<>;
  weighted_graph<G> f{5};
  f.add(0, 1, 3);
  f.add(3, 4, 2);
  std::array const roots{at<G>(0), at<G>(3)};
  REQUIRE(runtime_visit(f.g, roots, f.map(), empty_visitor{}));
}

TEST_CASE("Dijkstra visit accepts root duplicates", "[graph][dijkstra][visit]")
{
  using G = graph<>;
  weighted_graph<G> f{4};
  std::array const roots{at<G>(0), at<G>(0)};
  REQUIRE(runtime_visit(f.g, roots, f.map(), empty_visitor{}));
}

TEST_CASE("Dijkstra visit can update an externally owned distance map", "[graph][dijkstra][visit]")
{
  using G = graph<>;
  using V = vertex<G>;
  weighted_graph<G> f{4};
  f.add(0, 1, 6);
  f.add(0, 2, 1);
  f.add(2, 1, 2);
  std::unordered_map<V, unsigned> distances;
  using DM = external_map<V, unsigned>;
  REQUIRE(runtime_visit(f.g, at<G>(0), f.map(), empty_visitor{}, DM{&distances}));
  REQUIRE(distances.at(at<G>(0)) == 0);
  REQUIRE(distances.at(at<G>(1)) == 3);
  REQUIRE(distances.at(at<G>(2)) == 1);
  REQUIRE(distances.at(at<G>(3)) == std::numeric_limits<unsigned>::max());
}

TEST_CASE("Dijkstra visit uses a read-only edge weight map", "[graph][dijkstra][visit]")
{
  using G = graph<>;
  weighted_graph<G> f{3};
  f.add(0, 1, 2);
  f.add(1, 2, 3);
  auto const map = f.map();
  REQUIRE(runtime_visit(f.g, at<G>(0), map, empty_visitor{}));
}

TEST_CASE("Dijkstra visit can be invoked repeatedly", "[graph][dijkstra][visit]")
{
  using G = graph<>;
  weighted_graph<G> f{3};
  f.add(0, 1, 2);
  f.add(1, 2, 3);
  REQUIRE(runtime_visit(f.g, at<G>(0), f.map(), empty_visitor{}));
  REQUIRE(runtime_visit(f.g, at<G>(2), f.map(), empty_visitor{}));
}
