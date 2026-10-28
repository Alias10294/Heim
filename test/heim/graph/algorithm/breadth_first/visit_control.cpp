#include "visitors.hpp"

using namespace heim_test::graph_algorithm::bfs;

TEST_CASE("BFS stop is propagated from every supported event", "[graph][bfs][control]")
{
  std::array const stops{
      event_kind::initialize, event_kind::source, event_kind::discover,
      event_kind::examine_vertex, event_kind::finish, event_kind::examine_edge,
      event_kind::tree, event_kind::non_tree, event_kind::gray, event_kind::black};

  for (auto event : stops)
  {
    INFO("selected event=" << static_cast<int>(event));
    external_graph g = make_external_graph();
    unsigned hits{};
    control_visitor visitor{event, hg::visit::stop, &hits};

    REQUIRE_FALSE(hg::breadth_first_visit(g, 0, visitor));
    REQUIRE(hits == 1);
  }
}

TEST_CASE("BFS skip at source prevents that root from being discovered", "[graph][bfs][control]")
{
  external_graph g{3};
  g.add_edge(0, 1);
  std::unordered_map<int, hg::color> colors;
  external_map<int, hg::color> cm{&colors};
  unsigned hits{};

  REQUIRE(hg::breadth_first_visit(
      g, 0, control_visitor{event_kind::source, hg::visit::skip, &hits}, cm));
  REQUIRE(hits == 1);
  REQUIRE(colors.at(0) == hg::color::white);
  REQUIRE(colors.at(1) == hg::color::white);
}

TEST_CASE("BFS skip at one source permits subsequent roots", "[graph][bfs][control]")
{
  external_graph g{3};
  std::array<int, 2> roots{0, 1};
  std::unordered_map<int, hg::color> colors;
  unsigned skipped{};

  struct visitor
  {
    unsigned *skipped;
    hg::visit source_vertex(external_graph const &, int v) const
    {
      if (v == 0)
      {
        ++*skipped;
        return hg::visit::skip;
      }
      return hg::visit::proceed;
    }
  };

  REQUIRE(hg::breadth_first_visit(g, roots, visitor{&skipped}, external_map<int, hg::color>{&colors}));
  REQUIRE(skipped == 1);
  REQUIRE(colors.at(0) == hg::color::white);
  REQUIRE(colors.at(1) == hg::color::black);
}

TEST_CASE("BFS prune at source discovery finishes root without examining edges", "[graph][bfs][control]")
{
  external_graph g{2};
  g.add_edge(0, 1);
  std::unordered_map<int, hg::color> colors;
  std::vector<std::string> events;

  struct visitor
  {
    std::vector<std::string> *events;
    hg::visit discover_vertex(external_graph const &, int v) const
    {
      events->push_back("D" + std::to_string(v));
      return hg::visit::prune;
    }
    void finish_vertex(external_graph const &, int v) const
    { events->push_back("F" + std::to_string(v)); }
    void examine_edge(external_graph const &, int) const
    { events->push_back("E"); }
  };

  REQUIRE(hg::breadth_first_visit(g, 0, visitor{&events}, external_map<int, hg::color>{&colors}));
  REQUIRE((events == std::vector<std::string>{"D0", "F0"}));
  REQUIRE(colors.at(0) == hg::color::black);
  REQUIRE(colors.at(1) == hg::color::white);
}

TEST_CASE("BFS prune at a discovered child prevents its descendants", "[graph][bfs][control]")
{
  external_graph g{3};
  g.add_edge(0, 1);
  g.add_edge(1, 2);
  std::unordered_map<int, hg::color> colors;

  struct visitor
  {
    hg::visit discover_vertex(external_graph const &, int v) const
    { return v == 1 ? hg::visit::prune : hg::visit::proceed; }
  };

  REQUIRE(hg::breadth_first_visit(g, 0, visitor{}, external_map<int, hg::color>{&colors}));
  REQUIRE(colors.at(0) == hg::color::black);
  REQUIRE(colors.at(1) == hg::color::black);
  REQUIRE(colors.at(2) == hg::color::white);
}

TEST_CASE("BFS prune at examine_vertex skips all outgoing edges", "[graph][bfs][control]")
{
  external_graph g{3};
  g.add_edge(0, 1);
  g.add_edge(1, 2);
  std::unordered_map<int, hg::color> colors;
  unsigned finished{};

  struct visitor
  {
    unsigned *finished;
    hg::visit examine_vertex(external_graph const &, int v) const
    { return v == 1 ? hg::visit::prune : hg::visit::proceed; }
    void finish_vertex(external_graph const &, int) const { ++*finished; }
  };

  REQUIRE(hg::breadth_first_visit(g, 0, visitor{&finished}, external_map<int, hg::color>{&colors}));
  REQUIRE(finished == 2);
  REQUIRE(colors.at(1) == hg::color::black);
  REQUIRE(colors.at(2) == hg::color::white);
}

TEST_CASE("BFS skip at examine_edge does not mark its target discovered", "[graph][bfs][control]")
{
  external_graph g{3};
  g.add_edge(0, 1);
  g.add_edge(0, 2);
  std::unordered_map<int, hg::color> colors;

  struct visitor
  {
    hg::visit examine_edge(external_graph const &, int e) const
    { return e == 0 ? hg::visit::skip : hg::visit::proceed; }
  };

  REQUIRE(hg::breadth_first_visit(g, 0, visitor{}, external_map<int, hg::color>{&colors}));
  REQUIRE(colors.at(1) == hg::color::white);
  REQUIRE(colors.at(2) == hg::color::black);
}

TEST_CASE("BFS skip at non_tree_edge suppresses gray and black callbacks", "[graph][bfs][control]")
{
  external_graph g = make_external_graph();
  unsigned non_tree{};
  unsigned colored{};

  struct visitor
  {
    unsigned *non_tree;
    unsigned *colored;
    hg::visit non_tree_edge(external_graph const &, int) const
    {
      ++*non_tree;
      return hg::visit::skip;
    }
    void gray_target(external_graph const &, int) const { ++*colored; }
    void black_target(external_graph const &, int) const { ++*colored; }
  };

  REQUIRE(hg::breadth_first_visit(g, 0, visitor{&non_tree, &colored}));
  REQUIRE(non_tree == 3);
  REQUIRE(colored == 0);
}
