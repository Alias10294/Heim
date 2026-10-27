#include <catch2/catch_test_macros.hpp>
#include <heim/common/sparse.hpp>
#include <algorithm>
#include <cstdint>
#include <random>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

namespace {
using map_type = heim::sparse_map<unsigned, int>;

void check_against(map_type const& actual, std::unordered_map<unsigned, int> const& expected)
{
  REQUIRE(actual.size() == expected.size());
  for (auto const& [key, value] : expected) {
    INFO("key=" << key);
    REQUIRE(actual.contains(key));
    REQUIRE(actual.at(key) == value);
    auto it = actual.find(key);
    REQUIRE(it != actual.end());
    REQUIRE(it->first == key);
    REQUIRE(it->second == value);
  }
  std::unordered_map<unsigned, int> iterated;
  for (auto [key, value] : actual) {
    REQUIRE(iterated.emplace(key, value).second);
    REQUIRE(expected.at(key) == value);
  }
  REQUIRE(iterated == expected);
}
}

TEST_CASE("sparse_map supports sparse keys and checked access", "[common][sparse_map]")
{
  map_type m;
  REQUIRE(m.empty());
  REQUIRE_FALSE(m.contains(7));
  REQUIRE(m.find(7) == m.end());
  REQUIRE_THROWS_AS(m.at(7), std::out_of_range);
  REQUIRE(m.try_emplace(0u, 12).second);
  REQUIRE(m.try_emplace(2048u, 99).second);
  REQUIRE(m.at(2048) == 99);
  REQUIRE_FALSE(m.try_emplace(0u, -1).second);
  check_against(m, {{0u, 12}, {2048u, 99}});
}

TEST_CASE("sparse_map insertion does not overwrite and insert_or_assign does", "[common][sparse_map]")
{
  map_type m;
  REQUIRE(m.insert({5u, 10}).second);
  REQUIRE_FALSE(m.insert({5u, 20}).second);
  REQUIRE(m.at(5u) == 10);
  REQUIRE_FALSE(m.insert_or_assign(5u, 30).second);
  REQUIRE(m.at(5u) == 30);
  REQUIRE(m.insert_or_assign(7u, 70).second);
  check_against(m, {{5u, 30}, {7u, 70}});
}

TEST_CASE("sparse_map swap-and-pop erasure keeps values with their keys", "[common][sparse_map]")
{
  map_type m;
  for (unsigned i = 0; i < 20; ++i) m.try_emplace(i * 31u, static_cast<int>(i * 9));
  REQUIRE(m.try_erase(6u * 31u));
  REQUIRE(m.try_erase(0u));
  REQUIRE(m.try_erase(19u * 31u));
  REQUIRE_FALSE(m.try_erase(0u));
  std::unordered_map<unsigned, int> expected;
  for (unsigned i = 1; i < 19; ++i)
    if (i != 6) expected.emplace(i * 31u, static_cast<int>(i * 9));
  check_against(m, expected);
  REQUIRE(m.try_emplace(0u, -4).second);
  REQUIRE(m.at(0) == -4);
}

TEST_CASE("sparse_map copy and swap do not alias values", "[common][sparse_map]")
{
  map_type original;
  original.try_emplace(2u, 40);
  original.try_emplace(8u, 70);
  auto copy = original;
  copy.at(8) = -1;
  REQUIRE(original.at(8) == 70);
  map_type alternate;
  alternate.try_emplace(1u, 19);
  swap(copy, alternate);
  check_against(copy, {{1u, 19}});
  check_against(alternate, {{2u, 40}, {8u, -1}});
  alternate.clear();
  REQUIRE(alternate.empty());
  REQUIRE(alternate.try_emplace(8u, 11).second);
}

TEST_CASE("sparse_map preserves key/value alignment through replacement", "[common][sparse_map]")
{
  map_type m;
  m.replace(std::vector<unsigned>{3u, 7u, 3u, 9u}, std::vector<int>{30, 70, 300, 90});
  check_against(m, {{3u, 30}, {7u, 70}, {9u, 90}});
  m.insert_or_assign(7u, 71);
  REQUIRE(m.at(7u) == 71);
}

TEST_CASE("sparse_map deterministic model-based mutation regression", "[common][sparse_map][model]")
{
  map_type m;
  std::unordered_map<unsigned, int> oracle;
  std::mt19937 rng{0xA3B21u};
  for (unsigned step = 0; step < 3000; ++step) {
    const unsigned key = rng() % 700;
    const int value = static_cast<int>(rng() % 10000);
    const unsigned op = rng() % 4;
    if (op == 0) {
      REQUIRE(m.try_erase(key) == (oracle.erase(key) != 0));
    } else if (op == 1) {
      REQUIRE(m.try_emplace(key, value).second == oracle.emplace(key, value).second);
    } else {
      bool inserted = oracle.insert_or_assign(key, value).second;
      REQUIRE(m.insert_or_assign(key, value).second == inserted);
    }
    if (step % 89 == 0) check_against(m, oracle);
  }
  check_against(m, oracle);
}
