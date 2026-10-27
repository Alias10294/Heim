#include "support.hpp"
using namespace heim_test::sparse_map;

TEST_CASE("sparse_map default state and missing keys", "[common][sparse_map][basic]")
{
  map<> m;
  REQUIRE(m.empty());
  REQUIRE(m.size() == 0);
  REQUIRE(m.begin() == m.end());
  REQUIRE(m.rbegin() == m.rend());
  for (auto key : {0u, 1u, 7u, 8u, 65u, 1000000u})
  {
    REQUIRE_FALSE(m.contains(key));
    REQUIRE(m.find(key) == m.end());
    REQUIRE_FALSE(m.try_erase(key));
    REQUIRE_THROWS_AS(m.at(key), std::out_of_range);
  }
  verify(m, {});
}

TEST_CASE("sparse_map keys across distinct sparse pages", "[common][sparse_map][basic]")
{
  map<> m;
  std::unordered_map<unsigned,int> expected;
  for (auto [k,v] : {std::pair{0u, 7}, {1u, 8}, {7u, 9}, {8u, 10}, {9u, 11},
                     {15u, 12}, {16u, 13}, {255u, 14}, {4096u, 15}})
  {
    REQUIRE(m.try_emplace(k,v).second);
    expected.emplace(k,v);
  }
  verify(m, expected);
  for (auto [k,v] : expected) REQUIRE(m[k] == v);
}

TEST_CASE("sparse_map missing at throws for const and mutable objects", "[common][sparse_map][basic]")
{
  map<> m{{2u, 20}, {12u, 120}};
  auto const &cm = m;
  for (unsigned key : {0u,1u,3u,11u,13u,999u})
  {
    REQUIRE_THROWS_AS(m.at(key), std::out_of_range);
    REQUIRE_THROWS_AS(cm.at(key), std::out_of_range);
    REQUIRE(m.find(key) == m.end());
    REQUIRE(cm.find(key) == cm.end());
  }
  verify(m, {{2u,20},{12u,120}});
}

TEST_CASE("sparse_map mutable access does not modify other entries", "[common][sparse_map][basic]")
{
  map<> m{{0u,10},{9u,90},{17u,170}};
  m.at(9u) = -10;
  m[0u] += 12;
  m.find(17u)->second = 123;
  verify(m, {{0u,22},{9u,-10},{17u,123}});
}

TEST_CASE("sparse_map clear is idempotent and supports reuse", "[common][sparse_map][basic]")
{
  map<> m{{0u,3},{7u,4},{2000u,5}};
  m.clear(); m.clear();
  verify(m, {});
  REQUIRE(m.try_emplace(2000u, 8).second);
  REQUIRE(m.try_emplace(0u, 9).second);
  verify(m, {{0u,9},{2000u,8}});
}

TEST_CASE("sparse_map const lookup is independent of dense order", "[common][sparse_map][basic]")
{
  map<> m;
  for(unsigned i=0;i<80;++i) REQUIRE(m.try_emplace(79u-i, int(i*11)).second);
  auto const &cm=m;
  for(unsigned i=0;i<80;++i)
  {
    REQUIRE(cm.contains(79u-i));
    REQUIRE(cm.at(79u-i)==int(i*11));
  }
  REQUIRE(cm.size()==80);
}
