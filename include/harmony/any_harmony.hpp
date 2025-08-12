#ifndef HEIM_HARMONY_ANY_HARMONY_HPP
#define HEIM_HARMONY_ANY_HARMONY_HPP

#include <typeinfo>
#include "harmony.hpp"

namespace heim
{
/**
 * @brief The container for a type-erased harmony.
 */
class any_harmony
{
private:
  /// @cond INTERNAL

  /**
   * @brief The manager for the contained harmony, stored as a void pointer.
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
    do_destroy_type       do_destroy = nullptr;
    do_clone_type         do_clone   = nullptr;

    std::type_info const *type       = &typeid(void);

  };

  /**
   * @tparam Harmony The type of harmony to male a manager for.
   * @return The manager for the specific harmony type.
   */
  template<typename Harmony>
  requires harmony_specialization<Harmony>
  static manager make_manager()
  {
    return manager{
        [](void *ptr)
        noexcept
        {
          delete static_cast<Harmony *>(ptr);
        },
        [](void const *ptr)
        -> void *
        {
          return new Harmony{*static_cast<Harmony const *>(ptr)};
        },
        &typeid(Harmony)};
  }

  /// @endcond

public:
  constexpr
  any_harmony()
  = default;
  constexpr
  any_harmony(any_harmony const &other)
    : ptr_    {other.ptr_ ? other.manager_.do_clone(other.ptr_) : nullptr},
      manager_{other.manager_}
  { }
  constexpr
  any_harmony(any_harmony &&other)
  noexcept
    : ptr_    {other.ptr_},
      manager_{other.manager_}
  {
    other.ptr_ = nullptr;
  }
  template<typename    Harmony,
           typename ...Args>
  requires harmony_specialization<Harmony>
  any_harmony(std::in_place_type_t<Harmony>, Args &&...args)
    : ptr_    {new Harmony{std::forward<Args>(args)...}},
      manager_{make_manager<Harmony>()}
  { }

  constexpr
  ~any_harmony()
  noexcept
  {
    if (ptr_)
      manager_.do_destroy(ptr_);
  }


  constexpr
  any_harmony &operator=(any_harmony const &other)
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
  any_harmony &operator=(any_harmony &&other)
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
   * @return A const reference to the type info of the contained harmony.
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
   * @tparam Harmony The type of the contained harmony.
   * @return A reference to the contained harmony.
   */
  template<typename Harmony>
  requires harmony_specialization<Harmony>
  [[nodiscard]]
  constexpr
  Harmony       &get()
  noexcept
  {
    return *static_cast<Harmony *>(ptr_);
  }
  /**
   * @tparam Harmony The type of the contained harmony.
   * @return A const reference to the contained harmony.
   */
  template<typename Harmony>
  requires harmony_specialization<Harmony>
  [[nodiscard]]
  constexpr
  Harmony const &get() const
  noexcept
  {
    return *static_cast<Harmony *>(ptr_);
  }



  /**
   * @brief Emplaces a new harmony, overwriting any previously contained
   *     harmony of any type.
   *
   * @tparam Harmony The type of harmony to emplace.
   * @tparam Args    The type of arguments to construct the harmony.
   * @return A reference to the newly contained harmony.
   */
  template<typename    Harmony,
           typename ...Args>
  requires harmony_specialization<Harmony>
  constexpr
  Harmony &emplace(Args &&...args)
  {
    auto ptr = new Harmony(std::forward<Args>(args)...);

    if (ptr_)
      manager_.do_destroy(ptr_);

    ptr_     = ptr;
    manager_ = make_manager<Harmony>();

    return get<Harmony>();
  }


  /**
   * @brief Destroys any contained harmony.
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
   * @param other The other any_harmony to swap the contents of.
   */
  constexpr
  void swap(any_harmony &other)
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
 * @param lhs The first  any_harmony to swap the contents of.
 * @param rhs The second any_harmony to swap the contents of.
 */
constexpr
void swap(any_harmony &lhs, any_harmony &rhs)
noexcept
{
  lhs.swap(rhs);
}

}

#endif // HEIM_HARMONY_ANY_HARMONY_HPP
