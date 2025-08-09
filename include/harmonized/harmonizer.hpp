#ifndef HEIM_COMPONENT_HARMONIZER_HPP
#define HEIM_COMPONENT_HARMONIZER_HPP

#include <cstddef>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>
#include "any_harmonized.hpp"

namespace heim
{
/**
 * @brief A generic container for harmonized of all types of compositions.
 *
 * @tparam Entity The type of entities for each held harmonized.
 */
template<typename Entity>
requires core::entity<Entity>
class harmonizer
{
public:
  using entity_type = Entity;

  using iterator       = std::vector<any_harmonized>::iterator;
  using const_iterator = std::vector<any_harmonized>::const_iterator;

public:
  /**
   * @return @c true if the harmonizer holds any harmonized, @c false otherwise.
   */
  [[nodiscard]]
  constexpr
  bool empty() const
  noexcept
  {
    return harmonized_.empty();
  }

  /**
   * @return The number of harmonized in the harmonizer.
   */
  [[nodiscard]]
  constexpr
  std::size_t size() const
  noexcept
  {
    return harmonized_.size();
  }



  /**
   * @return An iterator to the first type-erased harmonized of the harmonizer.
   */
  [[nodiscard]]
  constexpr
  iterator       begin()
  noexcept
  {
    return harmonized_.begin();
  }
  /**
   * @return A const iterator to the first type-erased harmonized of the
   *     harmonizer.
   */
  [[nodiscard]]
  constexpr
  const_iterator begin() const
  noexcept
  {
    return harmonized_.begin();
  }

  /**
   * @return An iterator to after the last type-erased harmonized of the
   *     harmonizer.
   *
   * @note This returned iterator only acts as a sentinel, and is not to be
   *     dereferenced.
   */
  [[nodiscard]]
  constexpr
  iterator       end()
  noexcept
  {
    return harmonized_.end();
  }
  /**
   * @return A const iterator to after the last type-erased harmonized of the
   *     harmonizer.
   *
   * @note This returned const iterator only acts as a sentinel, and is not to
   *     be dereferenced.
   */
  [[nodiscard]]
  constexpr
  const_iterator end() const
  noexcept
  {
    return harmonized_.end();
  }


  /**
   * @return A const iterator to the first type-erased harmonized of the
   *     harmonizer.
   */
  [[nodiscard]]
  constexpr
  const_iterator cbegin() const
  noexcept
  {
    return harmonized_.cbegin();
  }

  /**
   * @return A const iterator to after the last type-erased harmonized of the
   *     harmonizer.
   *
   * @note This returned const iterator only acts as a sentinel, and is not to
   *     be dereferenced.
   */
  [[nodiscard]]
  constexpr
  const_iterator cend() const
  noexcept
  {
    return harmonized_.cend();
  }



  /**
   * @tparam Composition The type of composition.
   * @return The type index of the composition.
   */
  template<typename Composition>
  requires core::specialization_of<Composition, composition>
        && std::is_same_v<typename Composition::entity_type, entity_type>
  [[nodiscard]]
  constexpr
  static std::type_index type()
  noexcept
  {
    return std::type_index{typeid(Composition)};
  }
  /**
   * @tparam Compositions The types of compositions of the harmonized.
   * @return The type index of a harmonized of such compositions.
   */
  template<typename ...Compositions>
  requires (sizeof...(Compositions) > 1)
        && (core::specialization_of<Compositions, composition>
            && ...)
        && (std::is_same_v<typename Compositions::entity_type, entity_type>
            && ...)
  [[nodiscard]]
  constexpr
  static std::type_index type()
  noexcept
  {
    return std::type_index{typeid(harmonized<Entity, Compositions ...>)};
  }

  /**
   * @tparam Composition The type of composition of the harmonized.
   * @return The index in the harmonizer of the harmonized of this type of
   *     composition.
   */
  template<typename Composition>
  requires core::specialization_of<Composition, composition>
        && std::is_same_v<typename Composition::entity_type, entity_type>
  [[nodiscard]]
  constexpr
  std::size_t index() const
  noexcept
  {
    return composition_indexes_.find(type<Composition>())->second;
  }
  /**
   * @tparam Compositions The types of compositions of the harmonized.
   * @return The index in the harmonizer of the harmonized of such compositions.
   */
  template<typename ...Compositions>
  requires (sizeof...(Compositions) > 1)
        && (core::specialization_of<Compositions, composition>
            && ...)
        && (std::is_same_v<typename Compositions::entity_type, entity_type>
            && ...)
  [[nodiscard]]
  constexpr
  std::size_t index() const
  noexcept
  {
    return harmonized_indexes_.find(type<Compositions ...>())->second;
  }


  /**
   * @tparam Composition The type of composition of the harmonized.
   * @return @c true if the harmonizer holds a harmonized comprising the given
   *     type of composition.
   */
  template<typename Composition>
  requires core::specialization_of<Composition, composition>
        && std::is_same_v<typename Composition::entity_type, entity_type>
  [[nodiscard]]
  constexpr
  bool holds() const
  noexcept
  {
    return composition_indexes_.contains(type<Composition>());
  }
  /**
   * @tparam Compositions The types of compositions of the harmonized.
   * @return @c true if the harmonizer holds a harmonized of such compositions.
   */
  template<typename ...Compositions>
  requires (sizeof...(Compositions) > 1)
        && (core::specialization_of<Compositions, composition>
            && ...)
        && (std::is_same_v<typename Compositions::entity_type, entity_type>
            && ...)
  [[nodiscard]]
  constexpr
  bool holds() const
  noexcept
  {
    return harmonized_indexes_.contains(type<Compositions ...>());
  }


