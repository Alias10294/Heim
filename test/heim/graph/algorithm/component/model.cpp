#include "../derived/reference.hpp"

using namespace heim_test::graph_algorithm::derived;

namespace
{
template<typename G>
void run_model(std::uint32_t seed, unsigned rounds)
{
  random_generator random{seed};

  for (unsigned round = 0; round < rounds; ++round)
  {
    unsigned const count = 1 + random.bounded(24);
    unsigned const number_of_edges = random.bounded(5 * count + 1);
    G g;
    make_vertices(g, count);
    reference_graph reference{count};

    for (unsigned i = 0; i < number_of_edges; ++i)
    {
      unsigned const u = random.bounded(count);
      unsigned const v = random.bounded(count);
      auto [edge, inserted] = g.place_edge(at<G>(u), at<G>(v));
      (void)edge;

      if (inserted)
        reference.add(u, v);
    }

    INFO("seed=" << seed << " round=" << round << " vertices=" << count
         << " edges=" << number_of_edges);
    auto result = hg::cc(g);
    verify_components(g, result, reference);
  }
}
}

TEST_CASE("Connected components reference simple graph, first seed", "[graph][cc][model]")
{ run_model<graph<false>>(0xDEADBEEFu, 40); }

TEST_CASE("Connected components reference simple graph, second seed", "[graph][cc][model]")
{ run_model<graph<false>>(0x12345678u, 40); }

TEST_CASE("Connected components reference multigraph with loops", "[graph][cc][model]")
{ run_model<graph<false, true>>(0xC0FFEE01u, 40); }

TEST_CASE("Connected components reference multigraph with a different seed", "[graph][cc][model]")
{ run_model<graph<false, true>>(0x13579BDFu, 40); }

TEST_CASE("Connected components reference alternate page size", "[graph][cc][model]")
{ run_model<graph<false, false, true, true, 16>>(0xA5A5A5A5u, 40); }

TEST_CASE("Connected components reference on a complete graph", "[graph][cc][model]")
{
  using G = graph<false>;
  G g;
  make_vertices(g, 16);
  reference_graph reference{16};

  for (unsigned u = 0; u < 16; ++u)
    for (unsigned v = u + 1; v < 16; ++v)
    {
      (void)g.place_edge(at<G>(u), at<G>(v));
      reference.add(u, v);
    }

  verify_components(g, hg::cc(g), reference);
}

TEST_CASE("Connected components reference on disconnected cliques", "[graph][cc][model]")
{
  using G = graph<false>;
  G g;
  make_vertices(g, 12);
  reference_graph reference{12};

  for (unsigned first = 0; first < 12; first += 4)
    for (unsigned u = first; u < first + 4; ++u)
      for (unsigned v = u + 1; v < first + 4; ++v)
      {
        (void)g.place_edge(at<G>(u), at<G>(v));
        reference.add(u, v);
      }

  verify_components(g, hg::cc(g), reference);
}

TEST_CASE("Connected components reference on a large chain", "[graph][cc][model]")
{
  using G = graph<false>;
  G g;
  make_vertices(g, 128);
  reference_graph reference{128};

  for (unsigned u = 1; u < 128; ++u)
  {
    (void)g.place_edge(at<G>(u - 1), at<G>(u));
    reference.add(u - 1, u);
  }

  verify_components(g, hg::cc(g), reference);
}
