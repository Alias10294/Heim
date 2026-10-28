#ifndef HEIM_TEST_GRAPH_ALGORITHM_BREADTH_FIRST_VISITORS_HPP
#define HEIM_TEST_GRAPH_ALGORITHM_BREADTH_FIRST_VISITORS_HPP

#include "support.hpp"

namespace heim_test::graph_algorithm::bfs
{
struct trace_visitor
{
  std::vector<std::string> *events{};

  void record(char const *event, int value) const
  { events->push_back(std::string{event} + std::to_string(value)); }

  void initialize_vertex(external_graph const &, int v) const { record("I", v); }
  void source_vertex(external_graph const &, int v) const { record("S", v); }
  void discover_vertex(external_graph const &, int v) const { record("D", v); }
  void examine_vertex(external_graph const &, int v) const { record("V", v); }
  void finish_vertex(external_graph const &, int v) const { record("F", v); }
  void examine_edge(external_graph const &, int e) const { record("E", e); }
  void tree_edge(external_graph const &, int e) const { record("T", e); }
  void non_tree_edge(external_graph const &, int e) const { record("N", e); }
  void gray_target(external_graph const &, int e) const { record("G", e); }
  void black_target(external_graph const &, int e) const { record("B", e); }
};

enum class event_kind
{
  initialize, source, discover, examine_vertex, finish,
  examine_edge, tree, non_tree, gray, black
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
  hg::visit source_vertex(external_graph const &, int) const { return report(event_kind::source); }
  hg::visit discover_vertex(external_graph const &, int) const { return report(event_kind::discover); }
  hg::visit examine_vertex(external_graph const &, int) const { return report(event_kind::examine_vertex); }
  hg::visit finish_vertex(external_graph const &, int) const { return report(event_kind::finish); }
  hg::visit examine_edge(external_graph const &, int) const { return report(event_kind::examine_edge); }
  hg::visit tree_edge(external_graph const &, int) const { return report(event_kind::tree); }
  hg::visit non_tree_edge(external_graph const &, int) const { return report(event_kind::non_tree); }
  hg::visit gray_target(external_graph const &, int) const { return report(event_kind::gray); }
  hg::visit black_target(external_graph const &, int) const { return report(event_kind::black); }
};

} // namespace heim_test::graph_algorithm::bfs

#endif // HEIM_TEST_GRAPH_ALGORITHM_BREADTH_FIRST_VISITORS_HPP