  /**
   * @tparam Compositions The types of compositions of the harmonized.
   * @return A reference to the held harmonized of such compositions.
   */
  template<typename ...Compositions>
  requires (sizeof...(Compositions) > 1)
        && (core::specialization_of<Compositions, composition>
            && ...)
        && (std::is_same_v<typename Compositions::entity_type, entity_type>
            && ...)
  [[nodiscard]]
  constexpr
  harmonized<entity_type, Compositions ...>       &get()
  noexcept
  {
    return harmonized_[index<Compositions ...>()].template
        get<entity_type, Compositions ...>();
  }
  /**
   * @tparam Compositions The types of compositions of the harmonized.
   * @return A const reference to the held harmonized of such compositions.
   */
  template<typename ...Compositions>
  requires (sizeof...(Compositions) > 1)
        && (core::specialization_of<Compositions, composition>
            && ...)
        && (std::is_same_v<typename Compositions::entity_type, entity_type>
            && ...)
  [[nodiscard]]
  constexpr
  harmonized<entity_type, Compositions ...> const &get() const
  noexcept
  {
    return harmonized_[index<Compositions ...>()].template
        get<entity_type, Compositions ...>();
  }



  /**
   * @brief Harmonizes the compositions by appending a new harmonized in the
   *     harmonizer.
   *
   * @tparam Compositions The types of compositions to harmonize.
   * @param compositions The compositions to harmonize.
   * @return @c true if the harmonizer has harmonized the compositions,
   *     @c false otherwise.
   *
   * @note If an exception is thrown for any reason, this function has no
   *     effect (strong exception safety guarantee).
   */
  template<typename ...Compositions>
  requires (sizeof...(Compositions) > 1)
        && (core::specialization_of<Compositions, composition>
            && ...)
        && (std::is_same_v<typename Compositions::entity_type, entity_type>
            && ...)
  constexpr
  bool harmonize(Compositions &...compositions)
  {
    if (holds<Compositions...>())
      return false;
    if ((holds<Compositions>() || ...))
      return false;

    (composition_indexes_.emplace(type<Compositions>(), harmonized_.size()),
        ...);
     harmonized_indexes_ .emplace(type<Compositions ...>(), harmonized_.size());

    try
    {
      harmonized_.emplace_back(
          std::in_place_type_t<entity_type>{},
          compositions...);
    }
    catch (...)
    {
      (composition_indexes_.erase(type<Compositions>()), ...);
       harmonized_indexes_ .erase(type<Compositions ...>());
      throw;
    }
    return true;
  }

  /**
   * @brief Separates the compositions by destroying the harmonized that linked
   *     them, if there is any.
   *
   * @tparam Compositions The types of compositions to separate.
   * @return @c true if the compositions have been separated, @c false
   *     otherwise.
   *
   * @warning This method uses a "swap-and-pop" algorithm to avoid moving
   *     multiple harmonized. If any part of your program relies on the index
   *     of the last harmonized, you will have to update it (using the index
   *     method).
   */
  template<typename ...Compositions>
  requires (sizeof...(Compositions) > 1)
        && (core::specialization_of<Compositions, composition>
            && ...)
        && (std::is_same_v<typename Compositions::entity_type, entity_type>
            && ...)
  constexpr
  bool separate()
  noexcept
  {
    if (!holds<Compositions...>())
      return false;

    std::size_t const idx      = index<Compositions ...>();
    std::size_t const back_idx = harmonized_.size() - 1;
    if (idx != back_idx)
    {
      for (auto &[t, i] : composition_indexes_)
      {
        if (i == back_idx)
          i = idx;
      }
      harmonized_indexes_[std::type_index{harmonized_.back().type()}] = idx;
      harmonized_[idx] = std::move(harmonized_.back());
    }

    (composition_indexes_.erase(type<Compositions>()), ...);
     harmonized_indexes_ .erase(type<Compositions ...>());
    harmonized_.pop_back();

    return true;
  }


  /**
   * @brief Swaps the contents of @p *this and @code other@endcode.
   *
   * @param other The other harmonizer whose contents to swap.
   */
  constexpr
  void swap(harmonizer &other)
  noexcept
  {
    harmonized_.swap(other.harmonized_);
    harmonized_indexes_ .swap(other.harmonized_indexes_ );
    composition_indexes_.swap(other.composition_indexes_);
  }

private:
  std::vector<any_harmonized> harmonized_;

  std::unordered_map<std::type_index, std::size_t> harmonized_indexes_ ;
  std::unordered_map<std::type_index, std::size_t> composition_indexes_;

};


/**
 * @brief Swaps the contents of @code lhs@endcode and @code rhs@endcode.
 *
 * @tparam Entity The type of entities of the harmonizers.
 * @param lhs The first  harmonizer whose contents to swap.
 * @param rhs The second harmonizer whose contents to swap.
 */
template<typename Entity>
requires core::entity<Entity>
constexpr
void swap(harmonizer<Entity> &lhs, harmonizer<Entity> &rhs)
noexcept
{
  lhs.swap(rhs);
}

}

#endif // HEIM_COMPONENT_HARMONIZER_HPP
