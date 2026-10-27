#include "support.hpp"
using namespace heim_test::sparse_map;

TEST_CASE("sparse_map copy constructor creates independent keys and values", "[common][sparse_map][lifecycle]")
{
  map<> src{{2u,20},{5u,50},{100u,1000}};
  map<> dst(src);
  verify(dst,{{2u,20},{5u,50},{100u,1000}});
  dst.at(5u)=88;
  REQUIRE(dst.try_erase(100u));
  verify(src,{{2u,20},{5u,50},{100u,1000}});
}

TEST_CASE("sparse_map copy assignment replaces prior membership", "[common][sparse_map][lifecycle]")
{
  map<> src{{2u,20},{5u,50}};
  map<> dst{{6u,60},{7u,70}};
  dst=src;
  verify(dst,{{2u,20},{5u,50}});
  verify(src,{{2u,20},{5u,50}});
  dst.at(2u)=22;
  REQUIRE(src.at(2u)==20);
}

TEST_CASE("sparse_map self-copy preserves content", "[common][sparse_map][lifecycle]")
{
  map<> m{{0u,1},{200u,2},{8u,3}};
  auto const &alias=m;
  m=alias;
  verify(m,{{0u,1},{200u,2},{8u,3}});
}

TEST_CASE("sparse_map move constructor transfers key/value associations", "[common][sparse_map][lifecycle]")
{
  map<> from{{1u,10},{9u,90},{17u,170}};
  map<> to(std::move(from));
  verify(to,{{1u,10},{9u,90},{17u,170}});
}

TEST_CASE("sparse_map move assignment replaces previous content", "[common][sparse_map][lifecycle]")
{
  map<> from{{1u,10},{9u,90}};
  map<> to{{3u,30},{5u,50}};
  to=std::move(from);
  verify(to,{{1u,10},{9u,90}});
}

TEST_CASE("sparse_map self-move assignment retains a valid index", "[common][sparse_map][lifecycle]")
{
  map<> m{{0u,11},{1u,22},{7u,77},{255u,99}};
  m=std::move(m);
  // Self-move need not preserve values, but must retain invariants.
  auto expected=snapshot(m);
  verify(m,expected);
  REQUIRE(m.try_emplace(100u,123).second);
  REQUIRE(m.at(100u)==123);
  REQUIRE(m.try_erase(100u));
  verify(m,expected);
}

TEST_CASE("sparse_map member and ADL swap preserve all associations", "[common][sparse_map][lifecycle]")
{
  map<> a{{1u,10},{3u,30}};
  map<> b{{2u,20},{4u,40},{6u,60}};
  a.swap(b);
  verify(a,{{2u,20},{4u,40},{6u,60}});
  verify(b,{{1u,10},{3u,30}});
  swap(a,b);
  verify(a,{{1u,10},{3u,30}});
  verify(b,{{2u,20},{4u,40},{6u,60}});
}

TEST_CASE("sparse_map equality ignores iteration order", "[common][sparse_map][lifecycle]")
{
  map<> a{{1u,10},{2u,20},{3u,30}};
  map<> b{{3u,30},{1u,10},{2u,20}};
  REQUIRE(a==b); REQUIRE(b==a);
  b.insert_or_assign(1u,11);
  REQUIRE_FALSE(a==b);
  b.insert_or_assign(1u,10);
  REQUIRE(a==b);
  REQUIRE_FALSE(a==map<>{{1u,10},{2u,20}});
}

TEST_CASE("sparse_map initializer_list assignment replaces all entries", "[common][sparse_map][lifecycle]")
{
  map<> m{{1u,10},{2u,20}};
  m={{3u,30},{4u,40},{3u,300}};
  verify(m,{{3u,30},{4u,40}});
}

TEST_CASE("sparse_map replace deduplicates keys while retaining first value", "[common][sparse_map][lifecycle]")
{
  map<> m{{100u,1}};
  m.replace(std::vector<unsigned>{8,7,8,6},std::vector<int>{80,70,800,60});
  verify(m,{{8u,80},{7u,70},{6u,60}});
}

TEST_CASE("sparse_map replace unique accepts prevalidated distinct keys", "[common][sparse_map][lifecycle]")
{
  map<> m{{100u,1}};
  m.replace(heim::unique,std::vector<unsigned>{9,1,0},std::vector<int>{90,10,0});
  verify(m,{{9u,90},{1u,10},{0u,0}});
}

TEST_CASE("sparse_map extract returns aligned dense key and value sequences", "[common][sparse_map][lifecycle]")
{
  map<> m(heim::unique,std::vector<unsigned>{7,1,5},std::vector<int>{70,10,50});
  auto cont=std::move(m).extract();
  REQUIRE(cont.keys==(std::vector<unsigned>{7,1,5}));
  REQUIRE(cont.values==(std::vector<int>{70,10,50}));
}

TEST_CASE("sparse_map replaces empty and nonempty containers repeatedly", "[common][sparse_map][lifecycle]")
{
  map<> m{{7u,9}};
  m.replace(std::vector<unsigned>{},std::vector<int>{});
  verify(m,{});
  m.replace(std::vector<unsigned>{2,4},std::vector<int>{20,40});
  verify(m,{{2u,20},{4u,40}});
  m.replace(std::vector<unsigned>{},std::vector<int>{});
  verify(m,{});
}
