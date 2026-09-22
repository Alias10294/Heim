#include <cstddef>
#include <iostream>
#include <print>
#include <unordered_map>
#include "heim/heim.hpp"

static void test_001()
{
  using graph_type      = heim::adjacency_list<>;
  using vertex_type     = heim::graphs::vertex_t<graph_type>;
  using edge_type       = heim::graphs::edge_t  <graph_type>;
  using weight_map_type = std::unordered_map<edge_type, std::size_t>;

  graph_type      g{};
  weight_map_type w{};

  vertex_type const v0{heim::graphs::place_vertex(g)};
  vertex_type const v1{heim::graphs::place_vertex(g)};
  vertex_type const v2{heim::graphs::place_vertex(g)};
  vertex_type const v3{heim::graphs::place_vertex(g)};

  edge_type e0{heim::graphs::place_edge(g, v0, v1).first}; w[e0] = 1;
  edge_type e1{heim::graphs::place_edge(g, v1, v2).first}; w[e1] = 2;
  edge_type e2{heim::graphs::place_edge(g, v0, v2).first}; w[e2] = 10;
  edge_type e3{heim::graphs::place_edge(g, v2, v3).first}; w[e3] = 1;

  // breadth_first_search
  auto const bfs_res {heim::graphs::breadth_first_search(g, v0)};
  bool const bfs_ok  {bfs_res.reached(v3)};
  auto const bfs_path{bfs_res.path_from(v3)};

  std::println("breadth_first_search");
  std::println("  v3 reached ? {}", bfs_ok);

  std::print  ("  path from v3:");
  for (vertex_type v : bfs_path)
    std::print(" {}", v);
  std::println();

  // breadth_first_search_to
  auto const bfs_to_res {heim::graphs::breadth_first_search_to(g, v0, v3)};
  bool const bfs_to_ok  {bfs_to_res.reached()};
  auto const bfs_to_path{bfs_to_res.path()};

  std::println("breadth_first_search_to");
  std::println("  v3 reached ? {}", bfs_to_ok);

  std::print  ("  path from v3:");
  for (vertex_type v : bfs_to_path)
    std::print(" {}", v);
  std::println();

  // depth_first_search
  auto const dfs_res {heim::graphs::depth_first_search(g, v0)};
  bool const dfs_ok  {dfs_res.reached(v3)};
  auto const dfs_path{dfs_res.path_from(v3)};

  std::println("depth_first_search");
  std::println("  v3 reached ? {}", dfs_ok);

  std::print  ("  path from v3:");
  for (vertex_type v : dfs_path)
    std::print(" {}", v);
  std::println();

  // depth_first_search_to
  auto const dfs_to_res {heim::graphs::depth_first_search_to(g, v0, v3)};
  bool const dfs_to_ok  {dfs_to_res.reached()};
  auto const dfs_to_path{dfs_to_res.path()};

  std::println("depth_first_search_to");
  std::println("  v3 reached ? {}", dfs_to_ok);

  std::print  ("  path from v3:");
  for (vertex_type v : dfs_to_path)
    std::print(" {}", v);
  std::println();

  // dijkstra
  auto const dij_res {heim::graphs::dijkstra(g, v0, heim::make_readable_property_map(w))};
  bool const dij_ok  {dij_res.reached(v3)};
  auto const dij_path{dij_res.path_from(v3)};

  std::println("dijkstra");
  std::println("  v3 reached ? {}", dij_ok);

  std::print  ("  path from v3:");
  for (vertex_type v : dij_path)
    std::print(" {}", v);
  std::println();

  std::println("  distances:");
  for (vertex_type v : heim::graphs::vertices(g))
    std::println("    distance to v{} = {}", v, dij_res.distance_to(v));

  // dijkstra_to
  auto const dij_to_res {heim::graphs::dijkstra_to(g, v0, v3, heim::make_readable_property_map(w))};
  bool const dij_to_ok  {dij_to_res.reached()};
  auto const dij_to_path{dij_to_res.path()};

  std::println("dijkstra_to");
  std::println("  v3 reached ? {}", dij_to_ok);

  std::print  ("  path from v3:");
  for (vertex_type v : dij_to_path)
    std::print(" {}", v);
  std::println();
  std::println("  distance to v3 = {}", dij_to_res.distance());


  // connected_components
  auto const cc_res{heim::graphs::connected_components(g)};

  std::println("connected_components");
  std::println("  graph connected ? {}"    , cc_res.connected());
  std::println("  v0 and v3 connected ? {}", cc_res.connected(v0, v3));

  // topological_sort
  // auto const topo{heim::graphs::topological_sort(g)};
  //
  // std::println("topological_sort");
  //
  // std::print  ("  ordering:");
  // for (vertex_type v : topo)
  //   std::print(" {}", v);
  // std::println();
}

static void test_graph()
{
  // heim::adjacency_list g{};
  //
  // auto const v0{heim::graphs::place_vertex(g)};
  // auto const v1{heim::graphs::place_vertex(g)};
  // auto const v2{heim::graphs::place_vertex(g)};
  // auto const v3{heim::graphs::place_vertex(g)};
  //
  // auto const [e0, inserted]{heim::graphs::place_edge(g, v0, v1)};
  // auto const [e1, _       ]{heim::graphs::place_edge(g, v0, v2)};
  // auto const [e2, _       ]{heim::graphs::place_edge(g, v1, v2)};
  // auto const [e3, _       ]{heim::graphs::place_edge(g, v2, v3)};

  heim::adjacency_list g{{0, 1}, {0, 2}, {1, 2}, {2, 3}};

  auto const bfs{heim::graphs::breadth_first_search(g, 0)};

  for (auto v : bfs.path_from(3))
    std::cout << v << ' '; // 3 2 0
}

namespace { struct entity { int hp; }; }

static void test_property()
{
  std::unordered_map<int, int> map{{0, 100}, {1, 80}, {2, 60}};
  std::vector<entity>          vec{{100}, {80}, {60}};

  auto map_pm{heim::make_property_map(map)};
  auto vec_pm{heim::make_property_map(
      heim::composable_arg,
      heim::make_property_map(heim::member_arg, &entity::hp),
      heim::make_property_map(vec))};

  std::cout << heim::properties::get(map_pm, 1); // 80
  std::cout << heim::properties::get(vec_pm, 1); // 80

  heim::properties::set(map_pm, 1, 20);
  heim::properties::set(vec_pm, 1, 20);

  std::cout << heim::properties::get(map_pm, 1); // 20
  std::cout << heim::properties::get(vec_pm, 1); // 20
}


namespace
{
template<typename T, typename U>
struct compare : std::bool_constant<sizeof(T) < sizeof(U)>
{ };

template<typename T>
struct predicate : std::is_same<T, int>
{ };

}

using s0 = heim::type_sequence<long long, int, int, short, long, char>;
using s1 = s0::unique;                        // long long, int, short, long, char
using s2 = s0::transform<std::make_unsigned>; // unsigned long long, unsigned int, unsigned int, unsigned short, ...
using s3 = s0::filter   <predicate>;          // int, int
using s4 = s0::sort     <compare>;            // char, short, int, int, long, long long


int main()
{ test_001(); }