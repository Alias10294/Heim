#ifndef HEIM_COMPONENT_ANY_HARMONIZED_HPP
#define HEIM_COMPONENT_ANY_HARMONIZED_HPP

#include <typeinfo>
#include "harmonized.hpp"
#include "core/specialization_of.hpp"

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
    using do_destroy_type = void  (*)(void *) noexcept;
    using do_clone_type   = void *(*)(void const *);

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
      std::swap(do_destroy, other.do_destroy);
      std::swap(do_clone  , other.do_clone  );
      std::swap(type      , other.type      );
    }

  public:
    do_destroy_type       do_destroy;
    do_clone_type         do_clone;

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
  requires  core::entity<Entity>
        && (sizeof...(Compositions) > 1)
        && (core::specialization_of<Compositions, composition>         && ...)
        && (std::is_same_v<typename Compositions::entity_type, Entity> && ...)
  static manager make_manager()
  {
    return {
        [](void *ptr)
        noexcept
        {
          delete static_cast<harmonized<Entity, Compositions ...> *>(ptr);
        },
        [](void const *ptr)
        -> void *
        {
          return new harmonized<Entity, Compositions ...>{
              *static_cast<harmonized<Entity, Compositions ...> *>(ptr)};
        },
        &typeid(harmonized<Entity, Compositions ...>)};
  }

  /// @endcond

public:
  constexpr
  any_harmonized()
  = default;
  constexpr
  any_harmonized(any_harmonized const &other)
    : ptr_    {other.ptr_ ? other.manager_.do_clone(other.ptr_) : nullptr},
      manager_{other.manager_}
  { }
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
  requires  core::entity<Entity>
        && (sizeof...(Compositions) > 1)
        && (core::specialization_of<Compositions, composition>         && ...)
        && (std::is_same_v<typename Compositions::entity_type, Entity> && ...)
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
  any_harmonized &operator=(any_harmonized const &other)
  {
    if (this == &other)
      return *this;

    if (ptr_)
      manager_.do_destroy(ptr_);

    ptr_       = other.ptr_ ? other.manager_.do_clone(other.ptr_) : nullptr;
    manager_   = other.manager_;

    return *this;
  }
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
  requires  core::entity<Entity>
        && (sizeof...(Compositions) > 1)
        && (core::specialization_of<Compositions, composition>         && ...)
        && (std::is_same_v<typename Compositions::entity_type, Entity> && ...)
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
  requires  core::entity<Entity>
        && (sizeof...(Compositions) > 1)
        && (core::specialization_of<Compositions, composition>         && ...)
        && (std::is_same_v<typename Compositions::entity_type, Entity> && ...)
  [[nodiscard]]
  constexpr
  harmonized<Entity, Compositions ...> const &get() const
  noexcept
  {
    return *static_cast<harmonized<Entity, Compositions ...> *>(ptr_);
  }



  /**
   * @brief Changes the contained harmonized after destroying any currently
   *     contained harmonized.
   *
   * @tparam Entity       The type of entities of the harmonized.
   * @tparam Compositions The type of compositions of the harmonized.
   * @tparam Args         The type of arguments to construct the harmonized.
   * @return A reference to the newly contained harmonized.
   *
   * @note If an exception is thrown for any reason, this function has no
   *     effect (strong exception safety guarantee).
   */
  template<typename    Entity,
           typename ...Compositions,
           typename ...Args>
  requires  core::entity<Entity>
        && (sizeof...(Compositions) > 1)
        && (core::specialization_of<Compositions, composition>         && ...)
        && (std::is_same_v<typename Compositions::entity_type, Entity> && ...)
  constexpr
  harmonized<Entity, Compositions ...> &emplace(Args &&...args)
  {
    auto ptr = new harmonized<Entity, Compositions ...>(
        std::forward<Args>(args)...);

    if (ptr_)
      manager_.do_destroy(ptr_);

    ptr_     = ptr;
    manager_ = make_manager<Entity, Compositions ...>();

    return get<Entity, Compositions ...>();
  }


  /**
   * @brief Destroys any contained harmonized.
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
