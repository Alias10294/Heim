#ifndef HEIM_COMPOSITION_COMPOSER_HPP
#define HEIM_COMPOSITION_COMPOSER_HPP

#include "any_composition.hpp"
#include "composition.hpp"

namespace heim
{
class composer
{
public:
  /**
   * @return An iterator to the first any_composition in the composer.
   */
  [[nodiscard]]
  constexpr
  std::vector<any_composition>::iterator       begin()
  noexcept
  {
    return compositions_.begin();
  }
  /**
   * @return A const_iterator to the first any_composition in the composer.
   */
  [[nodiscard]]
  constexpr
  std::vector<any_composition>::const_iterator begin() const
  noexcept
  {
    return compositions_.begin();
  }

  /**
   * @return An iterator to after the last any_composition in the composer.
   *
   * @note This returned iterator only acts as a sentinel, and is not to be
   *     dereferenced.
   */
  [[nodiscard]]
  constexpr
  std::vector<any_composition>::iterator       end()
  noexcept
  {
    return compositions_.end();
  }
  /**
   * @return A const iterator to after the last any_composition in the
   *     composer.
   *
   * @note This returned const_iterator only acts as a sentinel, and is not to
   *     be dereferenced.
   */
  [[nodiscard]]
  constexpr
  std::vector<any_composition>::const_iterator end() const
  noexcept
  {
    return compositions_.end();
  }


  /**
   * @return A const_iterator to the first any_composition in the composer.
   */
  [[nodiscard]]
  constexpr
  std::vector<any_composition>::const_iterator cbegin() const
  noexcept
  {
    return compositions_.cbegin();
  }

  /**
   * @return A const iterator to after the last any_composition in the
   *     composer.
   *
   * @note This returned const_iterator only acts as a sentinel, and is not to
   *     be dereferenced.
   */
  [[nodiscard]]
  constexpr
  std::vector<any_composition>::const_iterator cend() const
  noexcept
  {
    return compositions_.cend();
  }



  /**
   * @tparam Composition The type of composition to get the index of.
   * @return The index of the given composition type.
   */
  template<typename Composition>
  requires composition_specialization<Composition>
  [[nodiscard]]
  constexpr
  static std::size_t index()
  noexcept
  {
    static std::size_t idx = next_index_++;
    return idx;
  }


  /**
   * @tparam Composition The type of composition to check for.
   * @return @c true if this type of composition exists in the composer,
   *     @c false otherwise.
   */
  template<typename Composition>
  requires composition_specialization<Composition>
  [[nodiscard]]
  constexpr
  bool composes() const
  noexcept
  {
    std::size_t const idx = index<Composition>();

    return compositions_.size() > idx
        && compositions_[idx].has_value();
  }


  /**
   * @tparam Composition The type of composition to get a reference of.
   * @return A reference to the composition of the given type.
   */
  template<typename Composition>
  requires composition_specialization<Composition>
  [[nodiscard]]
  constexpr
  Composition       &get()
  noexcept
  {
    return compositions_[index<Composition>()].template get<Composition>();
  }
  /**
   * @tparam Composition The type of composition to get a const reference of.
   * @return A const reference to the composition of the given type.
   */
  template<typename Composition>
  requires composition_specialization<Composition>
  [[nodiscard]]
  constexpr
  Composition const &get() const
  noexcept
  {
    return compositions_[index<Composition>()].template get<Composition>();
  }



  /**
   * @brief Adds a new composition to the composer, only if a composition
   *     of its type is not already contained.
   *
   * @tparam Composition The type of composition to compose.
   * @param composition The composition to add to the composer.
   * @return @c true if this type of composition is not already composed,
   *     @c false otherwise.
   */
  template<typename Composition>
  requires composition_specialization<Composition>
  constexpr
  bool compose(Composition &&composition)
  {
    std::size_t const idx = index<Composition>();

    if (composes<Composition>())
      return false;

    if (compositions_.size() <= idx)
      compositions_.resize(idx + 1);
    compositions_[idx].template emplace<Composition>(
        std::forward<Composition>(composition));

    return true;
  }



  /**
   * @brief Swaps the contents of @p *this and @code other@endcode.
   *
   * @param other The other composer whose contents to swap.
   */
  constexpr
  void swap(composer &other)
  noexcept
  {
    compositions_.swap(other.compositions_);
  }

private:
  inline static std::size_t    next_index_   = 0;
  std::vector<any_composition> compositions_;

};


/**
 * @brief Swaps the contents of @code lhs@endcode and @code rhs@endcode.
 *
 * @param lhs The first  composer whose contents to swap.
 * @param rhs The second composer whose contents to swap.
 */
constexpr
void swap(composer &lhs, composer &rhs)
noexcept
{
  lhs.swap(rhs);
}

}

#endif // HEIM_COMPOSITION_COMPOSER_HPP
