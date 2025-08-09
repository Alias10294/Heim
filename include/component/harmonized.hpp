#ifndef HEIM_COMPONENT_HARMONIZED_HPP
#define HEIM_COMPONENT_HARMONIZED_HPP

#include <algorithm>
#include <concepts>
#include "composition.hpp"
#include "utils/specialization_of.hpp"

namespace heim
{
/**
 * @brief An object providing harmonization of the common entities of the given
 *     compositions.
 *
 * @tparam Entity       The type  of entities of each composition.
 * @tparam Compositions The types of each composition.
 */
template<typename    Entity,
         typename ...Compositions>
requires  std::unsigned_integral<Entity>
      && (sizeof...(Compositions) > 1)
      && (specialization_of<Compositions, composition>               && ...)
      && (std::is_same_v<typename Compositions::entity_type, Entity> && ...)
class harmonized
{
public:
  using entity_type = Entity;

private:
  /// @cond INTERNAL

  /**
   * @brief The observer adaptor for each given composition.
   *
   * @tparam Composition The composition to adapt.
   */
  template<typename Composition>
  requires specialization_of<Composition, composition>
        && std::is_same_v<typename Composition::entity_type, Entity>
  class observer
  {
  public:
    using composition_type = Composition;

  public:
    constexpr
    observer(observer const &)
    = default;
    constexpr
    observer(observer &&)
    noexcept
    = default;
    constexpr
    observer(composition_type &composition)
      : composition_{composition}
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
     * @brief The callback to invoke when the observer is notified.
     *
     * @param e   The entity observed.
     * @param len The current length of the group of common entities.
     */
    constexpr
    void operator()(entity_type const e, std::ptrdiff_t len)
    noexcept(std::declval<composition_type>().swap(0, 0))
    {
      composition_.swap(e, std::get<0>(*(composition_.begin() + len)));
    }

  private:
    composition_type &composition_;

  };

private:
  using observer_tuple_type = std::tuple<observer<Compositions>...>;

  /// @endcond

public:
  constexpr
  harmonized(harmonized const &)
  = default;
  constexpr
  harmonized(harmonized &&)
  noexcept
  = default;
  constexpr
  harmonized(Compositions const &...compositions)
    : observers_{compositions...},
      length_   {0}
  {
    harmonize(compositions...);
  }

  constexpr
  ~harmonized()
  noexcept
  = default;


  [[nodiscard]]
  constexpr
  harmonized &operator=(harmonized const &)
  = default;
  [[nodiscard]]
  constexpr
  harmonized &operator=(harmonized &&)
  noexcept
  = default;



  /**
   * @return The number of common entities in the harmonized group of
   *     compositions.
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
  noexcept(noexcept(notify(0, std::index_sequence_for<Compositions...>{})))
  {
    notify(e, std::index_sequence_for<Compositions...>{});
    ++length_;
  }

  /**
   * @brief Excludes @code e@endcode of the group of common entities.
   *
   * @param e The entity to exclude.
   */
  constexpr
  void exclude(entity_type const e)
  noexcept(noexcept(notify(0, std::index_sequence_for<Compositions...>{})))
  {
    --length_;
    notify(e, std::index_sequence_for<Compositions...>{});
  }

private:
  /// @cond INTERNAL

  /**
   * @tparam Head The type  of the first given composition.
   * @tparam Tail The types of the rest of the given compositions.
   * @param head A reference to the first composition.
   * @param tail References to the rest of the compositions.
   * @return The smallest composition of the given pack of compositions.
   */
  template<typename    Head,
           typename ...Tail>
  requires  specialization_of<Head, composition>
        &&  std::is_same_v<typename Head::entity_type, Entity>
        && (specialization_of<Tail, composition>               && ...)
        && (std::is_same_v<typename Tail::entity_type, Entity> && ...)
  constexpr
  auto &min(Head &head, Tail &...tail)
  noexcept
  {
    auto* min = &head;
    ((min = tail.size() < min->size() ? &tail : min), ...);
    return *min;
  }


  /**
   * @brief harmonizes all the given compositions, packing their common
   *     entities at the beginning of each one.
   *
   * @param compositions The compositions to harmonize.
   */
  constexpr
  void harmonize(Compositions const &...compositions)
  noexcept(noexcept(include(0)))
  {
    auto &pivot = min(compositions...);

    if (pivot.empty())
      return;

    auto it = pivot.end();
    while (it != pivot.begin() + length_)
    {
      entity_type const e = (*--it).entity;
      if ((compositions.contains(e) && ...))
        include(e);
    }
  }



  /**
   * @brief Notifies each observer using their operator().
   *
   * @tparam Is The sequence of indexes of the tuple of observers.
   * @param e The entity to notify the observers of.
   */
  template<std::size_t ...Is>
  constexpr
  void notify(entity_type const e, std::index_sequence<Is...>)
  noexcept((noexcept(
      std::get<Is>(std::declval<observer<Compositions>...>())(0, 0))
   && ...))
  {
    (std::get<Is>(observers_)(e, length_), ...);
  }

  /// @endcond

private:
  observer_tuple_type observers_;
  std::ptrdiff_t      length_;

};

}

#endif // HEIM_COMPONENT_HARMONIZED_HPP
