#include "support.hpp"

using namespace heim_test::sparse_map;

TEST_CASE("sparse_map try_erase missing keys does not mutate", "[common][sparse_map][erasure]")
{
  map<> m{{1u, 10}, {9u, 90}};
  for (unsigned k:{0u, 2u, 8u, 10u, 1000u})
    REQUIRE_FALSE(m.try_erase(k));
  verify(m, {{1u, 10}, {9u, 90}});
}

TEST_CASE("sparse_map erase existing keys preserves values", "[common][sparse_map][erasure]")
{
  map<> m{{2u, 20}, {3u, 30}, {7u, 70}, {11u, 110}, {17u, 170}};
  m.erase(7u);
  verify(m, {{2u, 20}, {3u, 30}, {11u, 110}, {17u, 170}});
  m.erase(2u);
  m.erase(17u);
  verify(m, {{3u, 30}, {11u, 110}});
}

TEST_CASE("sparse_map erase last dense member", "[common][sparse_map][erasure]")
{
  map<> m{{1u, 10}, {2u, 20}, {3u, 30}};
  REQUIRE(m.try_erase(3u));
  verify(m, {{1u, 10}, {2u, 20}});
}

TEST_CASE("sparse_map erase first dense member via swap-and-pop", "[common][sparse_map][erasure]")
{
  map<> m{{1u, 10}, {2u, 20}, {3u, 30}};
  REQUIRE(m.try_erase(1u));
  verify(m, {{2u, 20}, {3u, 30}});
}

TEST_CASE("sparse_map erase only element", "[common][sparse_map][erasure]")
{
  map<> m{{5u, 50}};
  REQUIRE(m.try_erase(5u));
  verify(m, {});
  REQUIRE_FALSE(m.try_erase(5u));
  REQUIRE(m.try_emplace(5u, 51).second);
  verify(m, {{5u, 51}});
}

TEST_CASE("sparse_map erase(begin) returns replacement iterator", "[common][sparse_map][erasure]")
{
  map<> m{{1u, 10}, {2u, 20}, {3u, 30}};
  REQUIRE(m.begin()->first == 3);
  auto it = m.erase(m.begin());
  REQUIRE(it == m.begin());
  verify(m, {{1u, 10}, {2u, 20}});
}

TEST_CASE("sparse_map erase one iterator from the middle", "[common][sparse_map][erasure]")
{
  map<> m{{1u, 10}, {2u, 20}, {3u, 30}, {4u, 40}, {5u, 50}};
  auto it = m.begin()+2;
  unsigned removed = it->first;
  REQUIRE(removed == 3u);
  auto after = m.erase(it);
  REQUIRE(after == m.begin()+2);
  REQUIRE_FALSE(m.contains(removed));
  verify(m, {{1u, 10}, {2u, 20}, {4u, 40}, {5u, 50}});
}

TEST_CASE("sparse_map erase last iterator returns end", "[common][sparse_map][erasure]")
{
  map<> m{{1u, 10}, {2u, 20}, {3u, 30}};
  auto after = m.erase(m.end()-1);
  REQUIRE(after == m.end());
  verify(m, {{2u, 20}, {3u, 30}});
}

TEST_CASE("sparse_map erase empty iterator interval", "[common][sparse_map][erasure]")
{
  map<> m{{1u, 10}, {2u, 20}, {3u, 30}};
  auto it = m.begin()+1;
  auto after = m.erase(it, it);
  REQUIRE(after == it);
  verify(m, {{1u, 10}, {2u, 20}, {3u, 30}});
}

TEST_CASE("sparse_map erase suffix of forward traversal", "[common][sparse_map][erasure]")
{
  map<> m{{1u, 10}, {2u, 20}, {3u, 30}, {4u, 40}, {5u, 50}};
  auto result = m.erase(m.begin(), m.begin()+2);
  REQUIRE(result == m.begin());
  verify(m, {{1u, 10}, {2u, 20}, {3u, 30}});
}

TEST_CASE("sparse_map erase entire iterator range", "[common][sparse_map][erasure]")
{
  map<> m{{1u, 10}, {2u, 20}, {3u, 30}, {4u, 40}};
  auto after = m.erase(m.begin(), m.end());
  REQUIRE(after == m.end());
  verify(m, {});
}

TEST_CASE("sparse_map repeated alternating front/back removal", "[common][sparse_map][erasure]")
{
  map<> m;
  std::unordered_map<unsigned, int> oracle;
  for (unsigned k = 0; k < 96; ++k)
  {
    REQUIRE(m.try_emplace(k * 7u, int(k)).second);
    oracle.emplace(k * 7u, int(k));
  }
  for (unsigned i = 0; i < 96; ++i)
  {
    auto key = (i % 2 == 0 ? i/2 : 95u - i/2)*7u;
    REQUIRE(m.try_erase(key));
    REQUIRE(oracle.erase(key) == 1);
    if (i % 11 == 0)
      verify(m, oracle);
  }
  verify(m, {});
}
