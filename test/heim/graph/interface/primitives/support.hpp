#ifndef HEIM_TEST_GRAPH_INTERFACE_PRIMITIVES_SUPPORT_HPP
#define HEIM_TEST_GRAPH_INTERFACE_PRIMITIVES_SUPPORT_HPP

#include <heim/graph/interface/primitives.hpp>

#include <array>
#include <cstddef>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace heim_test::graph_interface
{
namespace graphs = heim::graphs;

// A graph for which every customization uses an ordinary member function.
struct member_graph
{
  using vertex_type = int;
  using edge_type = int;
  static constexpr bool is_directed = true;
  static constexpr bool is_multigraph = false;

  std::array<int, 3> vertex_ids{0, 1, 2};
  std::array<int, 2> edge_ids{10, 11};
  std::array<int, 2> outgoing{10, 11};
  std::array<int, 1> incoming{11};
  int next_vertex{3};
  int next_edge{12};
  int action{};

  constexpr auto vertices() const noexcept { return std::span{vertex_ids}; }
  constexpr auto edges() const noexcept { return std::span{edge_ids}; }
  constexpr auto out_edges(int) const noexcept { return std::span{outgoing}; }
  constexpr auto in_edges(int) const noexcept { return std::span{incoming}; }
  constexpr int vertex_count() const noexcept { return 3; }
  constexpr int edge_count() const noexcept { return 2; }
  constexpr int out_edge_count(int) const noexcept { return 2; }
  constexpr int in_edge_count(int) const noexcept { return 1; }
  constexpr int total_edge_count(int) const noexcept { return 3; }
  constexpr int source(int) const noexcept { return 0; }
  constexpr int target(int) const noexcept { return 1; }
  constexpr auto endpoints(int) const noexcept { return std::pair{0, 1}; }

  constexpr int place_vertex() noexcept { action = 1; return next_vertex++; }
  constexpr auto place_edge(int, int) noexcept { action = 2; return std::pair{next_edge++, true}; }
  constexpr void remove_vertex(int) noexcept { action = 3; }
  constexpr void remove_edge(int) noexcept { action = 4; }
  constexpr void remove_edges(int, int) noexcept { action = 5; }
  constexpr void clear_vertex(int) noexcept { action = 6; }
};

// Competing ADL overloads must not supersede viable member functions.
constexpr auto vertices(member_graph const &) noexcept { return std::array{99}; }
constexpr auto edges(member_graph const &) noexcept { return std::array{99}; }
constexpr int vertex_count(member_graph const &) noexcept { return 99; }
constexpr int edge_count(member_graph const &) noexcept { return 99; }
constexpr int out_edge_count(member_graph const &, int) noexcept { return 99; }
constexpr int in_edge_count(member_graph const &, int) noexcept { return 99; }
constexpr int total_edge_count(member_graph const &, int) noexcept { return 99; }
constexpr int source(member_graph const &, int) noexcept { return 99; }
constexpr int target(member_graph const &, int) noexcept { return 99; }
constexpr auto endpoints(member_graph const &, int) noexcept { return std::pair{99, 99}; }
constexpr int place_vertex(member_graph &) noexcept { return 99; }
constexpr auto place_edge(member_graph &, int, int) noexcept { return std::pair{99, false}; }
constexpr void remove_vertex(member_graph &g, int) noexcept { g.action = 99; }
constexpr void remove_edge(member_graph &g, int) noexcept { g.action = 99; }
constexpr void remove_edges(member_graph &g, int, int) noexcept { g.action = 99; }
constexpr void clear_vertex(member_graph &g, int) noexcept { g.action = 99; }
constexpr auto out_edges(member_graph const &, int) noexcept { return std::array{99}; }
constexpr auto in_edges(member_graph const &, int) noexcept { return std::array{99}; }

namespace adl
{
struct graph
{
  using vertex_type = int;
  using edge_type = int;
  static constexpr bool is_directed = true;
  static constexpr bool is_multigraph = true;
  std::array<int, 3> vs{0, 1, 2};
  std::array<int, 2> es{12, 13};
  std::array<int, 1> outs{12};
  std::array<int, 1> ins{13};
  int action{};
};
constexpr auto vertices(graph const &g) noexcept { return std::span{g.vs}; }
constexpr auto edges(graph const &g) noexcept { return std::span{g.es}; }
constexpr auto out_edges(graph const &g, int) noexcept { return std::span{g.outs}; }
constexpr auto in_edges(graph const &g, int) noexcept { return std::span{g.ins}; }
constexpr int vertex_count(graph const &) noexcept { return 3; }
constexpr int edge_count(graph const &) noexcept { return 2; }
constexpr int out_edge_count(graph const &, int) noexcept { return 1; }
constexpr int in_edge_count(graph const &, int) noexcept { return 1; }
constexpr int total_edge_count(graph const &, int) noexcept { return 2; }
constexpr int source(graph const &, int) noexcept { return 0; }
constexpr int target(graph const &, int) noexcept { return 1; }
constexpr auto endpoints(graph const &, int) noexcept { return std::pair{0, 1}; }
constexpr int place_vertex(graph &g) noexcept { g.action = 1; return 3; }
constexpr auto place_edge(graph &g, int, int) noexcept { g.action = 2; return std::pair{14, true}; }
constexpr void remove_vertex(graph &g, int) noexcept { g.action = 3; }
constexpr void remove_edge(graph &g, int) noexcept { g.action = 4; }
constexpr void remove_edges(graph &g, int, int) noexcept { g.action = 5; }
constexpr void clear_vertex(graph &g, int) noexcept { g.action = 6; }
}

// No member count or endpoint operations: exercise the documented fallbacks.
template<bool Directed>
struct fallback_graph
{
  using vertex_type = int;
  using edge_type = int;
  static constexpr bool is_directed = Directed;
  static constexpr bool is_multigraph = false;
  std::array<int, 4> vs{0, 1, 2, 3};
  std::array<int, 3> es{11, 12, 13};
  std::array<int, 2> outs{11, 12};
  std::array<int, 1> ins{13};

  constexpr auto vertices() const noexcept { return std::span{vs}; }
  constexpr auto edges() const noexcept { return std::span{es}; }
  constexpr auto out_edges(int) const noexcept { return std::span{outs}; }
  constexpr auto in_edges(int) const noexcept { return std::span{ins}; }
  constexpr int source(int) const noexcept { return 0; }
  constexpr int target(int) const noexcept { return 2; }
};

struct incoming_only_graph
{
  using vertex_type = int;
  using edge_type = int;
  static constexpr bool is_directed = false;
  static constexpr bool is_multigraph = false;
  std::array<int, 2> incoming{8, 9};
  constexpr auto in_edges(int) const noexcept { return std::span{incoming}; }
};

// A member returning an invalid range must not hide a viable ADL customization.
namespace invalid_member
{
struct graph
{
  using vertex_type = int;
  using edge_type = int;
  static constexpr bool is_directed = true;
  static constexpr bool is_multigraph = false;
  int vertices() const noexcept { return 0; }
  int source(int) const noexcept = delete;
};
constexpr auto vertices(graph const &) noexcept { return std::array{4, 5}; }
constexpr int source(graph const &, int) noexcept { return 4; }
}

struct empty_graph
{
  using vertex_type = int;
  using edge_type = int;
  static constexpr bool is_directed = false;
  static constexpr bool is_multigraph = false;
};

struct throwing_graph
{
  using vertex_type = int;
  using edge_type = int;
  static constexpr bool is_directed = true;
  static constexpr bool is_multigraph = false;
  int vertex_count() const { throw std::runtime_error{"count failed"}; }
  void remove_vertex(int) { throw std::runtime_error{"remove failed"}; }
};

struct ref_qualified_graph
{
  using vertex_type = int;
  using edge_type = int;
  static constexpr bool is_directed = true;
  static constexpr bool is_multigraph = false;
  constexpr int vertex_count() & noexcept { return 1; }
  constexpr int vertex_count() && noexcept { return 2; }
  constexpr int vertex_count() const & noexcept { return 3; }
  constexpr int vertex_count() const && noexcept { return 4; }
};
}

#endif
