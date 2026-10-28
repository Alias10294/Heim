#include <heim/graph/interface/implementation/adjacency_list.hpp>
#include <catch2/catch_test_macros.hpp>
#include <type_traits>
static_assert(std::is_same_v<heim::adjacency_list<>::id_type,unsigned>);
static_assert(sizeof(heim::adjacency_list<>)>0);
TEST_CASE("adjacency_list implementation header is independently includable", "[graph][adjacency_list][header]")
{
  heim::adjacency_list<> g;
  REQUIRE(g.vertex_count()==0);
}
