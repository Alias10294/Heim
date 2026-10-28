#include <heim/graph/algorithm/component/connected.hpp>
#include "../derived/support.hpp"

using namespace heim_test::graph_algorithm::derived;

TEST_CASE("Connected components header can be included independently", "[graph][cc][include]")
{
  graph<false> g;
  auto result = hg::connected_components(g);
  REQUIRE(result.component_count() == 0);
}
