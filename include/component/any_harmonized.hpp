#ifndef HEIM_COMPONENT_ANY_HARMONIZED_HPP
#define HEIM_COMPONENT_ANY_HARMONIZED_HPP

#include <concepts>
#include <typeinfo>
#include "composition.hpp"
#include "harmonized.hpp"
#include "utils/specialization_of.hpp"

namespace heim
{
/**
 * @brief The container for a type-erased harmonized,
 */
class any_harmonized
{
private:
  /// @cond INTERNAL

  /**
   * @brief The manager for the contained type-erased harmonized.
   */
  class manager
  {
  public:
    using do_destroy_type = void (*)(void *) noexcept;

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
      swap(type      , other.type      );
    }

  public:
    do_destroy_type       do_destroy;

    std::type_info const *type;

  };

  /**
   * @brief Constructs a manager specific to a type of harmonized, and used in
   *     the constructor of the any_composition.
   *
   * @tparam Entity       The type of entities of the harmonized.
   * @tparam Compositions The type of compositions of the harmonized.
   * @return The manager specific to the suggested harmonized.
   */
  template<typename    Entity,
           typename ...Compositions>
  requires  std::unsigned_integral<Entity>
        && (specialization_of<Compositions, composition> && ...)
  static manager make_manager()
  {
    return {
        [](void *ptr)
        noexcept
        {
          delete static_cast<harmonized<Entity, Compositions ...> *>(ptr);
        },
        &typeid(harmonized<Entity, Compositions ...>)};
  }

  /// @endcond

public:
  constexpr
  any_harmonized()
  = delete;
  constexpr
  any_harmonized(any_harmonized const &)
  = delete;
  constexpr
  any_harmonized(any_harmonized &&other)
  noexcept
    : ptr_    {other.ptr_},
      manager_{other.manager_}
  {
    other.ptr_ = nullptr;
  }
  template<typename    Entity,
           typename ...Compositions>
  requires  std::unsigned_integral<Entity>
        && (specialization_of<Compositions, composition> && ...)
  any_harmonized(
      std::in_place_type_t<Entity>,
      Compositions const &...compositions)
    : ptr_    {new harmonized<Entity, Compositions ...>{compositions...}},
      manager_{make_manager<Entity, Compositions ...>()}
  { }

  constexpr
  ~any_harmonized()
  noexcept
  {
    if (ptr_)
      manager_.do_destroy(ptr_);
  }


  constexpr
  any_harmonized &operator=(any_harmonized const &)
  = delete;
  constexpr
  any_harmonized &operator=(any_harmonized &&other)
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
   * @return A const reference to the type info of the contained harmonized.
   */
  [[nodiscard]]
  constexpr
  std::type_info const &type() const
  noexcept
  {
    return *manager_.type;
  }


  /**
   * @tparam Entity       The type of entities of the harmonized.
   * @tparam Compositions The type of compositions of the harmonized.
   * @return A reference to the contained harmonized, provided the given
   *     template arguments are correct.
   * @warning If the given template arguments do not correspond to the
   *     contained harmonized, using this method results in undefined
   *     behaviour.
   */
  template<typename    Entity,
           typename ...Compositions>
  requires  std::unsigned_integral<Entity>
        && (specialization_of<Compositions, composition> && ...)
  [[nodiscard]]
  constexpr
  harmonized<Entity, Compositions ...>       &get()
  noexcept
  {
    return *static_cast<harmonized<Entity, Compositions ...> *>(ptr_);
  }
  /**
   * @tparam Entity       The type of entities of the harmonized.
   * @tparam Compositions The type of compositions of the harmonized.
   * @return A const reference to the contained harmonized, provided the given
   *     template arguments are correct.
   * @warning If the given template arguments do not correspond to the
   *     contained harmonized, using this method results in undefined
   *     behaviour.
   */
  template<typename    Entity,
           typename ...Compositions>
  requires  std::unsigned_integral<Entity>
        && (specialization_of<Compositions, composition> && ...)
  [[nodiscard]]
  constexpr
  harmonized<Entity, Compositions ...> const &get() const
  noexcept
  {
    return *static_cast<harmonized<Entity, Compositions ...> *>(ptr_);
  }



  /**
   * @brief Swaps the contents of @p *this and @code other@endcode.
   *
   * @param other The other any_composition to swap the contents of.
   */
  constexpr
  void swap(any_harmonized &other)
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
 * @param lhs The first  any_harmonized to swap the contents of.
 * @param rhs The second any_harmonized to swap the contents of.
 */
constexpr
void swap(any_harmonized &lhs, any_harmonized &rhs)
noexcept
{
  lhs.swap(rhs);
}

}

#endif // HEIM_COMPONENT_ANY_HARMONIZED_HPP
