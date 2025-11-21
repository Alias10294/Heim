#ifndef HEIM_REGISTRY_HPP
#define HEIM_REGISTRY_HPP

#include <cstddef>
#include <memory>
#include <type_traits>
#include "lib/index_map.hpp"
#include "lib/type_sequence.hpp"
#include "lib/utility.hpp"
#include "entity.hpp"
#include "entity_manager.hpp"

namespace heim
{
namespace detail
{
template<typename TSeq>
struct is_component_scheme
  : std::false_type
{ };

template<
    typename    T,
    std::size_t PageSize,
    typename    Alloc>
struct is_component_scheme<type_sequence<
    T,
    std::integral_constant<std::size_t, PageSize>,
    Alloc>>
  : std::true_type
{ };

template<typename TSeq>
inline constexpr bool
is_component_scheme_v = is_component_scheme<TSeq>::value;


template<typename TSeq>
class component_scheme_traits
{
private:
  static_assert(
      is_component_scheme_v<TSeq>,
      "heim::detail::component_scheme_traits<TSeq>: "
          "heim::detail::is_component_scheme_v<TSeq>;");

public:
  using type
  = TSeq;


  using component
  = typename TSeq::template get<0>;

  static constexpr std::size_t
  page_size = TSeq::template get<1>::value;

  using allocator
  = typename TSeq::template get<2>;

};

template<
    typename    T,
    std::size_t PageSize,
    typename    Alloc>
using component_scheme
= type_sequence<T, std::integral_constant<std::size_t, PageSize>, Alloc>;



template<typename TSeq>
struct is_sync_scheme
  : std::false_type
{ };

template<typename ...TSeqs>
struct is_sync_scheme<type_sequence<TSeqs ...>>
  : std::bool_constant<
        (is_component_scheme_v<TSeqs> && ...)
     && type_sequence<typename component_scheme_traits<TSeqs>::component ...>
            ::is_unique>
{ };

template<typename TSeq>
inline constexpr bool
is_sync_scheme_v = is_sync_scheme<TSeq>::value;


template<typename TSeq>
class sync_scheme_traits
{
private:
  static_assert(
      is_sync_scheme_v<TSeq>,
      "heim::detail::sync_scheme_traits<TSeq>: "
          "heim::detail::is_sync_scheme_v<TSeq>;");

public:
  using type
  = TSeq;

private:
  template<typename USeq>
  struct to_component
  {
  public:
    using type
    = typename component_scheme_traits<USeq>::component;

  };

public:
  using component_sequence
  = typename TSeq::template map<to_component>;

  template<typename T>
  using component_scheme
  = typename TSeq::template get<component_sequence::template index<T>>;

};



template<typename TSeq>
struct is_scheme
  : std::false_type
{ };

template<typename ...TSeqs>
struct is_scheme<type_sequence<TSeqs ...>>
  : std::bool_constant<
        (is_sync_scheme_v<TSeqs> && ...)
     && type_sequence<
            typename sync_scheme_traits<TSeqs>::component_sequence ...>
                ::flat
                ::is_unique>
{ };

template<typename TSeq>
inline constexpr bool
is_scheme_v = is_scheme<TSeq>::value;


template<
    typename TSeq,
    typename T>
struct registry_scheme_get_sync_scheme
{
private:
  static_assert(
      is_scheme_v<TSeq>,
      "heim::detail::registry_scheme_get_sync_scheme<TSeq, T>: "
          "heim::detail::is_registry_scheme_v<TSeq>;");

private:
  template<typename USeq>
  struct has_component
  {
  private:
    static_assert(
        is_sync_scheme_v<USeq>,
        "heim::detail::registry_scheme_get_sync_scheme<TSeq, T>"
            "::has_component<USeq>: "
                "is_sync_scheme_v<USeq>;");

  public:
    static constexpr bool
    value
    = sync_scheme_traits<USeq>
        ::component_sequence
        ::template contains<T>;

  };

public:
  using type
  = typename TSeq
      ::template filter<has_component>
      ::template get<0>;

};


template<typename TSeq>
class registry_scheme_traits
{
private:
  static_assert(
      is_scheme_v<TSeq>,
      "heim::detail::registry_scheme_traits<TSeq>: "
          "is_registry_scheme_v<TSeq>;");

public:
  using type
  = TSeq;


  template<typename T>
  using get_sync_scheme
  = typename registry_scheme_get_sync_scheme<type, T>::type;

private:
  template<typename ...Ts>
  static constexpr bool
  can_sync
  = ((get_sync_scheme<Ts>::size == 1) && ...);


  template<
      typename    First,
      typename    Second,
      typename ...Rest>
  struct sync_components_wrapper
  {
  private:
    static_assert(
        can_sync<First, Second, Rest...>,
        "heim::detail::registry_scheme_traits<TSeq>"
            "::sync_components_wrapper<First, Second, Rest ...>: "
                "heim::detail::registry_scheme_traits<TSeq>::"
                    "can_sync<First, Second, Rest...>;");

