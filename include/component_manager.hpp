#ifndef HEIM_COMPONENT_MANAGER_HPP
#define HEIM_COMPONENT_MANAGER_HPP

#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>
#include "lib/index_map.hpp"
#include "lib/type_sequence.hpp"
#include "configuration.hpp"
#include "scheme.hpp"

namespace heim
{
namespace detail
{
template<
    typename CScheme,
    typename Index>
struct component_scheme_to_container
{
  static_assert(
      is_component_scheme_v<CScheme>,
      "heim::detail::component_scheme_to_container<CScheme, Index>: "
          "is_component_scheme_v<CScheme>;");

private:
  using scheme_traits_t
  = component_scheme_traits<CScheme>;

public:
  using type
  = index_map<
      Index,
      typename scheme_traits_t::component,
      scheme_traits_t         ::page_size,
      typename std::allocator_traits<typename scheme_traits_t::allocator>
          ::template rebind_alloc<
              std::pair<Index const, typename scheme_traits_t::component>>>;

};


} // namespace detail


/*!
 * @brief The global manager for a set of component maps.
 *
 * @tparam Index  The type of index for each component map.
 * @tparam Alloc  The default allocator type for each component map.
 * @tparam Scheme The type sequence describing the organisation and syncing
 *   behaviour of the component maps.
 */
template<
    typename Index,
    typename Alloc  = allocator_for_t<Index>,
    typename Scheme = type_sequence<>>
class component_manager
{
  static_assert(
      std::is_same_v<
          Index,
          typename std::allocator_traits<Alloc>::value_type>,
      "heim::component_manager<Index, Alloc, Scheme>: "
          "std::is_same_v<"
              "Index,"
              "typename std::allocator_traits<Alloc>::value_type>;");
  static_assert(
      is_scheme_v<Scheme>,
      "heim::component_manager<Index, Alloc, Scheme>: "
          "is_scheme_v<Scheme>;");

public:
  //! @brief The type of index for each component map.
  using index_type
  = Index;

  //! @brief The default allocator type for each component map.
  using allocator_type
  = Alloc;

  //! @brief The scheme type describing the organisation of component types.
  using scheme_type
  = Scheme;

private:
  //! @cond INTERNAL

  //! @brief The interface to access the traits of the allocator type.
  using alloc_traits_t
  = std::allocator_traits<allocator_type>;


  /*!
   * @brief The allocator to propose by default for the component type
   *   @code C@endcode.
   *
   * @tparam C The component to get the default allocator.
   *
   * @details Takes into account if the external default allocator for
   *   @code C@endcode has been redefined, otherwise defaults to the allocator
   *   type of the manager.
   */
  template<typename C>
  struct default_allocator_for
  {
  private:
    static constexpr bool
    is_redefined
    = !std::is_same_v<
        default_allocator<>::template type_for<C>,
        typename component_traits<C>::allocator>;

  public:
    using type
    = std::conditional_t<
        is_redefined,
        typename component_traits<C>::allocator,
        typename alloc_traits_t
            ::template rebind_alloc<C>>;

  };

  template<typename C>
  using default_allocator_for_t
  = typename default_allocator_for<C>::type;



  using component_scheme_sequence
  = typename scheme_type::flat;


  template<typename CScheme>
  using to_container
  = detail::component_scheme_to_container<CScheme, index_type>;

  using container_tuple_t
  = typename component_scheme_sequence
      ::template map<to_container>
      ::tuple;


  template<typename C>
  static constexpr std::size_t
  component_index_v
  = scheme_traits<scheme_type>::template component_index<C>;

  template<typename C>
  static constexpr std::size_t
  sync_index_v
  = scheme_traits<scheme_type>::template sync_index<C>;


  template<typename C>
  using container_for_t
  = std::tuple_element_t<component_index_v<C>, container_tuple_t>;

  //! @endcond

public:
  /*!
   * @brief The manager with the added component scheme described by the
   *   component type @code C@endcode, its container's page size
   *   @code PageSize@endcode and its allocator @code CAlloc@endcode.
   *
   * @tparam C        The type of the component.
   * @tparam PageSize The size of each internal page for the component's
   *   container.
   * @tparam CAlloc   The type of the allocator used for the component's
   *   container.
   */
  template<
      typename    C,
      std::size_t PageSize = default_page_size_v<C>,
      typename    CAlloc   = typename default_allocator_for<C>::type>
  using declare_component
  = component_manager<
      index_type,
      allocator_type,
      typename scheme_traits<scheme_type>
          ::template declare_component<C, PageSize, CAlloc>
          ::type>;

