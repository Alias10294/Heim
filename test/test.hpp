#ifndef HEIM_TEST_HPP
#define HEIM_TEST_HPP

#include <iostream>
#include <ranges>
#include "heim/heim.hpp"
#include "heim/storage/sparse_set/storage.hpp"

namespace heim::test
{
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
  // using registry
  // = heim::registry<heim::sparse_set_based::storage<>
  //     ::component<position>
  //     ::component<velocity>
  //     ::component<health  >
  //     ::component<tag     >
  //     ::group<position, velocity, tag>>;

  using query_expression
  = heim::query_expression<>
      ::include<position, velocity const, tag>
      ::exclude<health>;

  using entity
  = heim::entity<registry>;

  registry r;

  // auto const e0 = r.create();
  // r.emplace<position>(e0, 0.f, 0.f, 0.f);
  // r.emplace<velocity>(e0, 1.f, 0.f, 0.f);
  // r.emplace<tag     >(e0);
  //
  // auto const e1 = r.create();
  // r.emplace<position>(e1, 0.f, 1.f, 0.f);
  // r.emplace<health  >(e1, 10);
  //
  // auto &[px, py, pz] = r.get<position>(e0);
  // auto &[vx, vy, vz] = r.get<velocity>(e0);
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

  for (auto &&[id, pos, vel] : r.query<query_expression>())
  {
    pos.x += vel.x;
    pos.y += vel.y;
    pos.z += vel.z;
  }

  std::cout << "position (after):  " << px << py << pz << '\n';
  std::cout << "velocity (after):  " << vx << vy << vz << '\n';

  // r.destroy(e0);
  // r.destroy(e1);
  e0.destroy();
  e1.destroy();

  std::cout << '\n';
}


} // namespace heim::test

#endif // HEIM_TEST_HPP