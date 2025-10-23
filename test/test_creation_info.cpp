#include "doctest.h"
#include "creation_info.hpp"

#include <memory>

TEST_CASE("heim::creation_info: assertions")
{
  using CIi = heim::make_component_info<int,   256, std::allocator<int>>;
  using CIf = heim::make_component_info<float, 128, std::allocator<float>>;

  using S1 = heim::type_sequence<CIi>;
  using S2 = heim::type_sequence<CIf>;
  using S3 = heim::type_sequence<CIi, CIf>;

  static_assert( heim::is_component_info_v<CIi> );
  static_assert( heim::is_component_info_v<CIf> );
  static_assert( heim::is_sync_info_v<S1> );
  static_assert( heim::is_sync_info_v<S2> );
  static_assert( heim::is_sync_info_v<S3> );

  using CR0 = heim::type_sequence<S1, S1>;
  static_assert(!heim::is_creation_info_v<CR0>);

  using CR1 = heim::type_sequence<S1, S2>;
  static_assert( heim::is_creation_info_v<CR1> );

  using CR1T = heim::creation_info_traits<CR1>;
  static_assert( heim::is_creation_info_v<CR1T::type> );

  using CR2T = CR1T::sync_t<int, float>;
  static_assert( heim::is_creation_info_v<CR2T::type> );

  using CId  = heim::make_component_info<double, 64, std::allocator<double>>;
  using CR3T = CR2T::add_t<CId>;
  static_assert( heim::is_creation_info_v<CR3T::type> );

}