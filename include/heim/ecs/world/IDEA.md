```c++
#include <heim/heim.hpp>

int main()
{
  using world  = heim::world<heim::sparse::runtime_registry>
  using entity = typename world::entity_type;
  
  world w{};
  
  w.declare(
      heim::make_system<
            heim::conjunction<position, velocity>>(
          [](auto &ctx, auto movables)
          {
            auto dt = ctx.delta_time<std::chrono::milliseconds>();
            for (auto e : movables)
            {
              auto       &[px, py, pz]{e.get<position>()};
              auto const &[vx, vy, vz]{e.get<velocity>()};
              
              if (px || py || pz)
              {
                px += vx * dt;
                py += vy * dt;
                pz += vz * dt;
              }
              else
                ctx.commands(e).emplace<immobile>();
            }
          }));
  // ...
    
  entity e0{w.entity()};
  entity e1{w.entity()};
  
  e0.emplace<position>(0.f, 0.f, 0.f);
  e0.emplace<velocity>(1.f, 0.f, 0.f);
  
  e1.emplace<position>(0.f, 1.f, 0.f);
  e1.emplace<health  >(100);
  
  w.prepare();
  w.run    ();
  w.clear  ();
  
  return 0;
}

```