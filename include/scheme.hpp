#ifndef HEIM_SCHEME_HPP
#define HEIM_SCHEME_HPP

#include <cstddef>
#include <type_traits>
#include "lib/type_sequence.hpp"

namespace heim
{
/*!
 * @brief The representation of component type information in Heim.
 *
 * @tparam C        The type of the component.
 * @tparam PageSize The size of each internal page for the component's
 *   container.
 * @tparam Alloc    The type of the allocator used for the component's
 *   container.
 */
template<
    typename    C,
    std::size_t PageSize,
    typename    Alloc>
using component_scheme
= type_sequence<
    C,
    std::integral_constant<std::size_t, PageSize>,
    Alloc>;


/*!
 * @brief Checks whether the given type sequence @code TSeq@endcode is a valid
 *   component scheme.
 *
 * @tparam TSeq The type sequence to check for.
 */
template<typename TSeq>
struct is_component_scheme
  : std::false_type
{ };

template<
    typename    C,
    std::size_t PageSize,
    typename    Alloc>
struct is_component_scheme<type_sequence<
    C,
    std::integral_constant<std::size_t, PageSize>,
    Alloc>>
  : std::true_type
{ };

template<typename TSeq>
inline constexpr bool
is_component_scheme_v = is_component_scheme<TSeq>::value;


/*!
 * @brief The common interface to use component schemes.
 *
 * @tparam Scheme The component scheme to get the traits of.
 */
template<typename Scheme>
struct component_scheme_traits
{
  static_assert(
      is_component_scheme_v<Scheme>,
      "heim::detail::component_scheme_traits<Scheme>: "
          "is_component_scheme_v<Scheme>;");

public:
  //! @brief The component scheme type.
  using type
  = Scheme;


  //! @brief The component of the component scheme.
  using component
  = typename Scheme
      ::template get<0>;

  //! @brief The size of each internal page for the component's container.
  static constexpr std::size_t
  page_size = Scheme
      ::template get<1>::value;

  //! @brief The type of the allocator used for the component's container.
  using allocator
  = typename Scheme
      ::template get<2>;

};



/*!
 * @brief Checks whether the given type sequence @code TSeq@endcode is a valid
 *   sync scheme.
 *
 * @tparam TSeq The type sequence to check for.
 *
 * @details A sync scheme is a type sequence of component schemes for a set of
 *   unique components.
 */
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


/*!
 * @brief The common interface to use sync schemes.
 *
 * @tparam Scheme The sync scheme to get the traits of.
 */
template<typename Scheme>
struct sync_scheme_traits
{
  static_assert(
      is_sync_scheme_v<Scheme>,
      "heim::detail::sync_scheme_traits<Scheme>: "
          "is_sync_scheme_v<Scheme>;");

public:
  //! @brief The sync scheme type.
  using type
  = Scheme;

private:
  //! @cond INTERNAL

  /*!
   * @brief The component scheme @code CScheme@endcode 's component.
   */
  template<typename CScheme>
  struct to_component
  {
  public:
    using type
    = typename component_scheme_traits<CScheme>::component;

  };

  //! @endcond

public:
  //! @brief The sequence of component types involved in the sync scheme.
  using component_sequence
  = typename Scheme::template map<to_component>;

  /*!
   * @brief The component scheme in the sync scheme of the component type
   *   @code C@endcode.
   *
   * @tparam C The component type to get the scheme of.
   */
  template<typename C>
  using component_scheme
  = typename Scheme::template get<component_sequence::template index<C>>;

};



/*!
 * @brief Checks whether the given type sequence @code TSeq@endcode is a valid
 *   scheme.
 *
 * @tparam TSeq The type sequence to check for.
 *
 * @details A scheme is a type sequence of sync schemes for a set of unique
 *   components.
 */
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


namespace detail
{
template<
    typename Scheme,
    typename C>
struct scheme_get_sync_scheme
{
  static_assert(
      is_scheme_v<Scheme>,
      "heim::detail::scheme_get_scheme<Scheme, C>: "
          "is_scheme_v<Scheme>;");

private:
  template<typename SScheme>
  struct has_component
    : std::bool_constant<sync_scheme_traits<SScheme>
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
    typename    Scheme,
    typename ...Cs>
struct scheme_declare_sync
{
  static_assert(
      is_scheme_v<Scheme>,
      "heim::detail::scheme_declare_scheme<Scheme, Cs ...>: "
          "is_scheme_v<Scheme>;");

  static_assert(
      ((scheme_get_sync_scheme<Scheme, Cs>::size == 1) && ...),
      "heim::detail::scheme_declare_scheme<Scheme, Cs ...>: "
          "((scheme_get_sync_scheme<Scheme, Cs>::size == 1) && ...);");

public:
  using type
  = typename Scheme
      ::template difference<
          type_sequence<
              scheme_get_sync_scheme<Scheme, Cs> ...>>
      ::template extend<
          type_sequence<
              typename scheme_get_sync_scheme<Scheme, Cs>
                  ::template get<0> ...>>;

};


} // namespace detail


template<typename Scheme>
struct scheme_traits
{
private:
  static_assert(
      is_scheme_v<Scheme>,
      "heim::detail::scheme_traits<Scheme>: "
          "is_scheme_v<Scheme>;");

public:
  //! @brief The scheme type.
  using type
  = Scheme;


  /*!
   * @brief The sync scheme associated to the component type @code C@endcode.
   *
   * @tparam C The component type to get the sync scheme of.
   */
  template<typename C>
  using get_sync_scheme
  = detail::scheme_get_sync_scheme<Scheme, C>;


  /*!
   * @brief The scheme with the added component scheme described by the
   *   component type @code C@endcode, its container's page size
   *   @code PageSize@endcode and its allocator @code Alloc@endcode.
   *
   * @tparam C        The type of the component.
   * @tparam PageSize The size of each internal page for the component's
   *   container.
   * @tparam Alloc    The type of the allocator used for the component's
   *   container.
   */
  template<
      typename    C,
      std::size_t PageSize,
      typename    Alloc>
  using declare_component
  = scheme_traits<
      typename type
          ::template extend<
              type_sequence<component_scheme<C, PageSize, Alloc>>>>;

  /*!
   * @brief The scheme with component types @code First@endcode,
   *   @code Second@endcode and @code Rest ...@endcode synced together.
   *
   * @tparam First  The first  component type to sync.
   * @tparam Second The second component type to sync.
   * @tparam Rest   The rest of the component types to sync.
   *
   * @note Component types can only be synced once.
   */
  template<
      typename    First,
      typename    Second,
      typename ...Rest>
  using declare_sync
  = scheme_traits<
      typename detail::scheme_declare_sync<
          Scheme,
          First, Second, Rest ...>::type>;

};


} // namespace heim


#endif //HEIM_SCHEME_HPP
