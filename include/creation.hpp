#ifndef HEIM_CREATION_HPP
#define HEIM_CREATION_HPP

#include <cstddef>
#include <memory>
#include <type_traits>
#include "lib/index_map.hpp"
#include "lib/type_sequence.hpp"
#include "lib/utility.hpp"

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
  using get_component_scheme
  = typename TSeq::template get<component_sequence::template index<T>>;

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
                ::flat
                ::is_unique>
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
        ::template contains<T>;

  };

public:
  using type
  = typename TSeq
      ::template filter<has_component>
      ::template get<0>;

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


  template<typename T>
  using get_sync_scheme
  = typename creation_scheme_get_sync_scheme<type, T>::type;

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
        "heim::detail::creation_scheme_traits<TSeq>"
            "::sync_components_wrapper<First, Second, Rest ...>: "
                "heim::detail::creation_scheme_traits<TSeq>::"
                    "can_sync<First, Second, Rest...>;");

  public:
    using type
    = creation_scheme_traits<typename type
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
  = creation_scheme_traits<
      typename type::template extend<
          type_sequence<declare_component_scheme<T, PageSize, Alloc>>>>;

  template<
      typename    First,
      typename    Second,
      typename ...Rest>
  using sync_components
  = sync_components_wrapper<First, Second, Rest ...>::type;

};


} // namespace detail


template<typename T, typename = redefine_tag>
struct component_traits
{
  static constexpr std::size_t
  page_size = 4096;

  template<typename Index>
  using allocator_type
  = std::allocator<std::pair<Index const, T>>;

};



template<
    typename Index,
    typename Alloc  = std::allocator<Index>,
    typename Scheme = type_sequence<>>
class creation
{
private:
  static_assert(
      detail::is_creation_scheme_v<Scheme>,
      "heim::creation<Index, Alloc, Scheme>: "
          "detail::is_creation_scheme_v<Scheme>;");

public:
  using index_type     = Index;
  using allocator_type = Alloc;
  using scheme_type    = Scheme;

private:
  using alloc_traits_t
  = std::allocator_traits<allocator_type>;

  template<typename T>
  using default_alloc_t
  = std::conditional_t<
      std::is_same_v<
          typename alloc_traits_t
              ::template rebind_alloc<std::pair<Index const, T>>,
          typename component_traits<T>
              ::template allocator_type<Index>>,
      typename alloc_traits_t
          ::template rebind_alloc<std::pair<Index const, T>>,
      typename component_traits<T>
          ::template allocator_type<Index>>;

public:
  template<
      typename    T,
      std::size_t PageSize = component_traits<T>::page_size,
      typename    TAlloc   = default_alloc_t<T>>
  using component
  = creation<
      index_type,
      allocator_type,
      typename detail::creation_scheme_traits<scheme_type>
          ::template add_component<T, PageSize, TAlloc>
          ::type>;

  template<
      typename    First,
      typename    Second,
      typename ...Rest>
  using sync
  = creation<
      index_type,
      allocator_type,
      typename detail::creation_scheme_traits<scheme_type>
          ::template sync_components<First, Second, Rest ...>
          ::type>;

private:
  template<typename TSeq>
  struct to_container
  {
  private:
    static_assert(
        detail::is_component_scheme_v<TSeq>,
        "heim::creation<Index, Scheme>::to_container<TSeq>: "
            "detail::is_component_scheme_v<TSeq>;");

  public:
    using type
    = index_map<
        index_type,
        typename detail::component_scheme_traits<TSeq>::component,
        detail::component_scheme_traits         <TSeq>::page_size,
        typename detail::component_scheme_traits<TSeq>::allocator>;

  };


  using container_tuple
  = typename scheme_type
      ::flat
      ::template map<to_container>
      ::to_tuple;

private:
  container_tuple m_containers;

};


} // namespace heim


#endif // HEIM_CREATION_HPP
