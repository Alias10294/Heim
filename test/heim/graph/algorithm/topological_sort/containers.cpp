#include "../derived/reference.hpp"

using namespace heim_test::graph_algorithm::derived;

TEST_CASE("Topological sort returns an owning view for a temporary vector", "[graph][topo][container]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 4);
  add_edges(g, {{0, 1}, {1, 2}, {2, 3}});
  auto order = runtime_topological_sort(g, std::vector<vertex<G>>{});
  REQUIRE((ids<G>(order) == std::vector<unsigned>{0, 1, 2, 3}));
}

TEST_CASE("Topological sort writes into an lvalue vector", "[graph][topo][container]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 4);
  add_edges(g, {{0, 1}, {1, 2}, {2, 3}});
  std::vector<vertex<G>> container;
  auto order = runtime_topological_sort(g, container);
  REQUIRE((ids<G>(order) == std::vector<unsigned>{0, 1, 2, 3}));
  REQUIRE(ids<G>(container) == ids<G>(order));
}

TEST_CASE("Topological sort supports deque via push_back", "[graph][topo][container]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 4);
  add_edges(g, {{0, 1}, {1, 2}, {2, 3}});
  std::deque<vertex<G>> container;
  auto order = runtime_topological_sort(g, container);
  REQUIRE((ids<G>(order) == std::vector<unsigned>{0, 1, 2, 3}));
}

TEST_CASE("Topological sort supports list via push_front", "[graph][topo][container]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 4);
  add_edges(g, {{0, 1}, {1, 2}, {2, 3}});
  std::list<vertex<G>> container;
  auto order = runtime_topological_sort(g, container);
  REQUIRE((ids<G>(order) == std::vector<unsigned>{0, 1, 2, 3}));
}

TEST_CASE("Topological sort supports forward_list via push_front", "[graph][topo][container]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 4);
  add_edges(g, {{0, 1}, {1, 2}, {2, 3}});
  std::forward_list<vertex<G>> container;
  auto order = runtime_topological_sort(g, container);
  REQUIRE((ids<G>(order) == std::vector<unsigned>{0, 1, 2, 3}));
}

TEST_CASE("Topological sort supports moving an owned list", "[graph][topo][container]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 3);
  add_edges(g, {{0, 1}, {1, 2}});
  auto result = runtime_topological_sort(g, std::list<vertex<G>>{});
  REQUIRE((ids<G>(result) == std::vector<unsigned>{0, 1, 2}));
}

TEST_CASE("Topological sort preserves a returned view after graph scope ends", "[graph][topo][container]")
{
  using G = graph<>;
  auto result = []
  {
    G g;
    make_vertices(g, 3);
    add_edges(g, {{0, 1}, {1, 2}});
    return runtime_topological_sort(g);
  }();
  REQUIRE((ids<G>(result) == std::vector<unsigned>{0, 1, 2}));
}

TEST_CASE("Topological sort lvalue view refers to the provided container", "[graph][topo][container]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 2);
  add_edges(g, {{0, 1}});
  std::vector<vertex<G>> container;
  auto order = runtime_topological_sort(g, container);
  REQUIRE(std::addressof(*order.begin()) == std::addressof(container.front()));
}
