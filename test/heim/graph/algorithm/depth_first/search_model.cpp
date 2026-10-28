#include "model.hpp"

using namespace heim_test::graph_algorithm::dfs;

namespace
{
template<typename G>
void run_random_model(std::uint32_t seed, unsigned rounds)
{
  random_generator random{seed};
  for (unsigned round = 0; round < rounds; ++round)
  {
    unsigned const count = 1 + static_cast<unsigned>(random.bounded(18));
    unsigned const edge_count = static_cast<unsigned>(random.bounded(3 * count + 1));
    G g;
    make_vertices(g, count);
    reference_graph reference{count, G::is_directed};

    for (unsigned i = 0; i < edge_count; ++i)
    {
      unsigned const u = static_cast<unsigned>(random.bounded(count));
      unsigned const v = static_cast<unsigned>(random.bounded(count));
      auto [edge, inserted] = g.place_edge(at<G>(u), at<G>(v));
      (void)edge;
      if (inserted)
        reference.add(u, v);
    }

    unsigned const root_a = static_cast<unsigned>(random.bounded(count));
    unsigned const root_b = static_cast<unsigned>(random.bounded(count));
    std::vector<unsigned> const roots{root_a, root_b};
    auto const descriptors = std::array{at<G>(root_a), at<G>(root_b)};

    INFO("seed=" << seed << " round=" << round << " vertices=" << count
         << " edges=" << edge_count << " roots=" << root_a << "," << root_b);
    auto result = hg::dfs(g, descriptors);
    verify_search(g, result, reference, roots);

    unsigned const target = static_cast<unsigned>(random.bounded(count));
    auto to_result = hg::dfs_to(g, descriptors, at<G>(target));
    verify_target(to_result, reference, roots, target);
  }
}
}

TEST_CASE("DFS reference model directed simple graph, first seed", "[graph][dfs][model]")
{ run_random_model<graph<true>>(0xDEADBEEFu, 38); }

TEST_CASE("DFS reference model directed simple graph, second seed", "[graph][dfs][model]")
{ run_random_model<graph<true>>(0x12345678u, 38); }

TEST_CASE("DFS reference model directed multigraph with loops", "[graph][dfs][model]")
{ run_random_model<graph<true, true>>(0xF00DBABEu, 38); }

TEST_CASE("DFS reference model directed outgoing-only graph", "[graph][dfs][model]")
{ run_random_model<graph<true, false, false, true>>(0xAA55AA55u, 38); }

TEST_CASE("DFS reference model undirected simple graph", "[graph][dfs][model]")
{ run_random_model<graph<false>>(0x13579BDFu, 38); }

TEST_CASE("DFS reference model undirected multigraph", "[graph][dfs][model]")
{ run_random_model<graph<false, true>>(0x2468ACE0u, 38); }

TEST_CASE("DFS reference model all vertices isolated", "[graph][dfs][model]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 20);
  reference_graph reference{20, true};
  std::vector<unsigned> roots{0, 3, 10, 19};
  check_model(g, reference, roots);
}

TEST_CASE("DFS reference model directed dense graph", "[graph][dfs][model]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 9);
  reference_graph reference{9, true};
  for (unsigned u = 0; u < 9; ++u)
    for (unsigned v = 0; v < 9; ++v)
    {
      if ((u + v) % 5 == 0)
        continue;
      if (g.place_edge(at<G>(u), at<G>(v)).second)
        reference.add(u, v);
    }
  check_model(g, reference, {0});
}
