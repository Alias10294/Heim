#include <heim/graph/algorithm/dijkstra/search.hpp>
#include "reference.hpp"

using namespace heim_test::graph_algorithm::dijkstra;

TEST_CASE("Dijkstra search header can be included independently", "[graph][dijkstra][include]")
{
  using G = graph<>;
  weighted_graph<G> f{2};
  f.add(0, 1, 3);
  REQUIRE(f.g.vertex_count() == 2);
}
