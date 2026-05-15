#include <heim/registry.hpp>

struct position { float x, y, z; };
struct velocity { float x, y, z; };
struct tag      { };

using registry = heim::sparse::registry::with_all<position, velocity, tag>;
using entity   = typename registry::entity_type;


int main()
{
  registry reg{};
  entity   e0 {reg.create()};
  entity   e1 {reg.create()};
  
  e0.emplace<position>(0.f, 0.f, 0.f);
  e0.emplace<velocity>(1.f, 0.f, 0.f);
  
  e1.emplace<position>(0.f, 1.f, 0.f);
  e1.emplace<tag>     ();

  for (auto e : reg.query<heim::conjunction<position, velocity, heim::negation<tag>>>())
  {
    auto       &[px, py, pz]{e.get<position>()};
    auto const &[vx, vy, vz]{e.get<velocity>()};

    px += vx;
    py += vy;
    pz += vz;
  }

  e0.destroy();
  e1.destroy();
}