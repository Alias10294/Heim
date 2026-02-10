#include <iostream>
#include <ostream>

#include "doctest.h"
#include "registry.hpp"
#include "storage/sparse_set/storage.hpp"

TEST_CASE("test")
{
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

  auto q = r.query<query_expression>();

  for (auto &&[e, pos, vel] : q)
  {
    pos.x += vel.x;
    pos.y += vel.y;
    pos.z += vel.z;
  }

  r.destroy(e0);
  r.destroy(e1);
}