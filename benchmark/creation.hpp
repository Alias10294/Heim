#ifndef HEIM_BENCHMARK_CREATE_HPP
#define HEIM_BENCHMARK_CREATE_HPP

#include <cstddef>
#include <iostream>
#include <random>
#include "core.hpp"

namespace heim::benchmark::creation
{
template<std::size_t ECount, std::size_t CCount, std::size_t N>
constexpr
void benchmark();

template<std::size_t ECount, std::size_t CCount, std::size_t N>
constexpr
void benchmark()
{
  using entity_vector
  = std::vector<registry::entity_type>;

  std::mt19937                  rng (std::random_device{}());
  std::uniform_int_distribution pick(0, 4);
  std::chrono::nanoseconds      avg (0);

  for (std::size_t i = 0; i < N; ++i)
  {
    registry      reg;
    entity_vector es (ECount);

    const int choice1 = pick(rng);
    const int choice2 = pick(rng);

    auto start  = clock::now();
    for (std::size_t j = 0; j < ECount; ++j)
    {
      es[j] = reg.create();

      switch (choice1) {
      case 0: reg.emplace<component::transform>(es[j]); break;
      case 1: reg.emplace<component::tag      >(es[j]); break;
      case 2: reg.emplace<component::health   >(es[j]); break;
      case 3: reg.emplace<component::damage   >(es[j]); break;
      default:reg.emplace<component::sprite   >(es[j]); break;
      }

      switch (choice2) {
      case 0: reg.emplace<component::transform>(es[j]); break;
      case 1: reg.emplace<component::tag      >(es[j]); break;
      case 2: reg.emplace<component::health   >(es[j]); break;
      case 3: reg.emplace<component::damage   >(es[j]); break;
      default:reg.emplace<component::sprite   >(es[j]); break;
      }
    }
    auto finish = clock::now();
    avg += std::chrono::duration_cast<std::chrono::nanoseconds>(finish - start);
  }

  avg /= N;
  std::cout << avg << std::endl;
}


}

#endif // HEIM_BENCHMARK_CREATE_HPP