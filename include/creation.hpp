#ifndef HEIM_CREATION_HPP
#define HEIM_CREATION_HPP

#include <memory>
#include "type_sequence.hpp"

namespace heim
{
template<typename>
struct is_component_traits
  : std::false_type
{ };

template<
    typename Component,
    typename Allocator>
struct is_component_traits<
    type_sequence<Component, Allocator>>
  : std::true_type
{ };



template<
    typename Entity,
    typename Sequence>
class creation;

template<
    typename    Entity,
    typename ...Groups>
class creation<
    Entity,
    type_sequence<Groups ...>>
{
public:
  static_assert(
      (is_type_sequence_v<Groups> && ...),
      "heim::creation: "
          "(is_type_sequence_v<Groups> && ...)");

  using group_sequence
  = type_sequence<Groups ...>;

  static_assert(
      std::is_same_v<
          typename group_sequence::flat_t,
          typename group_sequence::flat_t
              ::template filter_t<is_component_traits>>,
      "heim::creation: "
          "std::is_same_v<"
              "typename group_sequence::flat_t, "
              "typename group_sequence::flat_t"
                  "::template filter_t<is_component_traits>>");

  using component_traits_sequence
  = typename group_sequence::flat_t;

  using component_sequence
  = typename component_traits_sequence
      ::template map_t<component_traits_sequence::template get<0>>;

  using allocator_sequence
  = typename component_traits_sequence
      ::template map_t<component_traits_sequence::template get<1>>;



};


}

#endif // HEIM_CREATION_HPP
