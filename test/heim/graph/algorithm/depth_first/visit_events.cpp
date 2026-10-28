#include "visitors.hpp"

using namespace heim_test::graph_algorithm::dfs;

TEST_CASE("DFS visit emits the complete deterministic event sequence", "[graph][dfs][events]")
{
  auto g = make_event_graph();
  std::vector<std::string> events;
  trace_visitor vis{&events};
  std::array const roots{0, 3};
  REQUIRE(hg::depth_first_visit(g, roots, vis));

  std::vector<std::string> const expected{
      "I0", "I1", "I2", "I3", "I4",
      "S0", "D0", "E0", "T0", "D1", "E1", "T1", "D2",
      "E2", "B2", "Z2", "F2", "Z1", "F1", "Z0",
      "E3", "X3", "Z3", "E4", "B4", "Z4", "F0",
      "S3", "D3", "E5", "T5", "D4", "F4", "Z5", "F3"};
  REQUIRE(events == expected);
}

TEST_CASE("DFS initializes every vertex before the first start event", "[graph][dfs][events]")
{
  auto g = make_event_graph();
  std::vector<std::string> events;
  trace_visitor vis{&events};
  REQUIRE(hg::depth_first_visit(g, 0, vis));
  REQUIRE(events.size() >= 6);
  for (unsigned i = 0; i < 5; ++i)
    REQUIRE(events.at(i) == "I" + std::to_string(i));
  REQUIRE(events.at(5) == "S0");
}

TEST_CASE("DFS emits finish_edge after finishing a tree child", "[graph][dfs][events]")
{
  auto g = make_event_graph();
  std::vector<std::string> events;
  trace_visitor vis{&events};
  REQUIRE(hg::depth_first_visit(g, 0, vis));
  auto const finish_child = std::ranges::find(events, std::string{"F1"});
  auto const finish_edge = std::ranges::find(events, std::string{"Z0"});
  REQUIRE(finish_child < finish_edge);
}

TEST_CASE("DFS emits back_edge for a directed self-loop", "[graph][dfs][events]")
{
  auto g = make_event_graph();
  std::vector<std::string> events;
  trace_visitor vis{&events};
  REQUIRE(hg::depth_first_visit(g, 0, vis));
  REQUIRE(std::ranges::count(events, std::string{"B4"}) == 1);
}

TEST_CASE("DFS emits back_edge to an active ancestor", "[graph][dfs][events]")
{
  auto g = make_event_graph();
  std::vector<std::string> events;
  trace_visitor vis{&events};
  REQUIRE(hg::depth_first_visit(g, 0, vis));
  REQUIRE(std::ranges::count(events, std::string{"B2"}) == 1);
}

TEST_CASE("DFS emits forward_or_cross_edge to an already finished target", "[graph][dfs][events]")
{
  auto g = make_event_graph();
  std::vector<std::string> events;
  trace_visitor vis{&events};
  REQUIRE(hg::depth_first_visit(g, 0, vis));
  REQUIRE(std::ranges::count(events, std::string{"X3"}) == 1);
}

TEST_CASE("DFS finish_edge is emitted once for every examined non-skipped edge", "[graph][dfs][events]")
{
  auto g = make_event_graph();
  std::vector<std::string> events;
  trace_visitor vis{&events};
  std::array const roots{0, 3};
  REQUIRE(hg::depth_first_visit(g, roots, vis));
  REQUIRE(std::ranges::count_if(events, [](std::string const &e) { return e.starts_with('E'); }) == 6);
  REQUIRE(std::ranges::count_if(events, [](std::string const &e) { return e.starts_with('Z'); }) == 6);
}

TEST_CASE("DFS does not emit events for edges in unreachable components", "[graph][dfs][events]")
{
  auto g = make_event_graph();
  std::vector<std::string> events;
  trace_visitor vis{&events};
  REQUIRE(hg::depth_first_visit(g, 0, vis));
  REQUIRE(std::ranges::find(events, std::string{"E5"}) == events.end());
  REQUIRE(std::ranges::find(events, std::string{"D4"}) == events.end());
}

TEST_CASE("DFS visit with empty root range emits only initialize events", "[graph][dfs][events]")
{
  auto g = make_event_graph();
  std::vector<std::string> events;
  trace_visitor vis{&events};
  std::array<int, 0> roots{};
  REQUIRE(hg::depth_first_visit(g, roots, vis));
  REQUIRE((events == std::vector<std::string>{"I0", "I1", "I2", "I3", "I4"}));
}

TEST_CASE("DFS accepts visitors that implement only a subset of events", "[graph][dfs][events]")
{
  auto g = make_event_graph();
  unsigned discoveries{};
  struct partial_visitor
  {
    unsigned *count;
    void discover_vertex(external_graph const &, int) const { ++*count; }
  } vis{&discoveries};
  REQUIRE(hg::depth_first_visit(g, 0, vis));
  REQUIRE(discoveries == 3);
}

TEST_CASE("DFS default visitor handles missing hooks", "[graph][dfs][events]")
{
  auto g = make_event_graph();
  REQUIRE(hg::depth_first_visit(g, 0, std::monostate{}));
}
