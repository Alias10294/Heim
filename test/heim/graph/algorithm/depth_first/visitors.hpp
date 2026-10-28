#ifndef HEIM_TEST_GRAPH_ALGORITHM_DEPTH_FIRST_VISITORS_HPP
#define HEIM_TEST_GRAPH_ALGORITHM_DEPTH_FIRST_VISITORS_HPP

#include "support.hpp"

namespace heim_test::graph_algorithm::dfs
{
struct trace_visitor
{
  std::vector<std::string> *events{};

  void record(char const *event, int value) const
  { events->push_back(std::string{event} + std::to_string(value)); }

  void initialize_vertex(external_graph const &, int v) const { record("I", v); }
  void start_vertex(external_graph const &, int v) const { record("S", v); }
  void discover_vertex(external_graph const &, int v) const { record("D", v); }
  void finish_vertex(external_graph const &, int v) const { record("F", v); }
  void examine_edge(external_graph const &, int e) const { record("E", e); }
  void tree_edge(external_graph const &, int e) const { record("T", e); }
  void back_edge(external_graph const &, int e) const { record("B", e); }
  void forward_or_cross_edge(external_graph const &, int e) const { record("X", e); }
  void finish_edge(external_graph const &, int e) const { record("Z", e); }
};

enum class event_kind
{
  initialize, start, discover, finish, examine_edge,
  tree, back, forward_or_cross, finish_edge
};

struct control_visitor
{
  event_kind selected;
  hg::visit action;
  unsigned *hits;

  hg::visit report(event_kind kind) const
  {
    if (selected == kind)
    {
      ++*hits;
      return action;
    }
    return hg::visit::proceed;
  }

  hg::visit initialize_vertex(external_graph const &, int) const { return report(event_kind::initialize); }
  hg::visit start_vertex(external_graph const &, int) const { return report(event_kind::start); }
  hg::visit discover_vertex(external_graph const &, int) const { return report(event_kind::discover); }
  hg::visit finish_vertex(external_graph const &, int) const { return report(event_kind::finish); }
  hg::visit examine_edge(external_graph const &, int) const { return report(event_kind::examine_edge); }
  hg::visit tree_edge(external_graph const &, int) const { return report(event_kind::tree); }
  hg::visit back_edge(external_graph const &, int) const { return report(event_kind::back); }
  hg::visit forward_or_cross_edge(external_graph const &, int) const { return report(event_kind::forward_or_cross); }
  hg::visit finish_edge(external_graph const &, int) const { return report(event_kind::finish_edge); }
};

inline external_graph make_event_graph()
{
  external_graph g{5};
  g.add_edge(0, 1); // 0: tree
  g.add_edge(1, 2); // 1: tree
  g.add_edge(2, 0); // 2: back edge to gray ancestor
  g.add_edge(0, 2); // 3: forward edge to black descendant
  g.add_edge(0, 0); // 4: self-loop back edge
  g.add_edge(3, 4); // 5: separate tree
  return g;
}

} // namespace heim_test::graph_algorithm::dfs

#endif // HEIM_TEST_GRAPH_ALGORITHM_DEPTH_FIRST_VISITORS_HPP
