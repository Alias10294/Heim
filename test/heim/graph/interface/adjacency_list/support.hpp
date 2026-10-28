#ifndef HEIM_TEST_GRAPH_ADJACENCY_SUPPORT_HPP
#define HEIM_TEST_GRAPH_ADJACENCY_SUPPORT_HPP
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <format>
#include <iterator>
#include <memory>
#include <numeric>
#include <ranges>
#include <set>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <heim/graph.hpp>
#include "model.hpp"

namespace heim_test::adjacency
{
template<bool Directed=false,bool Multi=false,bool In=true,bool Out=true,
         typename UInt=unsigned,std::size_t Page=8,typename Alloc=std::allocator<UInt>>
using graph=heim::adjacency_list<Directed,Multi,In,Out,UInt,Page,Alloc>;

template<typename G>
auto make_vertices(G &g,unsigned n)
{
  std::vector<typename G::vertex_type> result;
  result.reserve(n);
  for(unsigned i=0;i<n;++i)result.push_back(g.place_vertex());
  return result;
}

template<typename G>
void verify_empty(G const &g)
{
  REQUIRE(g.vertex_count()==0);
  REQUIRE(g.edge_count()==0);
  REQUIRE(std::ranges::empty(g.vertices()));
  REQUIRE(std::ranges::empty(g.edges()));
}

// Graph equality may include internal order and insertion counters.
// This snapshot compares *only* documented logical observation, not representation.
template<typename G>
reference_graph snapshot(G const &g)
{
  reference_graph ref;
  for(auto v:g.vertices()) ref.vertices.insert(static_cast<unsigned>(v.id()));
  for(auto e:g.edges())
  {
    auto [u,v]=g.endpoints(e);
    ref.edges.emplace(static_cast<unsigned>(e.id()),reference_graph::edge{
      static_cast<unsigned>(u.id()),static_cast<unsigned>(v.id())});
  }
  return ref;
}

template<typename G>
void check_consistency(G const &g)
{
  // Used for objects made via constructors/copy/move where next ids are not observable.
  verify(g,snapshot(g));
}
} // namespace heim_test::adjacency
#endif
