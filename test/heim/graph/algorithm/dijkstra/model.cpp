#include "reference.hpp"

using namespace heim_test::graph_algorithm::dijkstra;

namespace
{
template<typename G>
void run_random_model(std::uint32_t seed, unsigned rounds)
{
  random_generator random{seed};

  for (unsigned round = 0; round < rounds; ++round)
  {
    unsigned const count = 1 + random.bounded(22);
    unsigned const edge_count = random.bounded(4 * count + 1);
    weighted_graph<G> f{count};

    for (unsigned i = 0; i < edge_count; ++i)
    {
      unsigned const u = random.bounded(count);
      unsigned const v = random.bounded(count);
      unsigned const weight = random.bounded(41);
      f.add(u, v, weight);
    }

    auto const a = random.bounded(count);
    auto const b = random.bounded(count);
    std::array const roots{at<G>(a), at<G>(b)};
    INFO("seed=" << seed << " round=" << round << " count=" << count
         << " edges=" << edge_count << " roots=" << a << ',' << b);

    auto result = hg::dijkstra(f.g, roots, f.map());
    verify_search<G>(result, f.reference, {a, b});

    unsigned const target = random.bounded(count);
    auto to_result = hg::dijkstra_to(f.g, roots, at<G>(target), f.map());
    verify_target<G>(to_result, f.reference, {a, b}, target);
  }
}
} // namespace

TEST_CASE("Dijkstra Bellman-Ford model directed graph seed A", "[graph][dijkstra][model]")
{ run_random_model<graph<>>(0xA17F3021u, 36); }

TEST_CASE("Dijkstra Bellman-Ford model directed graph seed B", "[graph][dijkstra][model]")
{ run_random_model<graph<>>(0xDEADBEEFu, 36); }

TEST_CASE("Dijkstra Bellman-Ford model directed multigraph", "[graph][dijkstra][model]")
{ run_random_model<graph<true, true>>(0x1234BEEFu, 36); }

TEST_CASE("Dijkstra Bellman-Ford model outgoing-only graph", "[graph][dijkstra][model]")
{ run_random_model<graph<true, false, false, true>>(0xAFFEAFFEu, 36); }

TEST_CASE("Dijkstra Bellman-Ford model undirected graph", "[graph][dijkstra][model]")
{ run_random_model<graph<false>>(0x56781234u, 36); }

TEST_CASE("Dijkstra Bellman-Ford model undirected multigraph", "[graph][dijkstra][model]")
{ run_random_model<graph<false, true>>(0xF00DCAFEu, 36); }

TEST_CASE("Dijkstra Bellman-Ford model with small page size", "[graph][dijkstra][model]")
{ run_random_model<graph<true, true, true, true, 2>>(0xFEE1DEADu, 36); }

TEST_CASE("Dijkstra Bellman-Ford model with larger page size", "[graph][dijkstra][model]")
{ run_random_model<graph<true, false, true, true, 64>>(0x9ABCE120u, 36); }

TEST_CASE("Dijkstra Bellman-Ford model with undirected small pages", "[graph][dijkstra][model]")
{ run_random_model<graph<false, true, true, true, 4>>(0xABC13579u, 36); }

TEST_CASE("Dijkstra Bellman-Ford model includes zero-cost cycles", "[graph][dijkstra][model]")
{
  using G = graph<true, true>;
  weighted_graph<G> f{5};
  f.add(0, 1, 0);
  f.add(1, 2, 0);
  f.add(2, 0, 0);
  f.add(2, 3, 3);
  f.add(3, 4, 0);
  f.add(4, 2, 0);
  auto result = hg::dijkstra(f.g, at<G>(0), f.map());
  verify_search<G>(result, f.reference, {0});
}

TEST_CASE("Dijkstra Bellman-Ford model forces repeated decrease-key", "[graph][dijkstra][model]")
{
  using G = graph<>;
  weighted_graph<G> f{14};

  for (unsigned i = 1; i < 14; ++i)
  {
    f.add(0, i, 1000 - 3 * i);
    if (i > 1)
      f.add(i - 1, i, 1);
  }

  auto result = hg::dijkstra(f.g, at<G>(0), f.map());
  verify_search<G>(result, f.reference, {0});
}

TEST_CASE("Dijkstra Bellman-Ford model dense graph", "[graph][dijkstra][model]")
{
  using G = graph<true, true>;
  weighted_graph<G> f{10};

  for (unsigned u = 0; u < 10; ++u)
    for (unsigned v = 0; v < 10; ++v)
      f.add(u, v, (u * 13 + v * 17) % 31);

  auto result = hg::dijkstra(f.g, at<G>(0), f.map());
  verify_search<G>(result, f.reference, {0});
}
