#include "reference.hpp"
#include "visitors.hpp"

using namespace heim_test::graph_algorithm::dijkstra;

namespace
{
void run_stop(event_kind event)
{
  using G = graph<>;
  weighted_graph<G> f{4};
  f.add(0, 1, 1);
  f.add(0, 2, 2);
  f.add(1, 2, 10);
  f.add(2, 3, 1);
  stopping_visitor visitor{event};
  REQUIRE_FALSE(runtime_visit(f.g, at<G>(0), f.map(), visitor));
  REQUIRE(visitor.calls >= 1);
}
} // namespace

TEST_CASE("Dijkstra visit stops during initialization", "[graph][dijkstra][control]")
{ run_stop(event_kind::initialize); }

TEST_CASE("Dijkstra visit stops during source event", "[graph][dijkstra][control]")
{ run_stop(event_kind::source); }

TEST_CASE("Dijkstra visit stops during discovery event", "[graph][dijkstra][control]")
{ run_stop(event_kind::discover); }

TEST_CASE("Dijkstra visit stops during examine vertex event", "[graph][dijkstra][control]")
{ run_stop(event_kind::examine_vertex); }

TEST_CASE("Dijkstra visit stops during finish vertex event", "[graph][dijkstra][control]")
{ run_stop(event_kind::finish); }

TEST_CASE("Dijkstra visit stops during examine edge event", "[graph][dijkstra][control]")
{ run_stop(event_kind::examine_edge); }

TEST_CASE("Dijkstra visit stops during relaxed edge event", "[graph][dijkstra][control]")
{ run_stop(event_kind::relaxed); }

TEST_CASE("Dijkstra visit stops during non-relaxed edge event", "[graph][dijkstra][control]")
{ run_stop(event_kind::non_relaxed); }

TEST_CASE("Dijkstra visit can skip a source and continue with later roots", "[graph][dijkstra][control]")
{
  using G = graph<>;
  weighted_graph<G> f{4};
  f.add(0, 1, 1);
  f.add(2, 3, 2);
  std::array const roots{at<G>(0), at<G>(2)};
  struct visitor
  {
    hg::visit source_vertex(G const &, vertex<G> v) const
    { return v.id() == 0 ? hg::visit::skip : hg::visit::proceed; }
  } vis;
  std::unordered_map<vertex<G>, unsigned> distance;
  using DM = external_map<vertex<G>, unsigned>;
  REQUIRE(runtime_visit(f.g, roots, f.map(), vis, DM{&distance}));
  REQUIRE(distance.at(at<G>(0)) == std::numeric_limits<unsigned>::max());
  REQUIRE(distance.at(at<G>(1)) == std::numeric_limits<unsigned>::max());
  REQUIRE(distance.at(at<G>(2)) == 0);
  REQUIRE(distance.at(at<G>(3)) == 2);
}

TEST_CASE("Dijkstra visit can skip one edge before relaxation", "[graph][dijkstra][control]")
{
  using G = graph<>;
  weighted_graph<G> f{3};
  f.add(0, 1, 2);
  f.add(0, 2, 4);
  struct visitor
  {
    hg::visit examine_edge(G const &, edge<G> e) const
    { return e.id() == 0 ? hg::visit::skip : hg::visit::proceed; }
  } vis;
  std::unordered_map<vertex<G>, unsigned> distance;
  using DM = external_map<vertex<G>, unsigned>;
  REQUIRE(runtime_visit(f.g, at<G>(0), f.map(), vis, DM{&distance}));
  REQUIRE(distance.at(at<G>(1)) == std::numeric_limits<unsigned>::max());
  REQUIRE(distance.at(at<G>(2)) == 4);
}

TEST_CASE("Dijkstra visit can prune an examined vertex", "[graph][dijkstra][control]")
{
  using G = graph<>;
  weighted_graph<G> f{4};
  f.add(0, 1, 2);
  f.add(1, 2, 2);
  f.add(2, 3, 2);
  struct visitor
  {
    hg::visit examine_vertex(G const &, vertex<G> v) const
    { return v.id() == 1 ? hg::visit::prune : hg::visit::proceed; }
  } vis;
  std::unordered_map<vertex<G>, unsigned> distance;
  using DM = external_map<vertex<G>, unsigned>;
  REQUIRE(runtime_visit(f.g, at<G>(0), f.map(), vis, DM{&distance}));
  REQUIRE(distance.at(at<G>(1)) == 2);
  REQUIRE(distance.at(at<G>(2)) == std::numeric_limits<unsigned>::max());
}

TEST_CASE("Dijkstra visit accepts a visitor defining only an edge event", "[graph][dijkstra][control]")
{
  using G = graph<>;
  weighted_graph<G> f{3};
  f.add(0, 1, 2);
  f.add(1, 2, 3);
  unsigned observed{};
  struct visitor
  {
    unsigned *calls;
    void relaxed_edge(G const &, edge<G>) const
    { ++*calls; }
  } vis{&observed};
  REQUIRE(runtime_visit(f.g, at<G>(0), f.map(), vis));
  REQUIRE(observed == 2);
}

TEST_CASE("Dijkstra visit completes with a visitor returning proceed explicitly", "[graph][dijkstra][control]")
{
  using G = graph<>;
  weighted_graph<G> f{3};
  f.add(0, 1, 3);
  f.add(1, 2, 1);
  stopping_visitor visitor{event_kind::non_relaxed};
  // This graph has no non-relaxed edge; all visitor callbacks proceed.
  REQUIRE(runtime_visit(f.g, at<G>(0), f.map(), visitor));
  REQUIRE(visitor.calls > 0);
}
