#ifndef HEIM_CREATION_INFO_HPP
#define HEIM_CREATION_INFO_HPP

#include <cstddef>
#include <type_traits>
#include "type_sequence.hpp"

namespace heim
{
template<typename TSeq>
struct component_info;

template<
    typename    Component,
    std::size_t PageSize,
    typename    Allocator>
struct component_info<
    type_sequence<
        Component,
        std::integral_constant<std::size_t, PageSize>,
        Allocator>>
{
public:
  using type
  = type_sequence<
      Component,
      std::integral_constant<std::size_t, PageSize>,
      Allocator>;

public:
  using component_t
  = Component;

  constexpr static std::size_t
  page_size_v = PageSize;

  using allocator_t
  = Allocator;

};


template<
    typename    Component,
    std::size_t PageSize,
    typename    Allocator>
using declare_component_info
= component_info<
    type_sequence<
        Component,
        std::integral_constant<std::size_t, PageSize>,
        Allocator>>;


template<typename TSeq>
struct is_component_info
  : std::false_type
{ };

template<
    typename    Component,
    std::size_t PageSize,
    typename    Allocator>
struct is_component_info<
    type_sequence<
        Component,
        std::integral_constant<std::size_t, PageSize>,
        Allocator>>
  : std::true_type
{ };

template<typename TSeq>
constexpr inline bool
is_component_info_v = is_component_info<TSeq>::value;




template<typename TSeq>
struct sync_info;

template<typename ...TSeqs>
struct sync_info<type_sequence<TSeqs ...>>
{
private:
  static_assert(
      (is_component_info_v<TSeqs> && ...),
      "heim::sync_info<type_sequence<TSeqs ...>>: "
          "(is_component_info_v<TSeqs> && ...)");

  static_assert(
      type_sequence<typename component_info<TSeqs>::component_t ...>::unique_v,
      "heim::sync_info<type_sequence<TSeqs ...>>: type_sequence<"
          "typename component_info<TSeqs>::component_t ...>::unique_v");

public:
  using type
  = type_sequence<TSeqs ...>;

private:
  template<typename TSeq>
  struct to_component
  {
    using type
    = typename component_info<TSeq>::component_t;

  };

  using component_seq
  = typename type
      ::template map_t<to_component>;

public:
  template<
      typename    Component,
      std::size_t PageSize,
      typename    Allocator>
  using sync
  = sync_info<typename type::template extend_t<
      typename declare_component_info<Component, PageSize, Allocator>::type>>;


  template<typename Component>
  using syncs
  = typename component_seq::template contains<Component>;

  template<typename Component>
  constexpr static bool
  syncs_v = syncs<Component>::value;


  template<typename Component>
  using info_of
  = component_info<typename type
      ::template get_t<component_seq::template index_v<Component>>>;

};


template<typename TSeq>
struct is_sync_info
  : std::false_type
{ };

template<typename ...TSeqs>
struct is_sync_info<type_sequence<TSeqs ...>>
  : std::bool_constant<(is_component_info_v<TSeqs> && ...)>
{ };

template<typename TSeq>
constexpr inline bool
is_sync_info_v = is_sync_info<TSeq>::value;


using declare_sync_info
= sync_info<type_sequence<>>;



template<typename TSeq>
struct creation_info;


namespace detail
{
template<
    typename    Component,
    typename ...TSeqs>
struct creation_info_sync_of;

template<typename Component>
struct creation_info_sync_of<Component>
{
  using type
  = void;

};

template<
    typename    Component,
    typename    First,
    typename ...Rest>
struct creation_info_sync_of<
    Component,
    First,
    Rest ...>
{
private:
  static_assert(
      is_sync_info_v<First>,
      "heim::detail::creation_info_sync_of<Component, First, Rest ...>: "
          "is_sync_info_v<First>.");
  static_assert(
      (is_sync_info_v<Rest> && ...),
      "heim::detail::creation_info_sync_of<Component, First, Rest ...>: "
          "(is_sync_info_v<Rest> && ...).");

public:
  using type
  = std::conditional_t<
      sync_info<First>::template syncs_v<Component>,
      First,
      typename creation_info_sync_of<Component, Rest ...>::type>;

};

}


template<typename ...TSeqs>
struct creation_info<
    type_sequence<TSeqs ...>>
{
private:
  static_assert(
      (is_sync_info_v<TSeqs> && ...),
      "heim::detail::creation_info_sync_of<Component, First, Rest ...>: "
          "(is_sync_info_v<Rest> && ...).");

public:
  using type
  = type_sequence<TSeqs ...>;

private:
  template<typename Component>
  using sync_of
  = detail::creation_info_sync_of<Component, TSeqs ...>;


  template<typename Component>
  constexpr static std::size_t
  count_v = (0 + ... + (sync_info<TSeqs>::template syncs_v<Component> ? 1 : 0));

  template<typename ...Components>
  constexpr static bool
  can_sync_v
  =   type_sequence<Components ...>::unique_v
   && ((count_v<Components> == 1) && ...)
   && ((sync_of<Components>::type::size_v == 1) && ...);

public:
  template<
      typename    Component,
      std::size_t PageSize,
      typename    Allocator>
  using component
  = creation_info<typename type
      ::template extend_t<
          typename declare_sync_info
              ::sync<Component, PageSize, Allocator>::type>>;


  template<typename ...Components>
  using sync
  = std::conditional_t<
    can_sync_v<Components ...>,
    creation_info<typename type
      ::template difference_t<
          typename type_sequence<
              typename sync_of<Components>::type ...>::unique_t>
      ::template extend_t<
          typename sync_info<type_sequence<typename sync_of<Components>
              ::template info_of<Components>::type ...>>::type>>,
    creation_info>;



  using component_info_seq
  = typename type::flat_t;

};


using declare_creation_info
= creation_info<type_sequence<>>;


}


#endif // HEIM_CREATION_INFO_HPP
