#include <iostream>
#include <heim/heim.hpp>

struct position { float x, y, z; };
struct velocity { float x, y, z; };
struct tag      { };


using registry
= heim::sparse::registry<>
    ::with_all<position, velocity>
    ::with    <tag, 0>;

using expression
= heim::conjunction<position, velocity, heim::negation<tag>>;

using entity
= heim::entity<registry>;

int main()
{
  registry r;

  auto const id0 = r.create();
  r.emplace<position>(id0, 0.f, 0.f, 0.f);
  r.emplace<velocity>(id0, 1.f, 0.f, 0.f);

  entity e0{r};
  e0.emplace<position>(0.f, 1.f, 0.f);
  e0.emplace<tag>     ();

  auto const &pos = r.get<position>(id0);
  auto const &vel = r.get<velocity>(id0);

  std::cout << "id0's position(before): " << pos.x << ' ' << pos.y << ' ' << pos.z << std::endl;
  std::cout << "id0's velocity(before): " << vel.x << ' ' << vel.y << ' ' << vel.z << std::endl;

  for (auto e : r.query<expression>())
  {
    auto       &[px, py, pz] = e.get<position>();
    auto const &[vx, vy, vz] = e.get<velocity>();

    px += vx;
    py += vy;
    pz += vz;
  }

  std::cout << "id0's position(after):  " << pos.x << ' ' << pos.y << ' ' << pos.z << std::endl;
  std::cout << "id0's velocity(after):  " << vel.x << ' ' << vel.y << ' ' << vel.z << std::endl;

  r .destroy(id0);
  e0.destroy();
}