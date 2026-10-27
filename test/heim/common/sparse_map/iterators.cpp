#include "support.hpp"
#include <concepts>
using namespace heim_test::sparse_map;

TEST_CASE("sparse_map forward iteration is reverse dense storage", "[common][sparse_map][iterator]")
{
  map<> m(heim::unique,std::vector<unsigned>{2,9,4,7},std::vector<int>{20,90,40,70});
  std::vector<std::pair<unsigned,int>> observed;
  for(auto [key,value]:m) observed.emplace_back(key,value);
  REQUIRE(observed==(std::vector<std::pair<unsigned,int>>{{7,70},{4,40},{9,90},{2,20}}));
}

TEST_CASE("sparse_map reverse iteration preserves dense storage order", "[common][sparse_map][iterator]")
{
  map<> m(heim::unique,std::vector<unsigned>{2,9,4,7},std::vector<int>{20,90,40,70});
  std::vector<std::pair<unsigned,int>> observed;
  for(auto it=m.rbegin();it!=m.rend();++it) observed.emplace_back(it->first,it->second);
  REQUIRE(observed==(std::vector<std::pair<unsigned,int>>{{2,20},{9,90},{4,40},{7,70}}));
}

TEST_CASE("sparse_map const begin and cbegin traverse identically", "[common][sparse_map][iterator]")
{
  map<> m{{1u,10},{9u,90},{15u,150}};
  auto const &cm=m;
  auto a=cm.begin(); auto b=cm.cbegin();
  while(a!=cm.end()) { REQUIRE(a->first==b->first); REQUIRE(a->second==b->second); ++a; ++b; }
  REQUIRE(b==cm.cend());
}

TEST_CASE("sparse_map mutable iterator converts to const iterator", "[common][sparse_map][iterator]")
{
  map<> m{{1u,10},{9u,90},{15u,150}};
  map<>::const_iterator ci=m.begin();
  REQUIRE(ci->first==m.begin()->first);
  REQUIRE(ci->second==m.begin()->second);
}

TEST_CASE("sparse_map iterator random-access arithmetic", "[common][sparse_map][iterator]")
{
  map<> m(heim::unique,std::vector<unsigned>{1,2,3,4,5,6},std::vector<int>{10,20,30,40,50,60});
  auto first=m.begin();
  REQUIRE(m.end()-m.begin()==6);
  REQUIRE((first+2)->first==4);
  REQUIRE((2+first)->second==40);
  REQUIRE((m.end()-1)->first==1);
  REQUIRE(first[3].second==30);
  auto copy=first;
  REQUIRE((++copy)->first==5);
  REQUIRE((copy++)->first==5);
  REQUIRE(copy->first==4);
  REQUIRE((--copy)->first==5);
  REQUIRE((copy--)->first==5);
  REQUIRE(copy==first);
  copy+=3; REQUIRE(copy==first+3);
  copy-=2; REQUIRE(copy==first+1);
  REQUIRE(first<copy);
  REQUIRE(copy>first);
  REQUIRE(first<=copy);
  REQUIRE(copy>=first);
}

TEST_CASE("sparse_map iterator dereference is a writable reference pair", "[common][sparse_map][iterator]")
{
  map<> m{{2u,20},{4u,40}};
  for(auto [key,value]:m) value+=int(key);
  verify(m,{{2u,22},{4u,44}});
}

TEST_CASE("sparse_map iterator arrow accesses live mapped value", "[common][sparse_map][iterator]")
{
  map<> m{{1u,10},{3u,30},{5u,50}};
  for(auto it=m.begin();it!=m.end();++it) it->second+=1;
  verify(m,{{1u,11},{3u,31},{5u,51}});
}

TEST_CASE("sparse_map iterators traverse exactly size elements", "[common][sparse_map][iterator]")
{
  map<> m;
  for(unsigned i=0;i<120;++i) REQUIRE(m.try_emplace(i*13u,int(i)).second);
  REQUIRE(std::ranges::distance(m.begin(),m.end())==120);
  REQUIRE(std::ranges::distance(m.cbegin(),m.cend())==120);
  REQUIRE(std::ranges::distance(m.rbegin(),m.rend())==120);
  REQUIRE(std::ranges::distance(m.crbegin(),m.crend())==120);
}

TEST_CASE("sparse_map mutable/const reverse iterators agree", "[common][sparse_map][iterator]")
{
  map<> m{{1u,10},{3u,30},{5u,50}};
  auto const &cm=m;
  auto a=m.rbegin(); auto b=cm.crbegin();
  for(;a!=m.rend();++a,++b) { REQUIRE(a->first==b->first); REQUIRE(a->second==b->second); }
  REQUIRE(b==cm.crend());
}

TEST_CASE("sparse_map find returns iterator addressing the exact value", "[common][sparse_map][iterator]")
{
  map<> m;
  for(unsigned i=0;i<80;++i) m.try_emplace(i*19u,int(i*7));
  for(unsigned i=0;i<80;++i)
  {
    auto it=m.find(i*19u);
    REQUIRE(it!=m.end());
    REQUIRE(it->first==i*19u);
    REQUIRE(it->second==int(i*7));
    REQUIRE(std::addressof(it->second)==std::addressof(m.at(i*19u)));
  }
}
