#include "support.hpp"
#include <compare>
#include <functional>
#include <sstream>
#include <unordered_set>
using namespace heim_test::adjacency;

TEST_CASE("adjacency_list vertex descriptors compare by id", "[graph][adjacency_list][descriptor]")
{
  using V=graph<>::vertex_type;
  V a{1u},b{2u},copy{1u};
  REQUIRE(a==copy);
  REQUIRE(a!=b);
  REQUIRE(a<b);
  REQUIRE(b>a);
  REQUIRE(a.id()==1u);
  REQUIRE(static_cast<unsigned>(a)==1u);
}
TEST_CASE("adjacency_list edge descriptors preserve stable identity", "[graph][adjacency_list][descriptor]")
{
  graph<true> g;auto vs=make_vertices(g,3);
  auto [e,ok]=g.place_edge(vs[0],vs[1]);REQUIRE(ok);
  auto saved=e;
  g.place_edge(vs[2],vs[1]);
  REQUIRE(e==saved);
  REQUIRE(e.id()==saved.id());
  REQUIRE(g.source(e)==vs[0]);
}
TEST_CASE("adjacency_list vertex descriptors are hashable", "[graph][adjacency_list][descriptor]")
{
  using V=graph<>::vertex_type;
  std::unordered_set<V> ids;
  REQUIRE(ids.insert(V{3u}).second);
  REQUIRE_FALSE(ids.insert(V{3u}).second);
  REQUIRE(ids.contains(V{3u}));
}
TEST_CASE("adjacency_list edge descriptors are hashable", "[graph][adjacency_list][descriptor]")
{
  graph<> g;auto vs=make_vertices(g,2);
  auto [e,ok]=g.place_edge(vs[0],vs[1]);REQUIRE(ok);
  std::unordered_set<graph<>::edge_type> ids;
  REQUIRE(ids.insert(e).second);
  REQUIRE_FALSE(ids.insert(e).second);
  REQUIRE(ids.contains(e));
}
TEST_CASE("adjacency_list descriptors format and stream their identifiers", "[graph][adjacency_list][descriptor]")
{
  graph<> g;auto vs=make_vertices(g,2);
  auto [e,ok]=g.place_edge(vs[0],vs[1]);REQUIRE(ok);
  REQUIRE(std::format("{}",vs[1])=="1");
  REQUIRE(std::format("{}",e)=="0");
  std::ostringstream os;
  os<<vs[0]<<"/"<<e;
  REQUIRE(os.str()=="0/0");
}
TEST_CASE("adjacency_list descriptors survive removal of other vertices", "[graph][adjacency_list][descriptor]")
{
  graph<true> g;auto vs=make_vertices(g,4);
  auto [keep,ok]=g.place_edge(vs[2],vs[3]);REQUIRE(ok);
  g.place_edge(vs[0],vs[1]);
  g.remove_vertex(vs[0]);
  REQUIRE(g.source(keep)==vs[2]);
  REQUIRE(g.target(keep)==vs[3]);
  REQUIRE(g.edge_count()==1);
}
