#ifndef HEIM_COMPOSITION_ANY_COMPOSITION_HPP
#define HEIM_COMPOSITION_ANY_COMPOSITION_HPP

#include <cstdint>
#include <memory>
#include <typeinfo>
#include <utility>
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
   * @brief The manager for the contained composition, stored as a void
   *     pointer.
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
     * @param other The other manager whose contents to swap.
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
    do_destroy_type       do_destroy = nullptr;
    do_clone_type         do_clone   = nullptr;
    do_erase_type         do_erase   = nullptr;

    std::type_info const *type       = &typeid(void);

  };

  /**
   * @tparam Composition The type of composition to male a manager for.
   * @return The manager for the specific composition type.
   */
  template<typename Composition>
  requires composition_specialization<Composition>
  [[nodiscard]]
  constexpr
  static manager make_manager()
  noexcept
  {
    return manager{
        [](void *ptr)
        noexcept
        {
          delete static_cast<Composition *>(ptr);
        },
        [](void const *ptr)
        -> void *
        {
          return new Composition{*static_cast<Composition const *>(ptr)};
        },
        [](void *ptr, std::uintmax_t const e)
        -> bool
        {
          return static_cast<Composition *>(ptr)->erase(
              static_cast<typename Composition::entity_type>(e));
        },
        &typeid(Composition)};
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
  template<typename    Composition,
           typename ...Args>
  requires composition_specialization<Composition>
  explicit
  constexpr
  any_composition(std::in_place_type_t<Composition>, Args &&...args)
    : ptr_    {new Composition{std::forward<Args>(args)...}},
      manager_{make_manager<Composition>()}
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
   * @tparam Composition The type of the contained composition.
   * @return A reference to the contained composition.
   */
  template<typename Composition>
  requires composition_specialization<Composition>
  [[nodiscard]]
  constexpr
  Composition       &get()
  noexcept
  {
    return *static_cast<Composition *>(ptr_);
  }
  /**
   * @tparam Composition The type of the contained composition.
   * @return A const reference to the contained composition.
   */
  template<typename Composition>
  requires composition_specialization<Composition>
  [[nodiscard]]
  constexpr
  Composition const &get() const
  noexcept
  {
    return *static_cast<Composition *>(ptr_);
  }



  /**
   * @brief Emplaces a new composition, overwriting any previously contained
   *     composition of any type.
   *
   * @tparam Composition The type of composition to emplace.
   * @tparam Args        The type of arguments to construct the composition.
   * @return A reference to the newly contained composition.
   */
  template<typename    Composition,
           typename ...Args>
  requires composition_specialization<Composition>
  constexpr
  Composition &emplace(Args &&...args)
  {
    auto ptr = new Composition{std::forward<Args>(args)...};

    if (ptr_)
      manager_.do_destroy(ptr_);

    ptr_     = ptr;
    manager_ = make_manager<Composition>();

    return get<Composition>();
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
  void   *ptr_     = nullptr;
  manager manager_ = manager{};

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
