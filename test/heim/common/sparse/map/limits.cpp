#include "support.hpp"

using namespace heim_test::sparse_map;

TEST_CASE("sparse_map 8-bit keys reach capacity and reject overflow", "[common][sparse_map][limit]")
{
  map<std::uint8_t, int, 8> m;
  for (unsigned i = 0; i < 255; ++i)
    REQUIRE(m.try_emplace(static_cast<std::uint8_t>(i), int(i)).second);
  REQUIRE(m.size() == 255);
  REQUIRE_THROWS_AS(m.try_emplace(std::uint8_t{255}, -1), std::length_error);
  REQUIRE_THROWS_AS(m.insert_or_assign(std::uint8_t{255}, -1), std::length_error);
  REQUIRE(m.size() == 255);
  for (unsigned i = 0; i < 255; ++i) REQUIRE(m.at(static_cast<std::uint8_t>(i))
    == int(i));
  REQUIRE(m.try_erase(std::uint8_t{100}));
  REQUIRE(m.try_emplace(std::uint8_t{255}, 999).second);
  REQUIRE(m.at(std::uint8_t{255}) == 999);
}

TEST_CASE("sparse_map 8-bit sentinel key can be stored", "[common][sparse_map][limit]")
{
  map<std::uint8_t, int, 8> m;
  REQUIRE(m.try_emplace(std::uint8_t{255}, 42).second);
  REQUIRE(m.at(std::uint8_t{255}) == 42);
  REQUIRE(m.try_erase(std::uint8_t{255}));
  REQUIRE_FALSE(m.contains(std::uint8_t{255}));
}

TEST_CASE("sparse_map 16-bit keys accept boundary values", "[common][sparse_map][limit]")
{
  map<std::uint16_t, int, 8> m;
  REQUIRE(m.try_emplace(std::uint16_t{0}, 1).second);
  REQUIRE(m.try_emplace(std::uint16_t{65535}, 2).second);
  REQUIRE(m.at(std::uint16_t{65535}) == 2);
  REQUIRE(m.try_erase(std::uint16_t{65535}));
}

TEST_CASE("sparse_map contiguous mode maintains map invariants", "[common][sparse_map][limit]")
{
  map<unsigned, int, 0> m;
  std::unordered_map<unsigned, int> expected;
  for (unsigned i = 0; i < 400; ++i)
  {
    m.try_emplace(i * 2u, int(i));
    expected.emplace(i * 2u, int(i));
  }
  for (unsigned i = 0; i < 400; i += 3)
  {
    REQUIRE(m.try_erase(i * 2u));
    expected.erase(i * 2u);
  }
  verify(m, expected);
}

TEST_CASE("sparse_map page size one handles arbitrary gaps", "[common][sparse_map][limit]")
{
  map<unsigned, int, 1> m;
  m.try_emplace(3u, 30);
  m.try_emplace(4096u, 50);
  m.try_emplace(8u, 80);
  verify(m, {{3u, 30}, {4096u, 50}, {8u, 80}});
}

TEST_CASE("sparse_map page size two handles exact page boundaries", "[common][sparse_map][limit]")
{
  map<unsigned, int, 2> m;
  for (unsigned i = 0; i < 50; ++i)
    m.try_emplace(i, int(i * 2));
  for (unsigned i = 0; i < 50; i += 2)
    REQUIRE(m.try_erase(i));
  for (unsigned i = 1; i < 50; i += 2) REQUIRE(m.at(i)
    == int(i * 2));
}

TEST_CASE("sparse_map page size 64 handles wide rows", "[common][sparse_map][limit]")
{
  map<unsigned, int, 64> m;
  for (unsigned i = 0; i < 600; ++i)
    m.try_emplace(i, int(i * 3));
  for (unsigned i = 0; i < 600; ++i) REQUIRE(m.at(i)
    == int(i * 3));
}

TEST_CASE("sparse_map default page size behaves identically", "[common][sparse_map][limit]")
{
  heim::sparse_map<unsigned, int> m;
  m.try_emplace(0u, 10);
  m.try_emplace(1023u, 20);
  m.try_emplace(1024u, 30);
  REQUIRE(m.at(0u) == 10);
  REQUIRE(m.at(1023u) == 20);
  REQUIRE(m.at(1024u) == 30);
  REQUIRE(m.try_erase(1023u));
  REQUIRE(m.at(1024u) == 30);
}

TEST_CASE("sparse_map max_size is at least current size", "[common][sparse_map][limit]")
{
  map<> m;
  for (unsigned i = 0; i < 300; ++i)
  {
    REQUIRE(m.size()<m.max_size());
    REQUIRE(m.try_emplace(i, int(i)).second);
  }
  REQUIRE(m.size() <= m.max_size());
}
