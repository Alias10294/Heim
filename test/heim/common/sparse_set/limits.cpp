#include "support.hpp"

namespace hs = heim_test::sparse_set;

TEST_CASE("sparse_set uint8 keys handle the largest key", "[common][sparse_set][limits]")
{
  hs::set<std::uint8_t, 8> s;
  for (unsigned k : {0u, 1u, 127u, 254u, 255u})
    REQUIRE(s.insert(static_cast<std::uint8_t>(k)).second);
  hs::verify(s, {std::uint8_t{0}, std::uint8_t{1}, std::uint8_t{127},
                 std::uint8_t{254}, std::uint8_t{255}});
  REQUIRE(s.try_erase(std::uint8_t{255}));
  REQUIRE(s.insert(std::uint8_t{255}).second);
}

TEST_CASE("sparse_set uint8 contiguous backend accepts high keys", "[common][sparse_set][limits]")
{
  hs::set<std::uint8_t, 0> s;
  for (unsigned k : {0u, 129u, 250u, 255u})
    s.insert(static_cast<std::uint8_t>(k));
  hs::verify(s, {std::uint8_t{0}, std::uint8_t{129},
                 std::uint8_t{250}, std::uint8_t{255}});
}

TEST_CASE("sparse_set uint8 does not exceed the representable index limit", "[common][sparse_set][limits]")
{
  hs::set<std::uint8_t, 8> s;
  auto capacity = s.max_size();
  REQUIRE(capacity <= 255u);
  for (std::size_t k = 0; k < capacity; ++k)
    REQUIRE(s.insert(static_cast<std::uint8_t>(k)).second);
  REQUIRE(s.size() == capacity);
  REQUIRE_THROWS_AS(s.insert(std::uint8_t{255}), std::length_error);
  REQUIRE_FALSE(s.contains(std::uint8_t{255}));
  REQUIRE(s.try_erase(std::uint8_t{0}));
  REQUIRE(s.insert(std::uint8_t{255}).second);
  REQUIRE(s.size() == capacity);
}

TEST_CASE("sparse_set uint16 can represent distant keys with paged storage", "[common][sparse_set][limits]")
{
  hs::set<std::uint16_t, 64> s;
  for (auto k : {std::uint16_t{0}, std::uint16_t{63}, std::uint16_t{64},
                 std::uint16_t{65534}, std::uint16_t{65535}})
    REQUIRE(s.insert(k).second);
  REQUIRE(s.size() == 5);
  REQUIRE(s.contains(std::uint16_t{65535}));
  REQUIRE(s.try_erase(std::uint16_t{65535}));
}

TEST_CASE("sparse_set page size one allocates all disjoint key pages correctly", "[common][sparse_set][limits]")
{
  hs::set<unsigned, 1> s;
  for (auto k : {0u, 1u, 7u, 31u, 127u, 255u}) s.insert(k);
  hs::verify(s, {0u, 1u, 7u, 31u, 127u, 255u});
}

TEST_CASE("sparse_set non-power-of-two page size handles boundaries", "[common][sparse_set][limits]")
{
  hs::set<unsigned, 7> s;
  for (auto k : {6u, 7u, 13u, 14u, 20u, 21u, 27u, 28u}) s.insert(k);
  hs::verify(s, {6u, 7u, 13u, 14u, 20u, 21u, 27u, 28u});
  s.try_erase(14u); s.try_erase(28u);
  hs::verify(s, {6u, 7u, 13u, 20u, 21u, 27u});
}

TEST_CASE("sparse_set size never exceeds max_size", "[common][sparse_set][limits]")
{
  hs::set<std::uint8_t, 0> s;
  for (unsigned k = 0; k < 150; ++k) s.insert(static_cast<std::uint8_t>(k));
  REQUIRE(s.size() == 150);
  REQUIRE(s.size() <= s.max_size());
}
