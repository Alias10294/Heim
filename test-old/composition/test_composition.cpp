#include <chrono>
#include <iostream>
#include "../doctest.h"
#include "../../include-old/composition/composition.hpp"

TEST_CASE("heim::composition: capacity when empty")
{
  heim::composition<std::uint32_t, int> c;
  CHECK(c.empty()    == true);
  CHECK(c.size()     == 0);
  CHECK(c.capacity() == 0);
}

TEST_CASE("heim::composition: capacity when modified with emplace & erase")
{
  // Initial insertion
  heim::composition<std::uint32_t, int> c;
  CHECK(c.emplace(1, 2) == true);
  CHECK(c.empty()       == false);
  CHECK(c.size()        == 1);
  CHECK(c.capacity()    != 0);
  CHECK(c.contains(1)   == true);
  CHECK(c[1]            == 2);
  CHECK(c.at(1)         == 2);

  // Re-insertion should not do anything
  CHECK(c.emplace(1, 3) == false);
  CHECK(c.size()        == 1);
  CHECK(c[1]            == 2);
  CHECK(c.at(1)         == 2);

  // Erasure
  CHECK(c.erase(1)    == true);
  CHECK(c.empty()     == true);
  CHECK(c.size()      == 0);
  CHECK(c.capacity()  != 0);
  CHECK(c.contains(1) == false);
  CHECK_THROWS_AS(static_cast<void>(c.at(1)), std::out_of_range);

  // Re-erasing should not do anything
  CHECK(c.erase(1)    == false);
  CHECK(c.empty()     == true);
  CHECK(c.size()      == 0);
  CHECK(c.contains(1) == false);
}

TEST_CASE("heim::composition: multiple emplace and iteration")
{
  heim::composition<std::uint32_t, int> c;
  for (std::size_t i = 0; i < 5; ++i)
    c.emplace(i, i * 2);

  std::size_t idx = 0;
  for (auto [e, i] : c)
  {
    CHECK(e == idx);
    CHECK(i == idx * 2);
    ++idx;
  }

  const auto& cc = c;
  idx = 0;
  for (auto [e, i] : cc)
  {
    CHECK(e == idx);
    CHECK(i == idx * 2);
    ++idx;
  }
}

TEST_CASE("heim::composition: stress test (100'000'000, int) & quick benchmark")
{
  heim::composition<std::uint32_t, int> c;
  const std::size_t test_size = 100'000'000;

  auto start = std::chrono::high_resolution_clock::now();
  for (std::size_t i = 0; i < test_size; ++i)
    c.emplace(i, i * 2);
  auto end   = std::chrono::high_resolution_clock::now();

  auto duration = std::chrono::duration<double>(end - start).count();
  std::cout << "Insertion: " << duration << "s." << std::endl;

  CHECK(c.size() == test_size);

  std::size_t e_sum = 0;
  std::size_t c_sum = 0;
  start = std::chrono::high_resolution_clock::now();
  for (auto [e, i] : c)
  {
    e_sum += e;
    c_sum += i;
  }
  end   = std::chrono::high_resolution_clock::now();

  duration = std::chrono::duration<double>(end - start).count();
  std::cout << "Iteration: " << duration << "s." << std::endl;

  CHECK(e_sum == c_sum / 2);

  start = std::chrono::high_resolution_clock::now();
  for (std::size_t i = 0; i < test_size; ++i)
    c.erase(i);
  end   = std::chrono::high_resolution_clock::now();

  duration = std::chrono::duration<double>(end - start).count();
  std::cout << "Deletion: " << duration << "s." << std::endl;

  CHECK(c.size() == 0);
}

TEST_CASE("heim::composition: stress test (100'000'000, int) with reserve")
{
  heim::composition<std::uint32_t, int> c;
  const std::size_t test_size = 100'000'000;

  auto start = std::chrono::high_resolution_clock::now();
  c.reserve(test_size);
  for (std::size_t i = 0; i < test_size; ++i)
    c.emplace(i, i * 2);
  auto end   = std::chrono::high_resolution_clock::now();

  auto duration = std::chrono::duration<double>(end - start).count();
  std::cout << "Insertion (with reserve): " << duration << "s." << std::endl;
}