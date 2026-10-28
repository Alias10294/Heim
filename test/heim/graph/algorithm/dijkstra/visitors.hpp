#ifndef HEIM_TEST_GRAPH_ALGORITHM_DIJKSTRA_VISITORS_HPP
#define HEIM_TEST_GRAPH_ALGORITHM_DIJKSTRA_VISITORS_HPP

#include "support.hpp"

namespace heim_test::graph_algorithm::dijkstra
{
struct recording_visitor
{
  std::vector<std::string> *events{};

  template<typename G>
  void initialize_vertex(G const &, vertex<G> v) const
  { events->push_back("I" + std::to_string(v.id())); }

  template<typename G>
  void source_vertex(G const &, vertex<G> v) const
  { events->push_back("S" + std::to_string(v.id())); }

  template<typename G>
  void discover_vertex(G const &, vertex<G> v) const
  { events->push_back("D" + std::to_string(v.id())); }

  template<typename G>
  void examine_vertex(G const &, vertex<G> v) const
  { events->push_back("V" + std::to_string(v.id())); }

  template<typename G>
  void finish_vertex(G const &, vertex<G> v) const
  { events->push_back("F" + std::to_string(v.id())); }

  template<typename G>
  void examine_edge(G const &, edge<G> e) const
  { events->push_back("E" + std::to_string(e.id())); }

  template<typename G>
  void relaxed_edge(G const &, edge<G> e) const
  { events->push_back("R" + std::to_string(e.id())); }

  template<typename G>
  void non_relaxed_edge(G const &, edge<G> e) const
  { events->push_back("N" + std::to_string(e.id())); }
};

struct empty_visitor
{ };

enum class event_kind
{
  initialize,
  source,
  discover,
  examine_vertex,
  finish,
  examine_edge,
  relaxed,
  non_relaxed
};

struct stopping_visitor
{
  event_kind stop_on;
  unsigned calls{};

  hg::visit check(event_kind kind)
  {
    ++calls;
    return kind == stop_on ? hg::visit::stop : hg::visit::proceed;
  }

  template<typename G>
  hg::visit initialize_vertex(G const &, vertex<G>)
  { return check(event_kind::initialize); }

  template<typename G>
  hg::visit source_vertex(G const &, vertex<G>)
  { return check(event_kind::source); }

  template<typename G>
  hg::visit discover_vertex(G const &, vertex<G>)
  { return check(event_kind::discover); }

  template<typename G>
  hg::visit examine_vertex(G const &, vertex<G>)
  { return check(event_kind::examine_vertex); }

  template<typename G>
  hg::visit finish_vertex(G const &, vertex<G>)
  { return check(event_kind::finish); }

  template<typename G>
  hg::visit examine_edge(G const &, edge<G>)
  { return check(event_kind::examine_edge); }

  template<typename G>
  hg::visit relaxed_edge(G const &, edge<G>)
  { return check(event_kind::relaxed); }

  template<typename G>
  hg::visit non_relaxed_edge(G const &, edge<G>)
  { return check(event_kind::non_relaxed); }
};

} // namespace heim_test::graph_algorithm::dijkstra

#endif // HEIM_TEST_GRAPH_ALGORITHM_DIJKSTRA_VISITORS_HPP
