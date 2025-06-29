#include "doctest.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <random>

#include "composition.hpp"

TEST_CASE("heim::composition: creation and emptiness")
{
  heim::composition<int> c;
  CHECK(c.empty());
}
TEST_CASE("heim::composition: basic emplace")
{
  heim::composition<int> c;
  CHECK(c.emplace(1, 2));
  CHECK_FALSE(c.emplace(1, 2));
  CHECK_FALSE(c.emplace(1, 3));
}
struct complex_struct
{
  int x;
  int y;
  std::string str;
  std::unique_ptr<int> ptr;
  std::vector<int> vec;
};
TEST_CASE("heim::composition: complex emplace")
{
  heim::composition<complex_struct> c;
  CHECK(c.emplace(1,
      2,
      4,
      "string",
      std::make_unique<int>(2),
      std::vector<int>({ 1, 2, 4, 8, 16 })));
}
TEST_CASE("heim::composition: emplace 1'000'000 complex entities")
{
  heim::composition<complex_struct> c;
  for (std::size_t i = 0; i < 1'000'000; ++i)
    c.emplace(
        i,
        i * 2,
        i * 3,
        "string",
        std::make_unique<int>(i * 2),
        std::vector<int>({ 1, 2, 4, 8, 16 }));
  CHECK(c.size() == 1'000'000);
}

TEST_CASE("heim::composition: contains entity")
{
  heim::composition<int> c;
  c.emplace(1, 2);
  CHECK(c.contains(1));
  CHECK_FALSE(c.contains(2));
}
TEST_CASE("heim::composition: erase entity")
{
  heim::composition<int> c;
  c.emplace(1, 2);
  c.erase(1);
  CHECK_FALSE(c.contains(1));
}
TEST_CASE("heim::composition: erase complex emplace")
{
  heim::composition<complex_struct> c;
  c.emplace(1,
      2,
      4,
      "string",
      std::make_unique<int>(2),
      std::vector<int>({ 1, 2, 4, 8, 16 }));
  c.erase(1);
  CHECK_FALSE(c.contains(1));
}
TEST_CASE("heim::composition: erase 1'000'000 entities")
{
  heim::composition<complex_struct> c;
  for (std::size_t i = 0; i < 1'000'000; ++i)
    c.emplace(
        i,
        i * 2,
        i * 3,
        "string",
        std::make_unique<int>(i * 2),
        std::vector<int>({ 1, 2, 4, 8, 16 }));
  for (std::size_t i = 0; i < 1'000'000; ++i)
    c.erase(i);
  CHECK(c.empty());
}
TEST_CASE("heim::composition: clear entities")
{
  heim::composition<complex_struct> c;
  for (std::size_t i = 0; i < 1000; ++i)
    c.emplace(
        i,
        i * 2,
        i * 3,
        "string",
        std::make_unique<int>(i * 2),
        std::vector<int>({ 1, 2, 4, 8, 16 }));
  c.clear();
  CHECK(c.empty());
}


TEST_CASE("heim::composition: operator[]")
{
  heim::composition<int> c;
  c.emplace(1, 2);
  CHECK(c[1] == 2);
}
TEST_CASE("heim::composition: operator[] is right")
{
  heim::composition<int> c;
  c.emplace(1, 2);
  c.emplace(2, 4);
  CHECK_FALSE(c[1] == 4);
  CHECK(c[2] == 4);
}

TEST_CASE("heim::composition: at")
{
  heim::composition<int> c;
  c.emplace(1, 2);
  CHECK(c.at(1) == 2);
  c.emplace(2, 4);
  CHECK(c.at(2) == 4);
  CHECK_FALSE(c.at(1) == 4);
}
TEST_CASE("heim::composition: at throws")
{
  heim::composition<int> c;
  CHECK_THROWS_AS(c.at(1), std::out_of_range);
}
TEST_CASE("heim::composition: reserve and capacity")
{
  heim::composition<int> c;
  c.reserve(1'000);
  CHECK(c.capacity() == 1'000);
}
TEST_CASE("heim::composition: shrink_to_fit")
{
  heim::composition<int> c;
  c.reserve(3);
  CHECK(c.capacity() == 3);
  c.emplace(1, 2);
  c.shrink_to_fit();
  CHECK(c.capacity() == 1);
}
TEST_CASE("heim::composition: operator[] after shrink_to_fit")
{
  heim::composition<int> c;
  c.reserve(10);
  c.emplace(10000, 1);
  c.shrink_to_fit();
  CHECK(c.capacity() == 1);
  CHECK(c.at(10000) == 1);
}
TEST_CASE("heim::composition: sort")
{
  heim::composition<int> c;

  std::random_device dev;
  std::mt19937 rng(dev());
  std::uniform_int_distribution<std::mt19937::result_type> dist(1, 1000);

  for (std::size_t i = 0; i < 10; ++i)
    c.emplace(i, dist(rng));

  auto cmp =
      [](const int& i1, const int& i2) -> bool
      { return i1 < i2; };
  c.sort(cmp);

  std::vector<int> v;
  for (auto it : c)
    v.push_back(it.component);

  for (std::size_t i = 0; i < 9; ++i)
    CHECK(v[i] <= v[i + 1]);
}