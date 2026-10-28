#include "model.hpp"

using namespace heim_test::graph_algorithm::bfs;

namespace
{
template<typename G>
void run_model(std::uint32_t seed, unsigned size, unsigned edge_count)
{
  G g;
  make_vertices(g, size);
  reference_graph model{size, G::is_directed};
  random_generator random{seed};

  for (unsigned i = 0; i < edge_count; ++i)
  {
    unsigned const u = static_cast<unsigned>(random.bounded(size));
    unsigned const v = static_cast<unsigned>(random.bounded(size));
    (void)g.place_edge(at<G>(u), at<G>(v));
    model.add(u, v);
  }

  for (unsigned iteration = 0; iteration < 12; ++iteration)
  {
    INFO("seed=" << seed << " iteration=" << iteration
         << " directed=" << G::is_directed << " multi=" << G::is_multigraph);

    unsigned const root = static_cast<unsigned>(random.bounded(size));
    auto result = hg::bfs(g, at<G>(root));
    verify_search(g, result, model, {root});
  }

  std::vector<unsigned> roots;
  std::vector<vertex<G>> root_vertices;
  for (unsigned i = 0; i < 5; ++i)
  {
    unsigned const root = static_cast<unsigned>(random.bounded(size));
    roots.push_back(root);
    root_vertices.push_back(at<G>(root));
  }

  auto multi_result = hg::bfs(g, root_vertices);
  verify_search(g, multi_result, model, roots);
}
} // namespace

TEST_CASE("BFS reference model: directed simple full incidence", "[graph][bfs][model]")
{
  run_model<graph<true, false>>(0xC0FFEE11u, 48, 215);
  run_model<graph<true, false>>(0xA57F2341u, 48, 215);
}

TEST_CASE("BFS reference model: directed multigraph full incidence", "[graph][bfs][model]")
{
  run_model<graph<true, true>>(0xC0FFEE11u, 48, 250);
  run_model<graph<true, true>>(0xA57F2341u, 48, 250);
}

TEST_CASE("BFS reference model: undirected simple graph", "[graph][bfs][model]")
{
  run_model<graph<false, false>>(0xC0FFEE11u, 48, 215);
  run_model<graph<false, false>>(0xA57F2341u, 48, 215);
}

TEST_CASE("BFS reference model: undirected multigraph", "[graph][bfs][model]")
{
  run_model<graph<false, true>>(0xC0FFEE11u, 48, 250);
  run_model<graph<false, true>>(0xA57F2341u, 48, 250);
}

TEST_CASE("BFS reference model: directed outgoing-only simple graph", "[graph][bfs][model]")
{
  run_model<graph<true, false, false, true>>(0xA57F2341u, 48, 215);
}

TEST_CASE("BFS reference model: directed outgoing-only multigraph", "[graph][bfs][model]")
{
  run_model<graph<true, true, false, true>>(0xA57F2341u, 48, 250);
}

TEST_CASE("BFS reference model: sparse pages", "[graph][bfs][model]")
{
  run_model<graph<true, true, true, true, 2>>(0x76543210u, 36, 140);
}

TEST_CASE("BFS directed and undirected reachability differ on reverse traversal", "[graph][bfs][model]")
{
  graph<true> directed;
  graph<false> undirected;
  make_vertices(directed, 3);
  make_vertices(undirected, 3);
  add_edges(directed, {{0, 1}, {1, 2}});
  add_edges(undirected, {{0, 1}, {1, 2}});

  auto d = hg::bfs(directed, at<graph<true>>(2));
  auto u = hg::bfs(undirected, at<graph<false>>(2));
  REQUIRE_FALSE(d.reached(at<graph<true>>(0)));
  REQUIRE(u.reached(at<graph<false>>(0)));
  REQUIRE(u.level_of(at<graph<false>>(0)) == 2);
}
