#include <catch2/catch_test_macros.hpp>
#include <heim/graph.hpp>

namespace hg = heim::graphs;
namespace {
struct bfs_observer {
  int initialized = 0;
  int sourced = 0;
  int discovered = 0;
  int examined = 0;
  int tree_edges = 0;
  int finished = 0;

  template<class G> void initialize_vertex(G const&, hg::vertex_t<G>) { ++initialized; }
  template<class G> void source_vertex(G const&, hg::vertex_t<G>) { ++sourced; }
  template<class G> void discover_vertex(G const&, hg::vertex_t<G>) { ++discovered; }
  template<class G> void examine_vertex(G const&, hg::vertex_t<G>) { ++examined; }
  template<class G> void tree_edge(G const&, hg::edge_t<G>) { ++tree_edges; }
  template<class G> void finish_vertex(G const&, hg::vertex_t<G>) { ++finished; }
};

struct dfs_observer {
  int initialized = 0;
  int started = 0;
  int discovered = 0;
  int finished = 0;
  int tree_edges = 0;
  template<class G> void initialize_vertex(G const&, hg::vertex_t<G>) { ++initialized; }
  template<class G> void start_vertex(G const&, hg::vertex_t<G>) { ++started; }
  template<class G> void discover_vertex(G const&, hg::vertex_t<G>) { ++discovered; }
  template<class G> void finish_vertex(G const&, hg::vertex_t<G>) { ++finished; }
  template<class G> void tree_edge(G const&, hg::edge_t<G>) { ++tree_edges; }
};

struct stop_bfs {
  int seen = 0;
  template<class G> hg::visit discover_vertex(G const&, hg::vertex_t<G>)
  { return ++seen == 2 ? hg::visit::stop : hg::visit::proceed; }
};

struct prune_bfs {
  int discovered = 0;
  int finished = 0;
  template<class G> void discover_vertex(G const&, hg::vertex_t<G>) { ++discovered; }
  template<class G> hg::visit examine_vertex(G const&, hg::vertex_t<G>) { return hg::visit::prune; }
  template<class G> void finish_vertex(G const&, hg::vertex_t<G>) { ++finished; }
};

struct stop_dfs {
  int seen = 0;
  template<class G> hg::visit discover_vertex(G const&, hg::vertex_t<G>)
  { return ++seen == 2 ? hg::visit::stop : hg::visit::proceed; }
};
}

TEST_CASE("breadth_first_visit invokes events for each reached vertex", "[graph][bfs][visit]")
{
  heim::adjacency_list<true> g{{0u, 1u}, {1u, 2u}, {0u, 3u}};
  bfs_observer obs;
  REQUIRE(hg::breadth_first_visit(g, 0u, obs));
  REQUIRE(obs.initialized == 4);
  REQUIRE(obs.sourced == 1);
  REQUIRE(obs.discovered == 4);
  REQUIRE(obs.examined == 4);
  REQUIRE(obs.tree_edges == 3);
  REQUIRE(obs.finished == 4);
}

TEST_CASE("breadth_first_visit supports explicit early stop", "[graph][bfs][visit]")
{
  heim::adjacency_list<true> g{{0u, 1u}, {1u, 2u}};
  stop_bfs vis;
  REQUIRE_FALSE(hg::breadth_first_visit(g, 0u, vis));
  REQUIRE(vis.seen == 2);
}

TEST_CASE("breadth_first_visit prune skips outgoing edges", "[graph][bfs][visit]")
{
  heim::adjacency_list<true> g{{0u, 1u}, {0u, 2u}};
  prune_bfs vis;
  REQUIRE(hg::breadth_first_visit(g, 0u, vis));
  REQUIRE(vis.discovered == 1);
  REQUIRE(vis.finished == 1);
}

TEST_CASE("depth_first_visit invokes start and finish events", "[graph][dfs][visit]")
{
  heim::adjacency_list<true> g{{0u, 1u}, {1u, 2u}, {0u, 3u}};
  dfs_observer obs;
  REQUIRE(hg::depth_first_visit(g, 0u, obs));
  REQUIRE(obs.initialized == 4);
  REQUIRE(obs.started == 1);
  REQUIRE(obs.discovered == 4);
  REQUIRE(obs.tree_edges == 3);
  REQUIRE(obs.finished == 4);
}

TEST_CASE("depth_first_visit supports explicit early stop", "[graph][dfs][visit]")
{
  heim::adjacency_list<true> g{{0u, 1u}, {1u, 2u}};
  stop_dfs vis;
  REQUIRE_FALSE(hg::depth_first_visit(g, 0u, vis));
  REQUIRE(vis.seen == 2);
}
