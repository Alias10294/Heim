#include <iostream>
#include <heim/heim.hpp>

struct position { float x, y, z; };
struct velocity { float x, y, z; };
struct tag      { };


using registry
= heim::sparse::registry::with_all<position, velocity, tag>;

using identifier
= typename registry::identifier_type;

using entity
= heim::entity<registry>;

using expression
= heim::conjunction<position, velocity, heim::negation<tag>>;


int main()
{
  registry reg;

  identifier const id0{reg.create()};
  reg.emplace<position>(id0, 0.f, 0.f, 0.f);
  reg.emplace<velocity>(id0, 1.f, 0.f, 0.f);

  entity e0{reg};
  e0.emplace<position>(0.f, 1.f, 0.f);
  e0.emplace<tag>     ();

  std::cout << "id0 expired: " << reg.expired(id0) << std::endl; // false
  std::cout << "e0  expired: " << e0 .expired()    << std::endl; // false

  auto const &pos = reg.get<position>(id0);
  auto const &vel = reg.get<velocity>(id0);

  std::cout << "id0 matches: " << reg.matches<expression>(id0) << std::endl;
  std::cout << "e0  matches: " << e0 .matches<expression>()    << std::endl;

  std::cout << "id0's position (before): " << pos.x << ' ' << pos.y << ' ' << pos.z << std::endl;
  std::cout << "id0's velocity (before): " << vel.x << ' ' << vel.y << ' ' << vel.z << std::endl;

  for (auto e : reg.query<expression>())
  {
    auto       &[px, py, pz] = e.get<position>();
    auto const &[vx, vy, vz] = e.get<velocity>();

    px += vx;
    py += vy;
    pz += vz;
  }

  std::cout << "id0's position (after):  " << pos.x << ' ' << pos.y << ' ' << pos.z << std::endl;
  std::cout << "id0's velocity (after):  " << vel.x << ' ' << vel.y << ' ' << vel.z << std::endl;

  reg.destroy(id0);
  e0 .destroy();

  std::cout << "id0 expired: " << reg.expired(id0) << std::endl; // true
  std::cout << "e0  expired: " << e0 .expired()    << std::endl; // true
}