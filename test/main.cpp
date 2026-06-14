#include <iostream>
#include <heim/registry.hpp>
#include <heim/ecs/registry/sparse/registry.hpp>

#include "heim/ecs/registry/match_view.hpp"

struct position { float x, y, z; };
struct velocity { float x, y, z; };
struct tag      { };


// using registry
// = heim::sparse::static_registry::with_all<position, velocity, tag>;
//
// using expression
// = heim::conjunction<position, velocity, heim::negation<tag>>;
//
//
// int main()
// {
//   registry reg{};
//   auto     e0 {reg.entity()};
//
//   e0.emplace<position>(0.f, 0.f, 0.f);
//   e0.emplace<velocity>(1.f, 0.f, 0.f);
//
//   std::cout << "e0 expired: " << e0.expired()             << std::endl; // 0
//   std::cout << "e0 matches: " << e0.matches<expression>() << std::endl; // 1
//
//   auto &pos{e0.get<position>()};
//   auto &vel{e0.get<velocity>()};
//
//   std::cout << "e0's position (before): " << pos.x << ' ' << pos.y << ' ' << pos.z << std::endl;
//   std::cout << "e0's velocity (before): " << vel.x << ' ' << vel.y << ' ' << vel.z << std::endl;
//
//   for (auto e : reg.query<expression>())
//   {
//     auto       &[px, py, pz]{e.get<position>()};
//     auto const &[vx, vy, vz]{e.get<velocity>()};
//
//     px += vx;
//     py += vy;
//     pz += vz;
//   }
//
//   std::cout << "e0's position (after):  " << pos.x << ' ' << pos.y << ' ' << pos.z << std::endl;
//   std::cout << "e0's velocity (after):  " << vel.x << ' ' << vel.y << ' ' << vel.z << std::endl;
//
//   e0.destroy();
//
//   std::cout << "e0 expired: " << e0.expired()             << std::endl; // 1
//   std::cout << "e0 matches: " << e0.matches<expression>() << std::endl; // 0
// }

using registry
= heim::ecs::sparse::auto_registry;

using expression
= heim::conjunction<position, velocity, heim::negation<tag>>;

int main()
{
  registry   reg{};
  auto const e0 {reg.make()};

  reg.emplace<position>(e0, 0.f, 0.f, 0.f);
  reg.emplace<velocity>(e0, 1.f, 0.f, 0.f);

  std::cout << "e0 expired: " << reg.expired(e0)             << std::endl; // 0
  std::cout << "e0 matches: " << reg.matches<expression>(e0) << std::endl; // 1

  auto &pos{reg.get<position>(e0)};
  auto &vel{reg.get<velocity>(e0)};

  std::cout << "e0's position (before): " << pos.x << ' ' << pos.y << ' ' << pos.z << std::endl;
  std::cout << "e0's velocity (before): " << vel.x << ' ' << vel.y << ' ' << vel.z << std::endl;

  for (auto const e : reg | heim::ecs::views::match<expression>)
  {
    auto       &[px, py, pz]{reg.get<position>(e)};
    auto const &[vx, vy, vz]{reg.get<velocity>(e)};

    px += vx;
    py += vy;
    pz += vz;
  }

  std::cout << "e0's position (after):  " << pos.x << ' ' << pos.y << ' ' << pos.z << std::endl;
  std::cout << "e0's velocity (after):  " << vel.x << ' ' << vel.y << ' ' << vel.z << std::endl;

  reg.destroy(e0);

  std::cout << "e0 expired: " << reg.expired(e0)             << std::endl; // 1
  std::cout << "e0 matches: " << reg.matches<expression>(e0) << std::endl; // 0
}