  public:
    using type
    = registry_scheme_traits<typename type
        ::template difference<type_sequence<
            get_sync_scheme<First >,
            get_sync_scheme<Second>,
            get_sync_scheme<Rest  > ...>>
        ::template extend    <type_sequence<
            typename get_sync_scheme<First >::template get<0>,
            typename get_sync_scheme<Second>::template get<0>,
            typename get_sync_scheme<Rest  >::template get<0> ...>>>;

  };

public:
  template<
      typename    T,
      std::size_t PageSize,
      typename    Alloc>
  using add_component
  = registry_scheme_traits<
      typename type::template extend<
          type_sequence<component_scheme<T, PageSize, Alloc>>>>;

  template<
      typename    First,
      typename    Second,
      typename ...Rest>
  using sync_components
  = sync_components_wrapper<First, Second, Rest ...>::type;

};


} // namespace detail


/*!
 * @brief The uniform interface to use components.
 *
 * @tparam T The type of component to qualify.
 */
template<typename T>
struct component_traits
{
  using type
  = T;

  static constexpr std::size_t
  page_size = 4096;

  template<typename Index>
  using allocator_type
  = redefine_tag;

};



/*!
 * @brief The central object of the entity-component system that manages both
 *   entities and components alike.
 *
 * @tparam Entity The type of entity.
 * @tparam Alloc  The default allocator for entities (and components).
 * @tparam Scheme The scheme of the registry used to organize components.
 */
template<
    typename Entity,
    typename Alloc  = std::allocator<Entity>,
    typename Scheme = type_sequence<>>
class registry
{
private:
  static_assert(
      detail::is_scheme_v<Scheme>,
      "heim::registry<Entity, Alloc, Scheme>: "
          "detail::is_registry_scheme_v<Scheme>;");

public:
  using entity_type    = Entity;
  using allocator_type = Alloc;
  using scheme_type    = Scheme;


  using index_type      = entity_traits<entity_type>::index_type;
  using generation_type = entity_traits<entity_type>::generation_type;

private:
  using alloc_traits_t
  = std::allocator_traits<allocator_type>;


  using entity_alloc_t
  = typename alloc_traits_t
      ::template rebind_alloc<entity_type>;

  using entity_mgr_t
  = entity_manager<entity_type, allocator_type>;


  template<typename T>
  using component_alloc_t
  = std::conditional_t<
      std::is_same_v<
          typename component_traits<T>
              ::template allocator_type<index_type>,
          redefine_tag>,
      typename alloc_traits_t
          ::template rebind_alloc<std::pair<index_type const, T>>,
      typename component_traits<T>
          ::template allocator_type<index_type>>;

  template<typename TSeq>
  struct to_container
  {
  private:
    static_assert(
        detail::is_component_scheme_v<TSeq>,
        "heim::registry<Index, Scheme>::to_container<TSeq>: "
            "detail::is_component_scheme_v<TSeq>;");

  public:
    using type
    = index_map<
        index_type,
        typename detail::component_scheme_traits<TSeq>::component,
        detail::component_scheme_traits         <TSeq>::page_size,
        typename detail::component_scheme_traits<TSeq>::allocator>;

  };

  using container_tuple_t
  = typename scheme_type
      ::flat
      ::template map<to_container>
      ::to_tuple;

public:
  /*!
   * @brief The registry type with @code T@endcode added as a component type,
   *   along optional parameters @code PageSize@endcode and
   *   @code TAlloc@endcode.
   *
   * @tparam T        The type of component to add.
   * @tparam PageSize The size of each internal page for the component's
   *   container.
   * @tparam TAlloc   The type of the allocator used for the component's
   *   container.
   *
   * @note Component types can only be found once in a given registry, so
   *   adding an already informed component will have no effect.
   */
  template<
      typename    T,
      std::size_t PageSize = component_traits<T>::page_size,
      typename    TAlloc   = component_alloc_t<T>>
  using component
  = registry<
      entity_type,
      allocator_type,
      typename detail::registry_scheme_traits<scheme_type>
          ::template add_component<T, PageSize, TAlloc>
          ::type>;

  /*!
   * @brief The registry type with component types @code First@endcode,
   *   @code Second@endcode and remaining @code Rest ...@endcode types synced
   *   together.
   *
   * @tparam First  The first  component to sync.
   * @tparam Second The second component to sync.
   * @tparam Rest   The rest of the components to sync.
   *
   * @note Syncing a component that is not already informed in the registry
   *   will result in a compilation error.
   */
  template<
      typename    First,
      typename    Second,
      typename ...Rest>
  using sync
  = registry<
      entity_type,
      allocator_type,
      typename detail::registry_scheme_traits<scheme_type>
          ::template sync_components<First, Second, Rest ...>
          ::type>;

private:
  entity_mgr_t      m_entity_mgr;
  container_tuple_t m_containers;

};


} // namespace heim


#endif // HEIM_REGISTRY_HPP
