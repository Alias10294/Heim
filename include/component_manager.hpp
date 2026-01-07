#ifndef HEIM_COMPONENT_MANAGER_HPP
#define HEIM_COMPONENT_MANAGER_HPP

#include "allocator.hpp"
#include "container.hpp"
#include "entity.hpp"
#include "type_sequence.hpp"

namespace heim
{
template<
    typename Entity            = entity,
    typename Allocator         = allocator<Entity>,
    typename ComponentSequence = type_sequence<>,
    typename GroupSequence     = type_sequence<>>
class component_manager
{
public:
  using size_type       = std::size_t;
  using difference_type = std::ptrdiff_t;

  using entity_type        = Entity;
  using allocator_type     = Allocator;
  using component_sequence = ComponentSequence;
  using group_sequence     = GroupSequence;

private:
  struct component_sequence_traits
  {
  public:
    using type = ComponentSequence;

    static_assert(is_type_sequence_v<type>);
    static_assert(type::template filter<is_unqualified>::size == type::size);
    static_assert(type::is_unique);

  private:
    template<typename Component>
    struct meta_container_for
    {
      using type
      = container<Component, entity_type, allocator_type>;
    };

  public:
    template<typename Component>
    static constexpr
    size_type
    index
    = type::template index<Component>;

    template<typename Component>
    using container_for
    = meta_container_for<Component>::type;

    using container_tuple
    = typename type::template map<meta_container_for>::tuple;
  };

  struct group_sequence_traits
  {
  public:
    using type = GroupSequence;

    static_assert(is_type_sequence_v<type>);
    static_assert(type::template filter<is_type_sequence>::size == type::size);
    static_assert(type::flatten::is_unique);
    static_assert(type::flatten::template difference<component_sequence>::size == 0);

  private:
    template<typename Component>
    struct predicate
    {
      template<typename Group>
      struct contains : bool_constant<Group::template contains<Component>> { };
    };

    template<typename Component>
    using filtered_for
    = type::flatten::template filter<predicate<Component>::template contains>;

  public:
    template<typename Component>
    static constexpr
    bool
    is_grouped
    = filtered_for<Component>::size > 0;

    template<typename Component>
    requires(is_grouped<Component>)
    static constexpr
    size_type
    index
    = type::template index<filtered_for<Component>::template get<0>>;

    using length_array
    = std::array<size_type, type::size>;
  };


private:
  typename component_sequence_traits::container_tuple m_containers;
  typename group_sequence_traits    ::length_array    m_groups;

public:
};

}

#endif // HEIM_COMPONENT_MANAGER_HPP
