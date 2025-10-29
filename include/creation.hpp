#ifndef HEIM_CREATION_HPP
#define HEIM_CREATION_HPP

#include <memory>
#include "lib/type_sequence.hpp"

namespace heim
{
namespace detail
{
template<typename TSeq>
struct is_component_scheme
  : std::false_type
{};

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
  = TSeq::template get_t<0>;

  static constexpr std::size_t
  page_size = TSeq::template get_t<1>::value;

  using allocator
  = TSeq::template get_t<2>;

};

template<
    typename    T,
    std::size_t PageSize,
    typename    Alloc>
using declare_component_scheme
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
            ::unique_v>
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
    = component_scheme_traits<USeq>::component;

  };

public:
  using component_sequence
  = TSeq::template map_t<to_component>;

  template<typename T>
  using get_component_scheme
  = TSeq::template get_t<component_sequence::template index_v<T>>;

};



template<typename TSeq>
struct is_creation_scheme
  : std::false_type
{ };

template<typename ...TSeqs>
struct is_creation_scheme<type_sequence<TSeqs ...>>
  : std::bool_constant<
        (is_sync_scheme_v<TSeqs> && ...)
     && type_sequence<
            typename sync_scheme_traits<TSeqs>::component_sequence ...>
                ::flat_t
                ::unique_v>
{ };

template<typename TSeq>
inline constexpr bool
is_creation_scheme_v = is_creation_scheme<TSeq>::value;


template<
    typename TSeq,
    typename T>
struct creation_scheme_get_sync_scheme
{
private:
  static_assert(
      is_creation_scheme_v<TSeq>,
      "heim::detail::creation_scheme_get_sync_scheme<TSeq, T>: "
          "heim::detail::is_creation_scheme_v<TSeq>;");

private:
  template<typename USeq>
  struct has_component
  {
  private:
    static_assert(
        is_sync_scheme_v<USeq>,
        "heim::detail::creation_scheme_get_sync_scheme<TSeq, T>"
            "::has_component<USeq>: "
                "is_sync_scheme_v<USeq>;");

  public:
    static constexpr bool
    value
    = sync_scheme_traits<USeq>
        ::component_sequence
        ::template contains_v<T>;

  };

public:
  using type
  = TSeq
      ::template filter_t<has_component>
      ::template get_t<0>;

};


template<typename TSeq>
class creation_scheme_traits
{
private:
  static_assert(
      is_creation_scheme_v<TSeq>,
      "heim::detail::creation_scheme_traits<TSeq>: "
          "is_creation_scheme_v<TSeq>;");

public:
  using type
  = TSeq;

private:
  template<typename T>
  using get_sync_scheme
  = creation_scheme_get_sync_scheme<TSeq, T>::type;


  template<typename ...Ts>
  static constexpr bool
  can_sync
  = ((get_sync_scheme<Ts>::size_v == 1) && ...);

public:
  template<
      typename    T,
      std::size_t PageSize,
      typename    Alloc>
  using add_component
  = creation_scheme_traits<
      typename type::template extend_t<
          type_sequence<declare_component_scheme<T, PageSize, Alloc>>>>;

  template<
      typename    First,
      typename    Second,
      typename ...Rest>
  requires (can_sync<First, Second, Rest ...>)
  using sync_components
  = creation_scheme_traits<typename type
      ::template difference_t<type_sequence<
          get_sync_scheme<First >,
          get_sync_scheme<Second>,
          get_sync_scheme<Rest  > ...>>
      ::template extend_t    <type_sequence<
          typename get_sync_scheme<First >::template get_t<0>,
          typename get_sync_scheme<Second>::template get_t<0>,
          typename get_sync_scheme<Rest  >::template get_t<0> ...>>>;

};


} // namespace detail


template<
    typename Entity,
    typename Scheme = type_sequence<>>
class creation
{
private:
  static_assert(
      detail::is_creation_scheme_v<Scheme>,
      "heim::creation<Entity, Scheme>: detail::is_creation_scheme_v<Scheme>;");

public:
  using entity_type = Entity;
  using scheme_type = Scheme;

public:
  template<
      typename    T,
      std::size_t PageSize = 4096,
      typename    Alloc    = std::allocator<T>>
  using component
  = creation<
      entity_type,
      typename detail::creation_scheme_traits<scheme_type>
          ::template add_component<T, PageSize, Alloc>
          ::type>;

  template<
      typename    First,
      typename    Second,
      typename ...Rest>
  using sync
  = creation<
      entity_type,
      typename detail::creation_scheme_traits<scheme_type>
          ::template sync_components<First, Second, Rest ...>
          ::type>;

};


} // namespace heim


#endif // HEIM_CREATION_HPP
