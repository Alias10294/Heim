#include "../derived/reference.hpp"

using namespace heim_test::graph_algorithm::derived;

namespace
{
template<typename G>
void run_dag_model(std::uint32_t seed, unsigned rounds)
{
  random_generator random{seed};

  for (unsigned round = 0; round < rounds; ++round)
  {
    unsigned const count = 1 + random.bounded(22);
    unsigned const number_of_edges = random.bounded(5 * count + 1);
    G g;
    make_vertices(g, count);
    reference_graph reference{count};
    std::vector<unsigned> order(count);
    std::iota(order.begin(), order.end(), 0u);

    for (unsigned i = count; i > 1; --i)
      std::swap(order[i - 1], order[random.bounded(i)]);

    for (unsigned i = 0; i < number_of_edges; ++i)
    {
      unsigned const first = random.bounded(count);
      unsigned const second = random.bounded(count);

      if (first == second)
        continue;

      unsigned const u = order[std::min(first, second)];
      unsigned const v = order[std::max(first, second)];
      auto [edge, inserted] = g.place_edge(at<G>(u), at<G>(v));
      (void)edge;

      if (inserted)
        reference.add(u, v);
    }

    INFO("seed=" << seed << " round=" << round << " vertices=" << count
         << " edges=" << number_of_edges);
    verify_topological(g, reference);
  }
}
}

TEST_CASE("Topological reference random simple DAG, first seed", "[graph][topo][model]")
{ run_dag_model<graph<true>>(0xDEADBEEFu, 48); }

TEST_CASE("Topological reference random simple DAG, second seed", "[graph][topo][model]")
{ run_dag_model<graph<true>>(0x12345678u, 48); }

TEST_CASE("Topological reference random multigraph", "[graph][topo][model]")
{ run_dag_model<graph<true, true>>(0xC0FFEE01u, 48); }

TEST_CASE("Topological reference random outgoing-only graph", "[graph][topo][model]")
{ run_dag_model<graph<true, false, false, true>>(0xA5A5A5A5u, 48); }

TEST_CASE("Topological reference random alternate page size", "[graph][topo][model]")
{ run_dag_model<graph<true, false, true, true, 16>>(0x13579BDFu, 48); }

TEST_CASE("Topological reference dense DAG", "[graph][topo][model]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 16);
  reference_graph reference{16};

  for (unsigned u = 0; u < 16; ++u)
    for (unsigned v = u + 1; v < 16; ++v)
    {
      (void)g.place_edge(at<G>(u), at<G>(v));
      reference.add(u, v);
    }

  verify_topological(g, reference);
}

TEST_CASE("Topological reference long directed chain", "[graph][topo][model]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 96);
  reference_graph reference{96};

  for (unsigned u = 1; u < 96; ++u)
  {
    (void)g.place_edge(at<G>(u - 1), at<G>(u));
    reference.add(u - 1, u);
  }

  verify_topological(g, reference);
}

TEST_CASE("Topological reference with sparse reversed vertex numbering", "[graph][topo][model]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 8);
  reference_graph reference{8};
  for (unsigned u = 7; u > 0; --u)
  {
    (void)g.place_edge(at<G>(u), at<G>(u - 1));
    reference.add(u, u - 1);
  }
  verify_topological(g, reference);
}
