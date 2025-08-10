#ifndef HEIM_COMPOSITION_ANY_COMPOSITION_HPP
#define HEIM_COMPOSITION_ANY_COMPOSITION_HPP

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <typeinfo>
#include <utility>
#include "core/component.hpp"
#include "core/entity.hpp"
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
  class manager
  {
  public:
    using do_destroy_type = void  (*)(void *) noexcept;
    using do_clone_type   = void *(*)(void const *);
    using do_erase_type   = bool  (*)(void *, std::uintmax_t const);

  public:
    /**
     * @brief Swaps the contents of @p *this and @code other@endcode.
     *
     * @param other The other manager to swap the contents of.
     */
    constexpr
    void swap(manager &other)
    noexcept
    {
      using std::swap;

      swap(do_destroy, other.do_destroy);
      swap(do_clone  , other.do_clone  );
      swap(do_erase  , other.do_erase  );
      swap(type      , other.type      );
    }

  public:
    do_destroy_type       do_destroy;
    do_clone_type         do_clone;
    do_erase_type         do_erase;

    std::type_info const *type;

  };


  /**
   * @brief Constructs a manager specific to a type of composition, and used in
   *     the constructor of the any_composition.
   *
   * @tparam Entity             The type of the entities of the composition.
   * @tparam Component          The type of the components of the composition.
   * @tparam PageSize           The size of each page for the sparse container
   *     of the composition.
   * @tparam ComponentAllocator The allocator for the components of the
   *     composition.
   * @return The manager specific to the suggested composition to contain.
   */
  template<typename    Entity,
           typename    Component,
           std::size_t PageSize           = 4096,
           typename    ComponentAllocator = std::allocator<Component>>
  requires  core::entity<Entity>
        &&  core::component<Component>
        && (PageSize > 0)
        &&  std::same_as<
                typename std::allocator_traits<ComponentAllocator>::value_type,
                Component>
  [[nodiscard]]
  constexpr
  static manager make_manager()
  noexcept
  {
    return {
        [](void *ptr)
        noexcept
        {
          delete static_cast<
              composition<Entity, Component, PageSize, ComponentAllocator> *>(
                  ptr);
        },
        [](void const *ptr)
        -> void *
        {
          return new composition<
            Entity, Component, PageSize, ComponentAllocator>{
                *static_cast<composition<
                      Entity, Component, PageSize, ComponentAllocator> *>(ptr)};
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
  = default;
  constexpr
  any_composition(any_composition const &other)
    : ptr_    {other.ptr_ ? other.manager_.do_clone(other.ptr_) : nullptr},
      manager_{other.manager_}
  { }
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
  requires  core::entity<Entity>
        &&  core::component<Component>
        && (PageSize > 0)
        &&  std::same_as<
                typename std::allocator_traits<ComponentAllocator>::value_type,
                Component>
  explicit
  constexpr
  any_composition(
      std::in_place_type_t<
          composition<Entity, Component, PageSize, ComponentAllocator>>,
      Args &&...args)
    : ptr_    {new composition<
          Entity, Component, PageSize, ComponentAllocator>(
              std::forward<Args>(args)...)},
      manager_{make_manager<Entity, Component, PageSize, ComponentAllocator>()}
  { }

  constexpr
  ~any_composition()
  noexcept
  {
    if (ptr_)
      manager_.do_destroy(ptr_);
  }


  constexpr
  any_composition &operator=(any_composition const &other)
  {
    if (this == &other)
      return *this;

    if (ptr_)
      manager_.do_destroy(ptr_);

    ptr_     = other.ptr_ ? other.manager_.do_clone(other.ptr_) : nullptr;
    manager_ = other.manager_;

    return *this;
  }
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
   * @return @c true if @p *this contains a value, @c false otherwise.
   */
  [[nodiscard]]
  constexpr
  bool has_value() const
  noexcept
  {
    return ptr_ != nullptr;
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
  requires  core::entity<Entity>
        &&  core::component<Component>
        && (PageSize > 0)
        &&  std::same_as<
                typename std::allocator_traits<ComponentAllocator>::value_type,
                Component>
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
  requires  core::entity<Entity>
        &&  core::component<Component>
        && (PageSize > 0)
        &&  std::same_as<
                typename std::allocator_traits<ComponentAllocator>::value_type,
                Component>
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
   * @brief Changes the contained composition after destroying any currently
   *     contained composition.
   *
   * @tparam Entity             The type of the entities of the composition.
   * @tparam Component          The type of the components of the composition.
   * @tparam PageSize           The size of each page for the sparse container
   *     of the composition.
   * @tparam ComponentAllocator The allocator for the components of the
   *     composition.
   * @tparam Args               The type of arguments to construct the
   *     composition.
   * @return A reference to the newly contained composition.
   *
   * @note If an exception is thrown for any reason, this function has no
   *     effect (strong exception safety guarantee).
   */
  template<typename    Entity,
           typename    Component,
           std::size_t PageSize           = 4096,
           typename    ComponentAllocator = std::allocator<Component>,
           typename ...Args>
  requires  core::entity<Entity>
        &&  core::component<Component>
        && (PageSize > 0)
        &&  std::same_as<
                typename std::allocator_traits<ComponentAllocator>::value_type,
                Component>
  constexpr
  composition<Entity, Component, PageSize, ComponentAllocator> &emplace(
      Args &&...args)
  {
    auto ptr = new composition<Entity, Component, PageSize, ComponentAllocator>(
        std::forward<Args>(args)...);

    if (ptr_)
      manager_.do_destroy(ptr_);

    ptr_     = ptr;
    manager_ = make_manager<Entity, Component, PageSize, ComponentAllocator>();

    return get<Entity, Component, PageSize, ComponentAllocator>();
  }


  /**
   * @brief Destroys any contained composition.
   */
  constexpr
  void reset()
  noexcept
  {
    if (ptr_)
      manager_.do_destroy(ptr_);

    ptr_     = nullptr;
    manager_ = manager{};
  }



  /**
   * @brief Calls the @p erase method of the contained composition.
   *
   * @tparam Entity The type of the entities of the composition.
   * @param e The entity of the element to erase in the contained composition.
   * @return @c true if an element has been erased, @c false otherwise.
   */
  template<typename Entity>
  requires core::entity<Entity>
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
  void   *ptr_;
  manager manager_;

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

#endif // HEIM_COMPOSITION_ANY_COMPOSITION_HPP