  /*!
   * @brief The manager with component types @code First@endcode,
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
  = component_manager<
      index_type,
      allocator_type,
      typename scheme_traits<scheme_type>
          ::template declare_sync<First, Second, Rest ...>
          ::type>;

private:
  container_tuple_t m_containers;

public:
  //! @brief Default-constructs the component manager.
  constexpr
  component_manager()
  noexcept(noexcept(container_tuple_t{}))
    : m_containers{}
  { }

  /*!
   * @brief Constructs @c *this to be a copy of @code other@endcode.
   *
   * @param other The other component manager to copy.
   */
  constexpr
  component_manager(component_manager const &other)
  = default;

  /*!
   * @brief Constructs @c *this to be the moved @code other@endcode.
   *
   * @param other The moved component manager.
   */
  constexpr
  component_manager(component_manager &&other)
  noexcept(std::is_nothrow_move_constructible_v<container_tuple_t>)
  = default;


  //! @brief Destroys the component manager.
  constexpr
  ~component_manager()
  noexcept
  = default;


  /*!
   * @brief Assigns @c *this to be a copy of @code other@endcode.
   *
   * @param other The other component manager to copy.
   * @returns @c *this .
   */
  constexpr component_manager &
  operator=(component_manager const &other)
  requires (std::is_copy_assignable_v<container_tuple_t>)
  {
    m_containers = other.m_containers;
    return *this;
  }

  /*!
   * @brief Assigns @c *this to be the moved @code other@endcode.
   *
   * @param other The moved component manager.
   * @returns @c *this .
   */
  constexpr component_manager &
  operator=(component_manager &&other)
  noexcept(std::is_nothrow_move_assignable_v<container_tuple_t>)
  {
    m_containers = std::move(other.m_containers);
    return *this;
  }



  /*!
   * @brief Swaps the contents of @c *this and @code other@endcode.
   *
   * @param other The other component manager whose contents to swap.
   */
  constexpr void
  swap(component_manager &other)
  noexcept(noexcept(m_containers.swap(other.m_containers)))
  {
    m_containers.swap(other.m_containers);
  }

  /*!
   * @brief Swaps the contents of @code lhs@endcode and @code rhs@endcode.
   *
   * @param lhs The first  component manager whose contents to swap.
   * @param rhs The second component manager whose contents to swap.
   */
  friend constexpr void
  swap(component_manager &lhs, component_manager &rhs)
  noexcept(noexcept(lhs.swap(rhs)))
  {
    lhs.swap(rhs);
  }



  /*!
   * @brief Returns the container related to the component type
   *   @code C@endcode in the manager.
   *
   * @tparam C The component type whose container to get.
   * @returns The container related to the component type @code C@endcode in
   *   the manager.
   */
  template<typename C>
  [[nodiscard]]
  constexpr container_for_t<C> &
  container()
  noexcept
  {
    return std::get<component_index_v<C>>(m_containers);
  }

  /*!
   * @brief Returns the const container related to the component type
   *   @code C@endcode in the manager.
   *
   * @tparam C The component type whose container to get.
   * @returns The const container related to the component type @code C@endcode
   *   in the manager.
   */
  template<typename C>
  [[nodiscard]]
  constexpr container_for_t<C> const &
  container() const
  noexcept
  {
    return std::as_const(std::get<component_index_v<C>>(m_containers));
  }



  template<
      typename    C,
      typename ...Args>
  constexpr auto
  emplace(index_type const idx, Args &&...args)
  requires (std::constructible_from<C, Args ...>)
  {
    auto r = container<C>().emplace(idx, std::forward<Args>(args)...);
    // TODO: synchronize
    return r;
  }


  template<
      typename    C,
      typename ...Args>
  constexpr auto
  emplace_or_assign(index_type const idx, Args &&...args)
  requires (
      std::constructible_from<C, Args ...>
   && std::is_move_assignable_v<C>)
  {
    auto r = container<C>().emplace_or_assign(idx, std::forward<Args>(args)...);
    // TODO: synchronize
    return r;
  }

};


}


#endif // HEIM_COMPONENT_MANAGER_HPP
