#include "visitors.hpp"

using namespace heim_test::graph_algorithm::bfs;

TEST_CASE("BFS visitor events follow the complete FIFO event sequence", "[graph][bfs][events]")
{
  external_graph g = make_external_graph();
  std::vector<std::string> events;

  REQUIRE(hg::breadth_first_visit(g, 0, trace_visitor{&events}));
  std::vector<std::string> const expected{
      "I0", "I1", "I2", "I3", "S0", "D0",
      "V0", "E0", "T0", "D1", "E1", "T1", "D2", "F0",
      "V1", "E2", "N2", "G2", "F1",
      "V2", "E3", "N3", "B3", "E4", "T4", "D3", "F2",
      "V3", "E5", "N5", "B5", "F3"};
  REQUIRE(events == expected);
}

TEST_CASE("BFS initialization visits even unreachable vertices", "[graph][bfs][events]")
{
  external_graph g{5};
  g.add_edge(0, 1);
  std::vector<std::string> events;

  REQUIRE(hg::breadth_first_visit(g, 0, trace_visitor{&events}));
  REQUIRE((std::vector<std::string>{events.begin(), events.begin() + 5}
           == std::vector<std::string>{"I0", "I1", "I2", "I3", "I4"}));
  REQUIRE(std::ranges::count(events, std::string{"D0"}) == 1);
  REQUIRE(std::ranges::count(events, std::string{"D1"}) == 1);
  REQUIRE(std::ranges::count(events, std::string{"D2"}) == 0);
}

TEST_CASE("BFS multiple roots are discovered before dequeuing starts", "[graph][bfs][events]")
{
  external_graph g{4};
  g.add_edge(0, 1);
  g.add_edge(2, 3);
  std::vector<std::string> events;
  std::array<int, 2> roots{0, 2};

  REQUIRE(hg::breadth_first_visit(g, roots, trace_visitor{&events}));
  REQUIRE((std::vector<std::string>{events.begin() + 4, events.begin() + 8}
           == std::vector<std::string>{"S0", "D0", "S2", "D2"}));
  REQUIRE(std::ranges::find(events, std::string{"V0"}) != events.end());
  REQUIRE(std::ranges::find(events, std::string{"V2"}) != events.end());
}

TEST_CASE("BFS visitor classifies parallel and loop edges without new discoveries", "[graph][bfs][events]")
{
  external_graph g{2};
  g.add_edge(0, 1);
  g.add_edge(0, 1);
  g.add_edge(0, 0);
  g.add_edge(1, 1);

  std::vector<std::string> events;
  REQUIRE(hg::breadth_first_visit(g, 0, trace_visitor{&events}));
  REQUIRE(std::ranges::count(events, std::string{"D1"}) == 1);
  REQUIRE(std::ranges::count(events, std::string{"T0"}) == 1);
  REQUIRE(std::ranges::count(events, std::string{"N1"}) == 1);
  REQUIRE(std::ranges::count(events, std::string{"G1"}) == 1);
  REQUIRE(std::ranges::count(events, std::string{"N2"}) == 1);
  REQUIRE(std::ranges::count(events, std::string{"G2"}) == 1);
  REQUIRE(std::ranges::count(events, std::string{"N3"}) == 1);
  REQUIRE(std::ranges::count(events, std::string{"G3"}) == 1);
}
