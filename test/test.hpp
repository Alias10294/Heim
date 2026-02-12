#ifndef HEIM_TEST_HPP
#define HEIM_TEST_HPP

#include <iostream>
#include "heim/heim.hpp"

inline void test()
{
  std::cout << "TEST:\n";


  struct position { float x, y, z; };
  struct velocity { float x, y, z; };
  struct health   { int hp; };
  struct tag      { };

  using registry
  = heim::registry<heim::sparse_set_based::storage<>
      ::component<position>
      ::component<velocity>
      ::component<health  >
      ::component<tag     >>;

  using query_expression
  = heim::query_expression<>
      ::include<position, velocity const, tag>
      ::exclude<health>;

  registry r;

  auto const e0 = r.create();
  r.emplace<position>(e0, 0.f, 0.f, 0.f);
  r.emplace<velocity>(e0, 1.f, 0.f, 0.f);
  r.emplace<tag     >(e0);

  auto const e1 = r.create();
  r.emplace<position>(e1, 0.f, 1.f, 0.f);
  r.emplace<health  >(e1, 10);

  auto &[px, py, pz] = r.get<position>(e0);
  auto &[vx, vy, vz] = r.get<velocity>(e0);

  std::cout << "position (before): " << px << py << pz << '\n';
  std::cout << "velocity (before): " << vx << vy << vz << '\n';

  for (auto &&[e, pos, vel] : r.query<query_expression>())
  {
    pos.x += vel.x;
    pos.y += vel.y;
    pos.z += vel.z;
  }

  std::cout << "position (after):  " << px << py << pz << '\n';
  std::cout << "velocity (after):  " << vx << vy << vz << '\n';

  r.destroy(e0);
  r.destroy(e1);

  std::cout << '\n';
}


#endif // HEIM_TEST_HPP