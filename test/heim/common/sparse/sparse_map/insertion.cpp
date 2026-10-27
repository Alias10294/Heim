#include "support.hpp"

using namespace heim_test::sparse_map;

TEST_CASE("sparse_map try_emplace reports creation and duplicates", "[common][sparse_map][insertion]")
{
  map<> m;
  auto [i, ok] = m.try_emplace(7u, 70);
  REQUIRE(ok);
  REQUIRE(i->first == 7);
  REQUIRE(i->second == 70);
  auto [j, created] = m.try_emplace(7u, -1);
  REQUIRE_FALSE(created);
  REQUIRE(j->second == 70);
  verify(m, {{7u, 70}});
}

TEST_CASE("sparse_map emplace inserts new distinct keys", "[common][sparse_map][insertion]")
{
  map<> m;
  REQUIRE(m.emplace(1u, 10).second);
  REQUIRE(m.emplace(9u, 90).second);
  REQUIRE(m.emplace_hint(m.cbegin(), 17u, 170)->second == 170);
  verify(m, {{1u, 10}, {9u, 90}, {17u, 170}});
}

TEST_CASE("sparse_map try_emplace_hint ignores hint for duplicate", "[common][sparse_map][insertion]")
{
  map<> m{{1u, 100}, {2u, 200}};
  auto it = m.try_emplace_hint(m.cend(), 1u, 999);
  REQUIRE(it->second == 100);
  REQUIRE(m.try_emplace_hint(m.cbegin(), 3u, 300)->first == 3);
  verify(m, {{1u, 100}, {2u, 200}, {3u, 300}});
}

TEST_CASE("sparse_map insert const pair retains old value", "[common][sparse_map][insertion]")
{
  map<> m;
  map<>::value_type const x{3u, 5};
  REQUIRE(m.insert(x).second);
  map<>::value_type const conflict{3u, 6};
  REQUIRE_FALSE(m.insert(conflict).second);
  verify(m, {{3u, 5}});
}

TEST_CASE("sparse_map insert rvalue pair retains old value", "[common][sparse_map][insertion]")
{
  map<> m;
  REQUIRE(m.insert(map<>::value_type{8u, 42}).second);
  REQUIRE_FALSE(m.insert(map<>::value_type{8u, 43}).second);
  verify(m, {{8u, 42}});
}

TEST_CASE("sparse_map insert conversion from pair-like object", "[common][sparse_map][insertion]")
{
  map<> m;
  std::pair<unsigned, short> pair{2u, 9};
  REQUIRE(m.insert(pair).second);
  REQUIRE_FALSE(m.insert(pair).second);
  verify(m, {{2u, 9}});
}

TEST_CASE("sparse_map hinted insert const and rvalue pair", "[common][sparse_map][insertion]")
{
  map<> m;
  map<>::value_type const a{5u, 50};
  REQUIRE(m.insert(m.cend(), a)->first == 5);
  REQUIRE(m.insert(m.cbegin(), map<>::value_type{6u, 60})->second == 60);
  REQUIRE(m.insert(m.cend(), map<>::value_type{5u, 77})->second == 50);
  verify(m, {{5u, 50}, {6u, 60}});
}

TEST_CASE("sparse_map hinted insert pair-like conversion", "[common][sparse_map][insertion]")
{
  map<> m;
  std::pair<unsigned, short> pair{10u, 77};
  REQUIRE(m.insert(m.cend(), pair)->first == 10);
  verify(m, {{10u, 77}});
}

TEST_CASE("sparse_map insert range via iterators and ranges", "[common][sparse_map][insertion]")
{
  map<> m;
  std::vector<std::pair<unsigned, int>> a{{9, 1}, {8, 2}, {9, 3}};
  m.insert(a.begin(), a.end());
  m.insert_range(std::vector<std::pair<unsigned, int>>{{7, 4}, {8, 100}});
  verify(m, {{9u, 1}, {8u, 2}, {7u, 4}});
}

TEST_CASE("sparse_map insert initializer_list respects duplicates", "[common][sparse_map][insertion]")
{
  map<> m{{1u, 1}};
  m.insert({{1u, 7}, {3u, 3}, {3u, 100}, {2u, 2}});
  verify(m, {{1u, 1}, {3u, 3}, {2u, 2}});
}

TEST_CASE("sparse_map insert_or_assign reports insert and replacement", "[common][sparse_map][insertion]")
{
  map<> m;
  REQUIRE(m.insert_or_assign(11u, 1).second);
  auto [it, created] = m.insert_or_assign(11u, 5);
  REQUIRE_FALSE(created);
  REQUIRE(it->second == 5);
  REQUIRE(m.insert_or_assign(17u, 9).second);
  verify(m, {{11u, 5}, {17u, 9}});
}

TEST_CASE("sparse_map hinted insert_or_assign handles both branches", "[common][sparse_map][insertion]")
{
  map<> m{{10u, 2}};
  REQUIRE(m.insert_or_assign(m.cend(), 10u, 3)->second == 3);
  REQUIRE(m.insert_or_assign(m.cbegin(), 20u, 5)->second == 5);
  verify(m, {{10u, 3}, {20u, 5}});
}

TEST_CASE("sparse_map insertion after erase reuses sparse index", "[common][sparse_map][insertion]")
{
  map<> m{{1u, 1}, {9u, 9}, {17u, 17}};
  REQUIRE(m.try_erase(9u));
  REQUIRE(m.try_emplace(9u, 99).second);
  verify(m, {{1u, 1}, {9u, 99}, {17u, 17}});
}

TEST_CASE(
    "sparse_map duplicate try_emplace does not evaluate mapped constructor",
    "[common][sparse_map][insertion]")
{
  struct value
  {
    int x;
    explicit value(int v) : x(v)
    {
      if (v < 0)
        throw std::runtime_error("constructed");
    }
    bool operator== (value const&) const = default;
  };
  map<unsigned, value> m;
  REQUIRE(m.try_emplace(1u, 2).second);
  REQUIRE_NOTHROW(m.try_emplace(1u, -1));
  REQUIRE(m.at(1u).x == 2);
}
