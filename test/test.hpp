#ifndef HEIM_TEST_HPP
#define HEIM_TEST_HPP

#include <iostream>
#include "heim/ecs/sparse/registry.hpp"

namespace heim::test
{
constexpr
void
test()
{
  std::cout << "TEST:\n";


  struct position { float x, y, z; };
  struct velocity { float x, y, z; };
  struct health   { int hp; };
  struct tag      { };

  using registry         = heim::sparse::registry<>::with_all<position, velocity, health, tag>;
  using query_expression = heim::conjunction<position, velocity, tag, heim::negation<health>>;
  using entity           = registry::entity;

  registry r;

  entity e0(r);
  e0.emplace<position>(0.f, 0.f, 0.f);
  e0.emplace<velocity>(1.f, 0.f, 0.f);
  e0.emplace<tag     >();

  entity e1(r);
  e1.emplace<position>(0.f, 1.f, 0.f);
  e1.emplace<health  >(10);

  auto &[px, py, pz] = e0.get<position>();
  auto &[vx, vy, vz] = e0.get<velocity>();

  std::cout << "position (before): " << px << py << pz << '\n';
  std::cout << "velocity (before): " << vx << vy << vz << '\n';

  for (auto e : r.query<query_expression>())
  {
    auto       &[pos_x, pos_y, pos_z] = e.get<position>();
    auto const &[vel_x, vel_y, vel_z] = e.get<velocity>();

    pos_x += vel_x;
    pos_y += vel_y;
    pos_z += vel_z;
  }

  std::cout << "position (after):  " << px << py << pz << '\n';
  std::cout << "velocity (after):  " << vx << vy << vz << '\n';

  e0.destroy();
  e1.destroy();

  std::cout << '\n';
}


}

#endif // HEIM_TEST_HPP
