#include "visitors.hpp"

using namespace heim_test::graph_algorithm::dfs;

namespace
{
void assert_stop(event_kind selected)
{
  auto g = make_event_graph();
  unsigned hits{};
  control_visitor visitor{selected, hg::visit::stop, &hits};
  REQUIRE_FALSE(hg::depth_first_visit(g, std::array{0, 3}, visitor));
  REQUIRE(hits >= 1);
}
}

TEST_CASE("DFS initialize_vertex can stop before visiting a root", "[graph][dfs][control]")
{ assert_stop(event_kind::initialize); }

TEST_CASE("DFS start_vertex can stop before discovery", "[graph][dfs][control]")
{ assert_stop(event_kind::start); }

TEST_CASE("DFS discover_vertex can stop in a recursive call", "[graph][dfs][control]")
{ assert_stop(event_kind::discover); }

TEST_CASE("DFS examine_edge can stop before classifying an edge", "[graph][dfs][control]")
{ assert_stop(event_kind::examine_edge); }

TEST_CASE("DFS tree_edge can stop before descending", "[graph][dfs][control]")
{ assert_stop(event_kind::tree); }

TEST_CASE("DFS back_edge can stop from a gray target", "[graph][dfs][control]")
{ assert_stop(event_kind::back); }

TEST_CASE("DFS forward_or_cross_edge can stop from a black target", "[graph][dfs][control]")
{ assert_stop(event_kind::forward_or_cross); }

TEST_CASE("DFS finish_edge can stop while unwinding the recursion", "[graph][dfs][control]")
{ assert_stop(event_kind::finish_edge); }

TEST_CASE("DFS finish_vertex can stop after completing a vertex", "[graph][dfs][control]")
{ assert_stop(event_kind::finish); }

TEST_CASE("DFS start_vertex skip ignores a root but continues with another", "[graph][dfs][control]")
{
  external_graph g{3};
  g.add_edge(0, 1);
  unsigned starts{};
  std::vector<int> discovered;
  struct visitor
  {
    unsigned *starts;
    std::vector<int> *discovered;

    hg::visit start_vertex(external_graph const &, int v) const
    { ++*starts; return v == 0 ? hg::visit::skip : hg::visit::proceed; }
    void discover_vertex(external_graph const &, int v) const { discovered->push_back(v); }
  } vis{&starts, &discovered};
  REQUIRE(hg::depth_first_visit(g, std::array{0, 2}, vis));
  REQUIRE(starts == 2);
  REQUIRE((discovered == std::vector<int>{2}));
}

TEST_CASE("DFS discover_vertex prune finishes the root without examining edges", "[graph][dfs][control]")
{
  auto g = make_event_graph();
  unsigned examined{};
  unsigned finished{};
  struct visitor
  {
    unsigned *examined;
    unsigned *finished;
    hg::visit discover_vertex(external_graph const &, int) const { return hg::visit::prune; }
    void examine_edge(external_graph const &, int) const { ++*examined; }
    void finish_vertex(external_graph const &, int) const { ++*finished; }
  } vis{&examined, &finished};
  REQUIRE(hg::depth_first_visit(g, 0, vis));
  REQUIRE(examined == 0);
  REQUIRE(finished == 1);
}

TEST_CASE("DFS discover_vertex prune finishes a child and resumes the parent", "[graph][dfs][control]")
{
  external_graph g{3};
  g.add_edge(0, 1);
  g.add_edge(0, 2);
  unsigned examined{};
  std::vector<int> finished;
  struct visitor
  {
    unsigned *examined;
    std::vector<int> *finished;
    hg::visit discover_vertex(external_graph const &, int v) const
    { return v == 1 ? hg::visit::prune : hg::visit::proceed; }
    void examine_edge(external_graph const &, int) const { ++*examined; }
    void finish_vertex(external_graph const &, int v) const { finished->push_back(v); }
  } vis{&examined, &finished};
  REQUIRE(hg::depth_first_visit(g, 0, vis));
  REQUIRE(examined == 2);
  REQUIRE((finished == std::vector<int>{1, 2, 0}));
}

TEST_CASE("DFS examine_edge skip prevents tree discovery", "[graph][dfs][control]")
{
  external_graph g{3};
  g.add_edge(0, 1);
  g.add_edge(0, 2);
  std::vector<int> discovered;
  unsigned finished_edges{};
  struct visitor
  {
    std::vector<int> *discovered;
    unsigned *finished_edges;
    hg::visit examine_edge(external_graph const &, int e) const
    { return e == 0 ? hg::visit::skip : hg::visit::proceed; }
    void discover_vertex(external_graph const &, int v) const { discovered->push_back(v); }
    void finish_edge(external_graph const &, int) const { ++*finished_edges; }
  } vis{&discovered, &finished_edges};
  REQUIRE(hg::depth_first_visit(g, 0, vis));
  REQUIRE((discovered == std::vector<int>{0, 2}));
  REQUIRE(finished_edges == 1);
}

TEST_CASE("DFS stop on a child prevents the finish events of its ancestors", "[graph][dfs][control]")
{
  external_graph g{3};
  g.add_edge(0, 1);
  g.add_edge(1, 2);
  std::vector<int> finished;
  struct visitor
  {
    std::vector<int> *finished;
    hg::visit discover_vertex(external_graph const &, int v) const
    { return v == 2 ? hg::visit::stop : hg::visit::proceed; }
    void finish_vertex(external_graph const &, int v) const { finished->push_back(v); }
  } vis{&finished};
  REQUIRE_FALSE(hg::depth_first_visit(g, 0, vis));
  REQUIRE(finished.empty());
}
