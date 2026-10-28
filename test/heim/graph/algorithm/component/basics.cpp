#include "../derived/reference.hpp"

using namespace heim_test::graph_algorithm::derived;

TEST_CASE("Connected components of an empty graph", "[graph][cc][basic]")
{
  graph<false> g;
  auto result = hg::cc(g);
  REQUIRE(result.component_count() == 0);
  REQUIRE(result.connected());
}

TEST_CASE("Connected components of a singleton graph", "[graph][cc][basic]")
{
  using G = graph<false>;
  G g;
  make_vertices(g, 1);
  auto result = hg::connected_components(g);
  REQUIRE(result.component_count() == 1);
  REQUIRE(result.connected());
  REQUIRE(result.connected(at<G>(0), at<G>(0)));
  REQUIRE(result.component_id_of(at<G>(0)) == 0);
}

TEST_CASE("Connected components of entirely isolated vertices", "[graph][cc][basic]")
{
  using G = graph<false>;
  G g;
  make_vertices(g, 8);
  auto result = hg::cc(g);
  REQUIRE(result.component_count() == 8);
  REQUIRE_FALSE(result.connected());

  for (unsigned u = 0; u < 8; ++u)
  {
    REQUIRE(result.connected(at<G>(u), at<G>(u)));

    for (unsigned v = 0; v < 8; ++v)
      REQUIRE(result.connected(at<G>(u), at<G>(v)) == (u == v));
  }
}

TEST_CASE("Connected components of a connected chain", "[graph][cc][basic]")
{
  using G = graph<false>;
  G g;
  make_vertices(g, 6);
  add_edges(g, {{0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 5}});
  auto result = hg::cc(g);
  REQUIRE(result.component_count() == 1);
  REQUIRE(result.connected());
  REQUIRE(result.connected(at<G>(0), at<G>(5)));
}

TEST_CASE("Connected components partition disconnected chains and isolates", "[graph][cc][basic]")
{
  using G = graph<false>;
  G g;
  make_vertices(g, 8);
  add_edges(g, {{0, 1}, {1, 2}, {3, 4}, {4, 5}, {5, 3}});
  reference_graph reference{8};
  for (auto [u, v] : std::array<std::pair<unsigned, unsigned>, 5>{
           {{0, 1}, {1, 2}, {3, 4}, {4, 5}, {5, 3}}})
    reference.add(u, v);

  verify_components(g, hg::cc(g), reference);
}

TEST_CASE("Connected components count self loops only once", "[graph][cc][basic]")
{
  using G = graph<false, true>;
  G g;
  make_vertices(g, 4);
  add_edges(g, {{0, 0}, {0, 1}, {1, 1}, {2, 2}, {3, 3}});
  auto result = hg::cc(g);
  REQUIRE(result.component_count() == 3);
  REQUIRE(result.connected(at<G>(0), at<G>(1)));
  REQUIRE_FALSE(result.connected(at<G>(1), at<G>(2)));
}

TEST_CASE("Connected components ignore the multiplicity of parallel edges", "[graph][cc][basic]")
{
  using G = graph<false, true>;
  G g;
  make_vertices(g, 4);
  add_edges(g, {{0, 1}, {0, 1}, {1, 0}, {1, 2}, {1, 2}});
  auto result = hg::cc(g);
  REQUIRE(result.component_count() == 2);
  REQUIRE(result.connected(at<G>(0), at<G>(2)));
  REQUIRE_FALSE(result.connected(at<G>(0), at<G>(3)));
}

TEST_CASE("Connected components use undirected edges regardless of orientation", "[graph][cc][basic]")
{
  using G = graph<false>;
  G g;
  make_vertices(g, 5);
  add_edges(g, {{2, 1}, {1, 0}, {4, 3}});
  auto result = hg::cc(g);
  REQUIRE(result.component_count() == 2);
  REQUIRE(result.connected(at<G>(0), at<G>(2)));
  REQUIRE(result.connected(at<G>(3), at<G>(4)));
}

TEST_CASE("Connected components remain correct for a cycle with a chord", "[graph][cc][basic]")
{
  using G = graph<false>;
  G g;
  make_vertices(g, 6);
  add_edges(g, {{0, 1}, {1, 2}, {2, 3}, {3, 0}, {0, 2}, {4, 5}});
  auto result = hg::cc(g);
  REQUIRE(result.component_count() == 2);
  REQUIRE(result.connected(at<G>(0), at<G>(3)));
  REQUIRE_FALSE(result.connected(at<G>(3), at<G>(4)));
}

TEST_CASE("Connected components do not mutate the input graph", "[graph][cc][basic]")
{
  using G = graph<false>;
  G g;
  make_vertices(g, 5);
  add_edges(g, {{0, 1}, {1, 2}, {3, 4}});
  auto const vertices_before = g.vertex_count();
  auto const edges_before = g.edge_count();
  auto result = hg::cc(g);
  REQUIRE(result.component_count() == 2);
  REQUIRE(g.vertex_count() == vertices_before);
  REQUIRE(g.edge_count() == edges_before);
}
