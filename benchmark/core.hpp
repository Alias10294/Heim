#ifndef HEIM_BENCHMARK_CORE_HPP
#define HEIM_BENCHMARK_CORE_HPP

#include <chrono>
#include <cstdint>
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


}

using registry
= heim::registry<heim::sparse_set_based::storage<>
    ::component<component::transform>
    ::component<component::tag>
    ::component<component::health>
    ::component<component::damage>
    ::component<component::sprite>>;

using clock
= std::chrono::steady_clock;


}

#endif // HEIM_BENCHMARK_CORE_HPP