#ifndef HEIM_COMPONENT_ANY_COMPOSITION_HPP
#define HEIM_COMPONENT_ANY_COMPOSITION_HPP

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <typeinfo>
#include <utility>
#include "composition.hpp"

namespace heim
{
/**
 * @brief The container for a type-erased composition,
 */
class any_composition
{
private:
  /// @cond INTERNAL

  /**
   * @brief The manager for the contained type-erased composition pointer.
   */
  class composition_manager
  {
  public:
    using do_destroy_type = void (*)(void *) noexcept;
    using do_erase_type   = bool (*)(void *, std::uintmax_t const);

  public:
    /**
     * @brief Swaps the contents of @p *this and @code other@endcode.
     *
     * @param other The other composition_manager to swap the contents of.
     */
    constexpr
    void swap(composition_manager &other)
    noexcept
    {
      using std::swap;

      swap(do_destroy, other.do_destroy);
      swap(do_erase  , other.do_erase  );
      swap(type      , other.type      );
    }

  public:
    do_destroy_type       do_destroy;
    do_erase_type         do_erase;

    std::type_info const *type;

  };


  /**
   * @brief Constructs a composition_manager specific to a component_type,
   *     and used in the constructor of the any_composition.
   *
   * @tparam Entity             The type of the entities of the composition.
   * @tparam Component          The type of the components of the composition.
   * @tparam PageSize           The size of each page for the sparse container
   *     of the composition.
   * @tparam ComponentAllocator The allocator for the components of the
   *     composition.
   * @return The composition_manager specific to the suggested composition to
   *     contain.
   */
  template<typename    Entity,
           typename    Component,
           std::size_t PageSize           = 4096,
           typename    ComponentAllocator = std::allocator<Component>>
  requires  std::unsigned_integral<Entity>
        && (PageSize > 0)
  [[nodiscard]]
  constexpr
  composition_manager make_composition_manager() const
  {
    return {
        [](void *ptr)
        noexcept
        {
          delete static_cast<
              composition<Entity, Component, PageSize, ComponentAllocator> *>(
                  ptr);
        },
        [](void *ptr, std::uintmax_t const e)
        -> bool
        {
          auto &composition = *static_cast<
              composition<Entity, Component, PageSize, ComponentAllocator> *>(
                  ptr);

          return composition.erase(static_cast<Entity>(e));
        },
        &typeid(
            composition<Entity, Component, PageSize, ComponentAllocator>)};
  }

  /// @endcond

public:
  constexpr
  any_composition()
  = delete;
  constexpr
  any_composition(any_composition const &)
  = delete;
  constexpr
  any_composition(any_composition &&other)
  noexcept
    : ptr_    {other.ptr_},
      manager_{other.manager_}
  {
    other.ptr_ = nullptr;
  }
  template<typename    Entity,
           typename    Component,
           std::size_t PageSize           = 4096,
           typename    ComponentAllocator = std::allocator<Component>,
           typename ...Args>
  requires  std::unsigned_integral<Entity>
        &&  std::is_copy_constructible_v<Component>
        &&  std::is_copy_assignable_v   <Component>
        && (PageSize > 0)
  explicit
  constexpr
  any_composition(
      std::in_place_type_t<
          composition<Entity, Component, PageSize, ComponentAllocator>>,
      Args &&...args)
    : ptr_    {new composition<
          Entity, Component, PageSize, ComponentAllocator>(
              std::forward<Args>(args)...)},
      manager_{make_composition_manager<
          Entity, Component, PageSize, ComponentAllocator>()}
  { }

  constexpr
  ~any_composition()
  noexcept
  {
    if (ptr_)
      manager_.do_destroy(ptr_);
  }


  constexpr
  any_composition &operator=(any_composition const &)
  = delete;
  constexpr
  any_composition &operator=(any_composition &&other)
  noexcept
  {
    if (this == &other)
      return *this;

    if (ptr_)
      manager_.do_destroy(ptr_);
    ptr_       = other.ptr_;
    manager_   = other.manager_;
    other.ptr_ = nullptr;

    return *this;
  }



  /**
   * @return A const reference to the type info of the contained composition.
   */
  [[nodiscard]]
  constexpr
  std::type_info const &type() const
  noexcept
  {
    return *manager_.type;
  }


  /**
   * @tparam Entity             The type of the entities of the composition.
   * @tparam Component          The type of the components of the composition.
   * @tparam PageSize           The size of each page for the sparse container
   *     of the composition.
   * @tparam ComponentAllocator The allocator for the components of the
   *     composition.
   * @return A reference to the contained composition, provided the given
   *     template arguments are correct.
   * @warning If the given template arguments do not correspond to the
   *     contained composition, using this method results in undefined
   *     behaviour.
   */
  template<typename    Entity,
           typename    Component,
           std::size_t PageSize           = 4096,
           typename    ComponentAllocator = std::allocator<Component>>
  requires  std::unsigned_integral<Entity>
        &&  std::is_copy_constructible_v<Component>
        &&  std::is_copy_assignable_v   <Component>
        && (PageSize > 0)
  [[nodiscard]]
  constexpr
  composition<Component, Entity, PageSize, ComponentAllocator>
      &get()
  noexcept
  {
    return *static_cast<
        composition<Component, Entity, PageSize, ComponentAllocator> *>(ptr_);
  }
  /**
   * @tparam Entity             The type of the entities of the composition.
   * @tparam Component          The type of the components of the composition.
   * @tparam PageSize           The size of each page for the sparse container
   *     of the composition.
   * @tparam ComponentAllocator The allocator for the components of the
   *     composition.
   * @return A const reference to the contained composition, provided the given
   *     template arguments are correct.
   * @warning If the given template arguments do not correspond to the
   *     contained composition, using this method results in undefined
   *     behaviour.
   */
  template<typename    Entity,
           typename    Component,
           std::size_t PageSize           = 4096,
           typename    ComponentAllocator = std::allocator<Component>>
  requires  std::unsigned_integral<Entity>
        &&  std::is_copy_constructible_v<Component>
        &&  std::is_copy_assignable_v   <Component>
        && (PageSize > 0)
  [[nodiscard]]
  constexpr
  composition<Component, Entity, PageSize, ComponentAllocator> const
      &get() const
  noexcept
  {
    return *static_cast<
        composition<Component, Entity, PageSize, ComponentAllocator> *>(ptr_);
  }



  /**
   * @brief Calls the @p erase method of the contained composition.
   *
   * @tparam Entity The type of the entities of the composition.
   * @param e The entity of the element to erase in the contained composition.
   * @return @c true if an element has been erased, @c false otherwise.
   */
  template<typename Entity>
  requires std::unsigned_integral<Entity>
  constexpr
  bool do_erase(Entity const e)
  {
    return manager_.do_erase(ptr_, static_cast<std::uintmax_t>(e));
  }


  /**
   * @brief Swaps the contents of @p *this and @code other@endcode.
   *
   * @param other The other any_composition to swap the contents of.
   */
  constexpr
  void swap(any_composition &other)
  noexcept
  {
    std::swap(ptr_, other.ptr_);
    manager_.swap(other.manager_);
  }

private:
  void*               ptr_;
  composition_manager manager_;

};


/**
 * @brief Swaps the contents of @code lhs@endcode and @code rhs@endcode.
 *
 * @param lhs The first  any_composition to swap the contents of.
 * @param rhs The second any_composition to swap the contents of.
 */
constexpr
void swap(any_composition &lhs, any_composition &rhs)
noexcept
{
  lhs.swap(rhs);
}

}

#endif // HEIM_COMPONENT_ANY_COMPOSITION_HPP
