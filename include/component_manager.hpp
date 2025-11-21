#ifndef HEIM_COMPONENT_MANAGER_HPP
#define HEIM_COMPONENT_MANAGER_HPP

#include <cstddef>
#include <memory>
#include <type_traits>
#include "lib/index_map.hpp"
#include "lib/type_sequence.hpp"
#include "configuration.hpp"
#include "scheme.hpp"

namespace heim
{
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
    typename Alloc  = default_allocator_t<Index>,
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

  using alloc_traits_t
  = std::allocator_traits<allocator_type>;


  /*!
   * @brief The allocator to use in the manager for component @code C@endcode.
   *
   * @tparam C The component whose allocator to determine.
   */
  template<typename C>
  struct component_allocator
  {
  private:
    static constexpr bool
    is_redefined = !std::is_same_v<
        default_allocator_t<C>,
        typename component_traits<C>::allocator_type>;

  public:
    using type
    = std::conditional_t<
        is_redefined,
        typename component_traits<C>
            ::allocator_type,
        typename alloc_traits_t
            ::template rebind_alloc<C>>;

  };

  template<typename C>
  using component_allocator_t
  = typename component_allocator<C>::type;


  //! @brief The tuple of component containers to manage.
  struct container_tuple
  {
  private:
    template<typename CScheme>
    struct to_container
    {
    private:
      using component_t
      = typename component_scheme_traits<CScheme>::component;

    public:
      using type
      = index_map<
          index_type,
          component_t,
          component_scheme_traits<CScheme>::page_size,
          typename std::allocator_traits<component_allocator_t<component_t>>
              ::template rebind_alloc<
                  std::pair<index_type const, component_t>>>;

    };

  public:
    using type
    = scheme_type
        ::flat
        ::template map<to_container>
        ::tuple;

  };

  using container_tuple_t
  = container_tuple::type;

  //! @endcond

public:
  /*!
   * @brief The manager with the added component scheme described by the
   *   component type @code C@endcode, its container's page size
   *   @code PageSize@endcode and its allocator @code Alloc@endcode.
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
      typename    CAlloc   = component_allocator_t<C>>
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
  noexcept
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
  noexcept(noexcept(m_containers = std::move(other.m_containers)))
  {
    m_containers = std::move(other.m_containers);
    return *this;
  }

};


}


#endif // HEIM_COMPONENT_MANAGER_HPP
