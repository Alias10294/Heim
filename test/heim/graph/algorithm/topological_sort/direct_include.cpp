#include <heim/graph/algorithm/topological_sort.hpp>
#include "../derived/support.hpp"

using namespace heim_test::graph_algorithm::derived;

TEST_CASE("Topological sort header can be included independently", "[graph][topo][include]")
{
  graph<> g;
  auto result = runtime_topological_sort(g);
  REQUIRE(std::ranges::empty(result));
}
