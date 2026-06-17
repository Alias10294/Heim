#include <iostream>
#include <heim/heim.hpp>


struct position { int x, y; };
struct velocity { int x, y; };
struct tag      { };

using registry
= heim::ecs::sparse::auto_registry;

using expression
= heim::ecs::conjunction<position, velocity, heim::ecs::negation<tag>>;


int main()
{
  registry   reg{};
  auto const id0{reg.make()};
  auto const id1{reg.make()};

  // DEBUG
  std::cout << "expired before: " << reg.expired(id0) << std::endl; // 0
  std::cout << "expired before: " << reg.expired(id1) << std::endl; // 0
  // DEBUG

  reg.emplace<position>(id0, 0, 0);
  reg.emplace<velocity>(id0, 1, 0);

  reg.emplace<position>(id1, 0, 1);
  reg.emplace<tag     >(id1);

  // DEBUG
  std::cout << "id0 matches: " << reg.matches<heim::ecs::conjunction<position, velocity>>(id0) << std::endl; // 1
  std::cout << "id1 matches: " << reg.matches<heim::ecs::conjunction<position, tag     >>(id1) << std::endl; // 1

  position const &p{reg.get<position>(id0)};
  velocity const &v{reg.get<velocity>(id0)};
  std::cout << "position before: " << p.x << ' ' << p.y << std::endl; // 0 0
  std::cout << "velocity before: " << v.x << ' ' << v.y << std::endl; // 1 0
  // DEBUG

  for (auto const id : reg | heim::ecs::views::match<expression>)
  {
    auto       &[px, py]{reg.get<position>(id)};
    auto const &[vx, vy]{reg.get<velocity>(id)};

    px += vx;
    py += vy;
  }

  // DEBUG
  std::cout << "position after: " << p.x << ' ' << p.y << std::endl; // 1 0
  std::cout << "velocity after: " << v.x << ' ' << v.y << std::endl; // 1 0
  // DEBUG

  reg.destroy(id0);
  reg.destroy(id1);

  // DEBUG
  std::cout << "expired after: " << reg.expired(id0) << std::endl; // 1
  std::cout << "expired after: " << reg.expired(id1) << std::endl; // 1
  // DEBUG
}