#include "doctest.h"
#include "component_manager.hpp"

TEST_CASE("heim::component_manager: ")
{
  using component_manager_t
  = heim::component_manager<unsigned int>
      // declare components
      ::declare_component<int   >
      ::declare_component<short >
      ::declare_component<float >
      ::declare_component<double>
      // declare syncs
      ::declare_sync<int  , short >
      // ::declare_sync<int, float> // impossible: int is already synced once
      ::declare_sync<float, double>;

  CHECK_EQ(component_manager_t::scheme_type::size, 2);

  component_manager_t mgr_0{};
  component_manager_t mgr_1{mgr_0};
  component_manager_t mgr_2{std::move(mgr_0)};

  component_manager_t mgr_3; mgr_3 = mgr_1;
  component_manager_t mgr_4; mgr_4 = std::move(mgr_2);

  CHECK_EQ(mgr_1.size<int   >(), 0);
  CHECK_EQ(mgr_1.size<short >(), 0);
  CHECK_EQ(mgr_1.size<float >(), 0);
  CHECK_EQ(mgr_1.size<double>(), 0);
}