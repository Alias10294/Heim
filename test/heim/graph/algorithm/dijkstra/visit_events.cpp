#include "reference.hpp"
#include "visitors.hpp"

using namespace heim_test::graph_algorithm::dijkstra;

TEST_CASE("Dijkstra visit initializes every vertex once", "[graph][dijkstra][events]")
{
  using G = graph<>;
  weighted_graph<G> f{4};
  std::vector<std::string> events;
  REQUIRE(runtime_visit(f.g, at<G>(0), f.map(), recording_visitor{&events}));

  for (unsigned i = 0; i < 4; ++i)
    REQUIRE(std::ranges::count(events, std::string{"I" + std::to_string(i)}) == 1);
}

TEST_CASE("Dijkstra visit emits source and discover events", "[graph][dijkstra][events]")
{
  using G = graph<>;
  weighted_graph<G> f{3};
  f.add(0, 1, 2);
  std::vector<std::string> events;
  REQUIRE(runtime_visit(f.g, at<G>(0), f.map(), recording_visitor{&events}));
  REQUIRE(std::ranges::count(events, std::string{"S0"}) == 1);
  REQUIRE(std::ranges::count(events, std::string{"D0"}) == 1);
  REQUIRE(std::ranges::count(events, std::string{"D1"}) == 1);
  REQUIRE(std::ranges::count(events, std::string{"D2"}) == 0);
}

TEST_CASE("Dijkstra visit emits examine and finish once per settled vertex", "[graph][dijkstra][events]")
{
  using G = graph<>;
  weighted_graph<G> f{3};
  f.add(0, 1, 4);
  f.add(1, 2, 3);
  std::vector<std::string> events;
  REQUIRE(runtime_visit(f.g, at<G>(0), f.map(), recording_visitor{&events}));
  for (unsigned i = 0; i < 3; ++i)
  {
    auto const v = std::to_string(i);
    REQUIRE(std::ranges::count(events, std::string{"V" + v}) == 1);
    REQUIRE(std::ranges::count(events, std::string{"F" + v}) == 1);
    auto const examine = std::ranges::find(events, std::string{"V" + v});
    auto const finish = std::ranges::find(events, std::string{"F" + v});
    REQUIRE(examine < finish);
  }
}

TEST_CASE("Dijkstra visit emits relaxed edge when a distance improves", "[graph][dijkstra][events]")
{
  using G = graph<>;
  weighted_graph<G> f{3};
  f.add(0, 1, 2);
  f.add(1, 2, 2);
  std::vector<std::string> events;
  REQUIRE(runtime_visit(f.g, at<G>(0), f.map(), recording_visitor{&events}));
  REQUIRE(std::ranges::count(events, std::string{"R0"}) == 1);
  REQUIRE(std::ranges::count(events, std::string{"R1"}) == 1);
}

TEST_CASE("Dijkstra visit emits non-relaxed edge when candidate is worse", "[graph][dijkstra][events]")
{
  using G = graph<>;
  weighted_graph<G> f{3};
  f.add(0, 1, 1);
  f.add(0, 2, 2);
  f.add(1, 2, 10);
  std::vector<std::string> events;
  REQUIRE(runtime_visit(f.g, at<G>(0), f.map(), recording_visitor{&events}));
  REQUIRE(std::ranges::count(events, std::string{"N2"}) == 1);
}

TEST_CASE("Dijkstra visit treats equal tentative paths as non-relaxations", "[graph][dijkstra][events]")
{
  using G = graph<>;
  weighted_graph<G> f{4};
  f.add(0, 1, 1);
  f.add(0, 2, 1);
  f.add(1, 3, 1);
  f.add(2, 3, 1);
  std::vector<std::string> events;
  REQUIRE(runtime_visit(f.g, at<G>(0), f.map(), recording_visitor{&events}));
  REQUIRE(std::ranges::count(events, std::string{"R2"})
        + std::ranges::count(events, std::string{"R3"}) == 1);
  REQUIRE(std::ranges::count(events, std::string{"N2"})
        + std::ranges::count(events, std::string{"N3"}) == 1);
}

TEST_CASE("Dijkstra visit discovers each vertex only on first relaxation", "[graph][dijkstra][events]")
{
  using G = graph<>;
  weighted_graph<G> f{3};
  f.add(0, 1, 10);
  f.add(0, 2, 1);
  f.add(2, 1, 1);
  std::vector<std::string> events;
  REQUIRE(runtime_visit(f.g, at<G>(0), f.map(), recording_visitor{&events}));
  REQUIRE(std::ranges::count(events, std::string{"D1"}) == 1);
  REQUIRE(std::ranges::count(events, std::string{"R0"}) == 1);
  REQUIRE(std::ranges::count(events, std::string{"R2"}) == 1);
}

TEST_CASE("Dijkstra visit reports no edge events for isolated source", "[graph][dijkstra][events]")
{
  using G = graph<>;
  weighted_graph<G> f{2};
  std::vector<std::string> events;
  REQUIRE(runtime_visit(f.g, at<G>(0), f.map(), recording_visitor{&events}));
  REQUIRE(std::ranges::none_of(events, [](std::string const &s)
  { return s.starts_with("E") || s.starts_with("R") || s.starts_with("N"); }));
}

TEST_CASE("Dijkstra visit examines each outgoing edge of settled vertices", "[graph][dijkstra][events]")
{
  using G = graph<>;
  weighted_graph<G> f{4};
  f.add(0, 1, 1);
  f.add(1, 2, 2);
  f.add(2, 3, 3);
  std::vector<std::string> events;
  REQUIRE(runtime_visit(f.g, at<G>(0), f.map(), recording_visitor{&events}));
  for (unsigned id = 0; id < 3; ++id)
    REQUIRE(std::ranges::count(events, std::string{"E" + std::to_string(id)}) == 1);
}

TEST_CASE("Dijkstra visit does not discover unreachable vertices", "[graph][dijkstra][events]")
{
  using G = graph<>;
  weighted_graph<G> f{4};
  f.add(0, 1, 1);
  f.add(2, 3, 1);
  std::vector<std::string> events;
  REQUIRE(runtime_visit(f.g, at<G>(0), f.map(), recording_visitor{&events}));
  REQUIRE(std::ranges::count(events, std::string{"D2"}) == 0);
  REQUIRE(std::ranges::count(events, std::string{"D3"}) == 0);
}
