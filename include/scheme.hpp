#ifndef HEIM_SCHEME_HPP
#define HEIM_SCHEME_HPP

#include <cstddef>
#include <type_traits>
#include "lib/type_sequence.hpp"
#include "configuration.hpp"

namespace heim
{
template<typename T>
struct is_component_scheme
  : std::false_type
{ };

template<typename TSeq>
inline constexpr bool
is_component_scheme_v = is_component_scheme<TSeq>::value;

template<
  typename    C,
  std::size_t PageSize,
  typename    CAlloc>
struct is_component_scheme<
    type_sequence<C, std::integral_constant<std::size_t, PageSize>, CAlloc>>
  : std::true_type
{ };


template<typename CScheme>
struct component_scheme_traits
{
  static_assert(
      is_component_scheme_v<CScheme>,
      "heim::component_scheme_traits<CScheme>: "
          "is_component_scheme_v<CScheme>;");

public:
  using type
  = CScheme;


  using component_type
  = typename CScheme::template get<0>;

  static constexpr std::size_t
  page_size = CScheme::template get<1>::value;

  using allocator_type
  = typename CScheme::template get<2>;


  template<typename Index>
  using container_type
  = typename container_for<component_type>
      ::template type_for<Index, page_size, allocator_type>;

};

template<
    typename    C,
    std::size_t PageSize,
    typename    CAlloc>
using component_scheme
= type_sequence<C, std::integral_constant<std::size_t, PageSize>, CAlloc>;



template<typename T>
struct is_sync_scheme
  : std::false_type
{ };

template<typename T>
inline constexpr bool
is_sync_scheme_v = is_sync_scheme<T>::value;

template<typename ...CSchemes>
struct is_sync_scheme<type_sequence<CSchemes ...>>
  : std::bool_constant<
        (is_component_scheme_v<CSchemes> && ...)
     && type_sequence<
            typename component_scheme_traits<CSchemes>::component_type ...>
            ::is_unique>
{ };


template<typename SScheme>
struct sync_scheme_traits
{ };

template<typename ...CSchemes>
struct sync_scheme_traits<type_sequence<CSchemes ...>>
{
  static_assert(
      is_sync_scheme_v<type_sequence<CSchemes ...>>,
      "heim::sync_scheme_traits<type_sequence<CSchemes ...>>: "
          "is_sync_scheme_v<type_sequence<CSchemes ...>>;");

public:
  using type
  = type_sequence<CSchemes ...>;


  using component_sequence
  = type_sequence<
      typename component_scheme_traits<CSchemes>::component_type ...>;

  template<typename C>
  using component_scheme_of
  = typename type::template get<component_sequence::template index<C>>;

};



template<typename T>
struct is_scheme
  : std::false_type
{ };

template<typename T>
inline constexpr bool
is_scheme_v = is_scheme<T>::value;

template<typename ...SSchemes>
struct is_scheme<type_sequence<SSchemes ...>>
  : std::bool_constant<
        (is_sync_scheme_v<SSchemes> && ...)
     && type_sequence<
            typename sync_scheme_traits<SSchemes>::component_sequence ...>
            ::flat
            ::is_unique>
{ };


namespace detail
{
template<
    typename Scheme,
    typename C>
struct scheme_traits_sync_scheme_of
{
  static_assert(
      is_scheme_v<Scheme>,
      "heim::detail::scheme_traits_sync_scheme_of<Scheme, C>: "
          "is_scheme_v<Scheme>;");

private:
  template<typename SScheme>
  struct has_component
    : std::bool_constant<
          sync_scheme_traits<SScheme>
              ::component_sequence
              ::template contains<C>>
  { };

public:
  using type
  = typename Scheme
      ::template filter<has_component>
      ::template get<0>;

};

template<
    typename Scheme,
    typename C>
using scheme_traits_sync_scheme_of_t
= scheme_traits_sync_scheme_of<Scheme, C>::type;


template<
    typename    Scheme,
    typename ...Cs>
struct scheme_traits_declare_sync
{
  static_assert(
      is_scheme_v<Scheme>,
      "heim::detail::scheme_traits_declare_sync<Scheme, Cs ...>: "
          "is_scheme_v<Scheme>;");
  static_assert(
      ((scheme_traits_sync_scheme_of_t<Scheme, Cs>::size == 1) && ...),
      "heim::detail::scheme_traits_declare_sync<Scheme, Cs ...>: "
          "((scheme_traits_sync_scheme_of<Scheme, Cs>::size == 1) && ...);");

public:
  using type
  = Scheme
      ::template difference<
          type_sequence<
              scheme_traits_sync_scheme_of_t<Scheme, Cs> ...>>
      ::template extend<
          type_sequence<
              typename scheme_traits_sync_scheme_of_t<Scheme, Cs>
              ::template get<0> ...>>;

};


template<
    typename Scheme,
    typename Index>
struct scheme_traits_container_sequence
{
  static_assert(
      is_scheme_v<Scheme>,
      "heim::detail::scheme_traits_container_sequence<Scheme, Index>: "
          "is_scheme_v<Scheme>;");

private:
  template<typename CScheme>
  struct to_container_type
  {
    using type
    = typename component_scheme_traits<CScheme>
        ::template container_type<Index>;

  };

public:
  using type
  = typename Scheme::flat::template map<to_container_type>;

};


} // namespace detail


template<typename Scheme>
struct scheme_traits
{ };

template<typename ...SSchemes>
struct scheme_traits<type_sequence<SSchemes ...>>
{
  static_assert(
      is_scheme_v<type_sequence<SSchemes ...>>,
      "heim::scheme_traits<type_sequence<SSchemes ...>>: "
          "is_scheme_v<type_sequence<SSchemes ...>>;");

public:
  using type
  = type_sequence<SSchemes ...>;



  using component_sequence
  = typename type_sequence<
      typename sync_scheme_traits<SSchemes>::component_sequence ...>
      ::flat;


  template<typename C>
  using sync_scheme_of
  = typename detail::scheme_traits_sync_scheme_of<type, C>::type;

  template<typename C>
  using component_scheme_of
  = typename sync_scheme_traits<sync_scheme_of<C>>
      ::template component_scheme_of<C>;


  template<typename C>
  static constexpr std::size_t
  component_index_of
  = component_sequence::template index<C>;

  template<typename C>
  static constexpr std::size_t
  sync_index_of
  = type::template index<sync_scheme_of<C>>;


  template<
      typename    C,
      std::size_t PageSize,
      typename    CAlloc>
  using declare_component
  = scheme_traits<
      typename type
          ::template extend<
              type_sequence<component_scheme<C, PageSize, CAlloc>>>>;

  template<typename ...Cs>
  using declare_sync
  = scheme_traits<
      typename detail::scheme_traits_declare_sync<type, Cs ...>::type>;

  template<typename Index>
  using container_sequence
  = typename detail::scheme_traits_container_sequence<type, Index>::type;

  template<
      typename Index,
      typename C>
  using container_of
  = typename container_sequence<Index>
      ::template get<component_sequence::template index<C>>;



};


} // namespace heim


#endif // HEIM_SCHEME_HPP
