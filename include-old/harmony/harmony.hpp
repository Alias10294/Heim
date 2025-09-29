#ifndef HEIM_HARMONY_HARMONY_HPP
#define HEIM_HARMONY_HARMONY_HPP

#include <algorithm>
#include "entity.hpp"
#include "iterator.hpp"

namespace heim
{
template<typename T>
concept observable = requires (
    T &t,
    T &u,
    typename T::entity_type a,
    typename T::entity_type b)
{
  requires iterator<typename T::iterator>;
  requires iterator<typename T::const_iterator>;
  { t.begin() } noexcept;
  { t.end()   } noexcept;
  { t.cbegin() } noexcept;
  { t.cend()   } noexcept;

  { t.empty() } noexcept -> std::same_as<bool>;
  { t.size()  } noexcept -> std::same_as<std::size_t>;

  { t.contains(a) } noexcept -> std::same_as<bool>;

  { t.swap(u) };
  { t.swap(a, b) };

};


/**
 * @tparam Entity The type of entities to harmonize.
 * @tparam Ts     The type of each container of entities to harmonize.
 */
template<typename    Entity,
         typename ...Ts>
requires  entity<Entity>
      && (sizeof...(Ts) > 1)
      && (observable<Ts>                                  && ...)
      && (std::is_same_v<typename Ts::entity_type, Entity> && ...)
class harmony
{
public:
  using entity_type         = Entity;
  using container_tuple_type = std::tuple<Ts ...>;

private:
  /// @cond INTERNAL

  /**
   * @brief The observer of a container of entities.
   *
   * Inspired from the observer software design pattern, the harmony
   *     representing the subject in the pattern.
   */
  template<typename T>
  requires observable<T>
  class observer
  {
  public:
    using entity_type   = typename T::entity_type;
    using arranged_type = T;

  public:
    constexpr
    observer()
    = default;
    constexpr
    observer(observer const &)
    = default;
    constexpr
    observer(observer &&)
    noexcept
    = default;
    explicit
    constexpr
    observer(T &arranged)
      : arranged_{&arranged}
    { }

    constexpr
    ~observer()
    noexcept
    = default;


    constexpr
    observer &operator=(observer const &)
    = default;
    constexpr
    observer &operator=(observer &&)
    noexcept
    = default;



    /**
     * @return The number of entities contained in the observed container.
     */
    [[nodiscard]]
    constexpr
    std::size_t size() const
    noexcept
    {
      return arranged_->size();
    }



    /**
     * @The callback to the observed container.
     */
    constexpr
    void operator()(entity_type const lhs, std::ptrdiff_t const len)
    noexcept(noexcept(std::declval<T &>().swap(0, 0)))
    {
      arranged_->swap(lhs, std::get<0>(*(arranged_->begin() + len)));
    }


    /**
     * @brief Swaps the contents of @p *this and @code other@endcode.
     *
     * @param other The other observer whose contents to swap.
     */
    constexpr
    void swap(observer &other)
    noexcept(noexcept(std::declval<T &>().swap(std::declval<T &>())))
    {
      arranged_->swap(*other.arranged_);
    }

  private:
    T *arranged_;

  };

public:
  constexpr
  harmony()
  = default;
  constexpr
  harmony(harmony const &)
  = default;
  constexpr
  harmony(harmony &&)
  noexcept
  = default;
  constexpr
  harmony(Ts &...arranged)
    : observers_{observer<Ts>(arranged)...},
      length_      {0}
  { }

  constexpr
  ~harmony()
  noexcept
  = default;


  [[nodiscard]]
  constexpr
  harmony &operator=(harmony const &)
  = default;
  [[nodiscard]]
  constexpr
  harmony &operator=(harmony &&)
  noexcept
  = default;



  /**
   * @return The number of harmonized entities.
   */
  [[nodiscard]]
  constexpr
  std::ptrdiff_t length() const
  noexcept
  {
    return length_;
  }



  /**
   * @brief Includes @code e@endcode to the group of common entities.
   *
   * @param e The entity to include.
   */
  constexpr
  void include(entity_type const e)
  noexcept(noexcept(notify(0, std::index_sequence_for<Ts...>{})))
  {
    notify(e, std::index_sequence_for<Ts...>{});
    ++length_;
  }

  /**
   * @brief Excludes @code e@endcode of the group of common entities.
   *
   * @param e The entity to exclude.
   */
  constexpr
  void exclude(entity_type const e)
  noexcept(noexcept(notify(0, std::index_sequence_for<Ts...>{})))
  {
    --length_;
    notify(e, std::index_sequence_for<Ts...>{});
  }


  /**
   * @brief Swaps the contents of @p *this and @code other@endcode.
   *
   * @param other The other harmony whose contents to swap.
   */
  constexpr
  void swap(harmony &other)
  noexcept
  {
    observers_.swap(other.observers_);
    std::swap(length_, other.length_);
  }

private:
  /// @cond INTERNAL

  /**
   * @brief Notifies all observers for the given entity.
   *
   * @tparam Is The indexes of the tuple of observers.
   * @param e The entity to notify the observers for.
   */
  template<std::size_t ...Is>
  constexpr
  void notify(entity_type const e, std::index_sequence<Is...>)
  noexcept((noexcept(std::get<Is>(observers_)(
      std::declval<entity_type>(),
      std::declval<std::ptrdiff_t>()))
   && ...))
  {
    (std::get<Is>(observers_)(e, length_), ...);
  }

  /// @endcond

private:
  std::tuple<observer<Ts> ...> observers_;
  std::ptrdiff_t               length_;

};


/**
 * @brief Swaps the contents of @code lhs@endcode and @code rhs@endcode.
 *
 * @param lhs The first  harmony whose contents to swap.
 * @param rhs The second harmony whose contents to swap.
 */
template<typename    Entity,
         typename ...Ts>
requires  entity<Entity>
      && (sizeof...(Ts) > 1)
      && (observable<Ts>                                  && ...)
      && (std::is_same_v<typename Ts::entity_type, Entity> && ...)
constexpr
void swap(harmony<Entity, Ts ...> &lhs, harmony<Entity, Ts ...> &rhs)
noexcept
{
  lhs.swap(rhs);
}



template<typename T>
struct is_harmony_specialization : std::false_type
{ };

template<typename    Entity,
         typename ...Ts>
requires  entity<Entity>
      && (sizeof...(Ts) > 1)
      && (observable<Ts>                                  && ...)
      && (std::is_same_v<typename Ts::entity_type, Entity> && ...)
struct is_harmony_specialization<
    harmony<Entity, Ts ...>>
  : std::true_type
{ };

template<typename T>
concept harmony_specialization = is_harmony_specialization<T>::value;

}

#endif // HEIM_HARMONY_HARMONY_HPP
