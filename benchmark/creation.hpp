#ifndef HEIM_BENCHMARK_CREATION_HPP
#define HEIM_BENCHMARK_CREATION_HPP

#include <random>
#include <chrono>
#include <cstdint>
#include <iostream>
#include "heim/heim.hpp"

namespace heim::benchmark
{
namespace component
{
struct transform
{
  struct position { float x = 0, y = 0; };
  struct velocity { float x = 0, y = 0; };


  position pos;
  velocity vel;
};

struct tag
{ };

struct health
{
  enum class status_effect { spawn, alive, dead };

  std::int32_t  hp     = 0;
  std::int32_t  max_hp = 0;
  status_effect status = status_effect::spawn;
};

struct damage
{
  std::int32_t atk = 0;
  std::int32_t def = 0;
};

struct sprite
{
  char character = ' ';
};


} // namespace component

using registry
= heim::registry<heim::sparse_set_based::storage<>
    ::component<component::transform>
    ::component<component::tag>
    ::component<component::health>
    ::component<component::damage>
    ::component<component::sprite>>;

using clock
= std::chrono::steady_clock;


template<std::size_t ECount, std::size_t CCount, std::size_t N>
constexpr
void benchmark();

template<std::size_t ECount, std::size_t CCount, std::size_t N>
constexpr
void benchmark()
{
  std::cout << "BENCHMARK:\n";


  using entity_vector
  = std::vector<registry::entity_type>;

  std::mt19937                  rng (std::random_device{}());
  std::uniform_int_distribution pick(0, 4);
  std::chrono::nanoseconds      avg (0);

  for (std::size_t i = 0; i < N; ++i)
  {
    registry      reg;
    entity_vector es (ECount);

    auto start = clock::now(); // START

    for (std::size_t j = 0; j < ECount; ++j)
    {

      es[j] = reg.create();

      std::array<int, CCount> choices;
      for (auto choice : choices)
        choice = pick(rng);
      for (auto const choice : choices)
      {
        switch (choice)
        {
        case 0 : reg.emplace<component::transform>(es[j]); break;
        case 1 : reg.emplace<component::tag      >(es[j]); break;
        case 2 : reg.emplace<component::health   >(es[j]); break;
        case 3 : reg.emplace<component::damage   >(es[j]); break;
        default: reg.emplace<component::sprite   >(es[j]); break;
        }
      }
    }

    auto finish = clock::now(); // FINISH
    avg += std::chrono::duration_cast<decltype(avg)>(finish - start);
  }

  avg /= N;
  std::cout
      << ECount << " entities with each "
      << CCount << " components created in: "
      << std::chrono::duration_cast<std::chrono::microseconds>(avg) << '\n';
}


} // namespace heim::benchmark

#endif // HEIM_BENCHMARK_CREATION_HPP