#ifndef HEIM_CREATION_INFO_HPP
#define HEIM_CREATION_INFO_HPP

#include <type_traits>
#include "lib/type_sequence.hpp"

namespace heim
{
/*!
 * Determines whether @code TSeq@endcode is a valid component_info type.
 *
 * @tparam TSeq The type to check for.
 */
template<typename TSeq>
struct is_component_info
  : std::false_type
{ };

template<
    typename    T,
    std::size_t PageSize,
    typename    Alloc>
struct is_component_info<
    type_sequence<T, std::integral_constant<std::size_t, PageSize>, Alloc>>
  : std::true_type
{ };

template<typename TSeq>
inline constexpr bool
is_component_info_v = is_component_info<TSeq>::value;


/*!
 * @brief An adapter useful to access properties of the component_info type
 *   @code TSeq@endcode.
 *
 * @tparam TSeq The component_info type whose properties to access.
 */
template<typename TSeq>
struct component_info_traits
{
private:
  static_assert(
      is_component_info_v<TSeq>,
      "heim::component_info_traits<TSeq>: is_component_info_v<TSeq>;");

public:
  //! @brief The component_info type.
  using type
  = TSeq;

public:
  //! @brief The type of component.
  using component_t
  = typename type::template get_t<0>;

  //! @brief The size of each page in its assigned container.
  static constexpr std::size_t
  page_size_v = type::template get_t<1>::value;

  //! @brief The allocator type used in its assigned container.
  using allocator_t
  = typename type::template get_t<2>;

};


/*!
 * @brief A simple way to make a valid component_info type.
 *
 * @tparam T        The type of component.
 * @tparam PageSize The size of each page in its assigned container.
 * @tparam Alloc    The allocator type used in its assigned container.
 */
template<
    typename    T,
    std::size_t PageSize,
    typename    Alloc>
using make_component_info
= type_sequence<T, std::integral_constant<std::size_t, PageSize>, Alloc>;



/*!
 * Determines whether @code TSeq@endcode is a valid sync_info type (i.e a
 *   sequence of unique component_info types).
 *
 * @tparam TSeq The type to check for.
 */
template<typename TSeq>
struct is_sync_info
  : std::false_type
{ };

template<typename ...TSeqs>
struct is_sync_info<
    type_sequence<TSeqs ...>>
  : std::bool_constant<
        (is_component_info_v<TSeqs> && ...)
     && type_sequence<typename component_info_traits<TSeqs>::component_t ...>
            ::unique_v>
{ };

template<typename TSeq>
inline constexpr bool
is_sync_info_v = is_sync_info<TSeq>::value;


/*!
 * @brief An adapter useful to access properties of the sync_info type
 *   @code TSeq@endcode.
 *
 * @tparam TSeq The sync_info type whose properties to access.
 */
template<typename TSeq>
struct sync_info_traits
{
private:
  static_assert(
      is_sync_info_v<TSeq>,
      "heim::sync_info_traits<TSeq>: is_sync_info_v<TSeq>;");

public:
  //! @brief The sync_info type.
  using type
  = TSeq;

private:
  template<typename USeq>
  struct to_component
  {
  public:
    using type
    = typename component_info_traits<USeq>::component_t;

  };

public:
  /*!
   * @brief The traits for the sync_info type with the synced component_info
   *   type @code USeq@endcode.
   *
   * @tparam USeq The component_info to sync with.
   */
  template<typename USeq>
  using sync_t
  = typename sync_info_traits<
      typename type::template extend_t<USeq>>::type;


  /*!
   * @brief The sequence of corresponding component types in each
   *   component_info type.
   */
  using component_sequence_t
  = typename type::template map_t<to_component>;


  /*!
   * @brief Whether @code Component@endcode has a component_info type in the
   *   sync_info type.
   *
   * @tparam Component The type whose component_info to check for.
   */
  template<typename Component>
  static constexpr bool
  syncs_v = component_sequence_t::template contains_v<Component>;


  /*!
   * @brief The corresponding component_info type of @code Component@endcode in
   *   the sync_info type.
   *
   * @tparam Component The type of component to access the component_info type
   *   of.
   */
  template<typename Component>
  using component_info_t
  = typename type
      ::template get_t<component_sequence_t::template index_v<Component>>;

};



/*!
 * Determines whether @code TSeq@endcode is a valid creation_info type (i.e a
 *   sequence of sync_info types with no same component types).
 *
 * @tparam TSeq The type to check for.
 */
template<typename TSeq>
struct is_creation_info
  : std::false_type
{ };

template<typename ...TSeqs>
struct is_creation_info<type_sequence<TSeqs ...>>
  : std::bool_constant<
        (is_sync_info_v<TSeqs> && ...)
     && type_sequence<typename sync_info_traits<TSeqs>
            ::component_sequence_t ...>::flat_t::unique_v>
{ };

template<typename TSeq>
inline constexpr bool
is_creation_info_v = is_creation_info<TSeq>::value;


namespace detail
{
template<typename TypeSeq, typename Component>
struct creation_info_traits_sync_info;

template<typename Component>
struct creation_info_traits_sync_info<
    type_sequence<>, Component>
{
public:
  using type
  = type_sequence<>;

};

template<typename First, typename ...Rest, typename Component>
struct creation_info_traits_sync_info<
    type_sequence<First, Rest ...>, Component>
{
private:
  static_assert(
     is_creation_info_v<type_sequence<First, Rest...>>,
     "heim::creation_info_traits<TSeq>: "
         "is_creation_info_v<type_sequence<First, Rest...>>;");

public:
  using type
  = std::conditional_t<
      sync_info_traits<First>::template syncs_v<Component>,
      First,
      typename creation_info_traits_sync_info<
          type_sequence<Rest ...>,
          Component>::type>;

};

template<typename TypeSeq, typename Component>
using creation_info_traits_sync_info_t
= typename creation_info_traits_sync_info<TypeSeq, Component>::type;


}


/*!
 * @brief An adapter useful to access properties of the creation_info type
 *   @code TSeq@endcode.
 *
 * @tparam TSeq The creation_info type whose properties to access.
 */
template<typename TSeq>
struct creation_info_traits
{
private:
  static_assert(
      is_creation_info_v<TSeq>,
      "heim::creation_info_traits<TSeq>: is_creation_info_v<TSeq>;");

public:
  //! @brief The creation_info type.
  using type
  = TSeq;

private:
  template<typename Component>
  using sync_info_t
  = detail::creation_info_traits_sync_info_t<type, Component>;


  template<typename Component>
  static constexpr bool
  can_sync_v = sync_info_t<Component>::size_v == 1;

public:
  /*!
   * @brief Determines the traits for the creation_type with an added
   *   component_info.
   *
   * @tparam USeq The component_info type to add.
   */
  template<typename USeq>
  using add_t
  = creation_info_traits<typename type
      ::template extend_t<type_sequence<USeq>>>;


  /*!
   * @brief Determines the traits for the creation_type with the component
   *   types @code Components ...@endcode synced.
   *
   * @tparam Components The type of components to sync together.
   *
   * @note If one of the components is already synced or does not exist in the
   *   creation_info type, the creation_type is not modified.
   */
  template<typename ...Components>
  using sync_t
  = std::conditional_t<
      ((can_sync_v<Components>) && ...),
      creation_info_traits<typename type
          ::template difference_t<
              typename type_sequence<sync_info_t<Components> ...>::unique_t>
          ::template extend_t<
              type_sequence<typename sync_info_traits<typename type::flat_t>
                  ::template component_info_t<Components> ...>>>,
      creation_info_traits>;

};


}


#endif // HEIM_CREATION_INFO_HPP
