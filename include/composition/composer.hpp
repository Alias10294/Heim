#ifndef HEIM_COMPOSITION_COMPOSER_HPP
#define HEIM_COMPOSITION_COMPOSER_HPP

#include "core/specialization_of.hpp"
#include "any_composition.hpp"
#include "composition.hpp"

namespace heim
{
class composer
{
public:
  constexpr
  void swap(composer &other)
  noexcept
  {
    compositions_.swap(other.compositions_);
  }



  [[nodiscard]]
  constexpr
  std::vector<any_composition>::iterator       begin()
  noexcept
  {
    return compositions_.begin();
  }
  [[nodiscard]]
  constexpr
  std::vector<any_composition>::const_iterator begin() const
  noexcept
  {
    return compositions_.begin();
  }

  [[nodiscard]]
  constexpr
  std::vector<any_composition>::iterator       end()
  noexcept
  {
    return compositions_.end();
  }
  [[nodiscard]]
  constexpr
  std::vector<any_composition>::const_iterator end() const
  noexcept
  {
    return compositions_.end();
  }


  [[nodiscard]]
  constexpr
  std::vector<any_composition>::const_iterator cbegin() const
  noexcept
  {
    return compositions_.cbegin();
  }

  [[nodiscard]]
  constexpr
  std::vector<any_composition>::const_iterator cend() const
  noexcept
  {
    return compositions_.cend();
  }



  template<typename Composition>
  requires core::specialization_of<Composition, composition>
  [[nodiscard]]
  constexpr
  static std::size_t index()
  noexcept
  {
    static std::size_t idx = next_index_++;
    return idx;
  }


  template<typename Composition>
  requires core::specialization_of<Composition, composition>
  [[nodiscard]]
  constexpr
  bool composes() const
  noexcept
  {
    std::size_t const idx = index<Composition>();

    return compositions_.size() > idx
        && compositions_[idx].has_value();
  }


  template<typename Composition>
  requires core::specialization_of<Composition, composition>
  [[nodiscard]]
  constexpr
  Composition       &get()
  noexcept
  {
    return compositions_[index<Composition>()].template get<Composition>();
  }
  template<typename Composition>
  requires core::specialization_of<Composition, composition>
  [[nodiscard]]
  constexpr
  Composition const &get() const
  noexcept
  {
    return compositions_[index<Composition>()].template get<Composition>();
  }



  template<typename Composition>
  requires core::specialization_of<Composition, composition>
  constexpr
  bool compose(Composition &&composition)
  {
    std::size_t const idx = index<Composition>();

    if (composes<Composition>())
      return false;

    if (compositions_.size() <= idx)
      compositions_.resize(idx + 1);
    compositions_[idx].emplace(std::forward<Composition>(composition));

    return true;
  }

private:
  inline static std::size_t    next_index_   = 0;
  std::vector<any_composition> compositions_;

};

}

#endif // HEIM_COMPOSITION_COMPOSER_HPP
