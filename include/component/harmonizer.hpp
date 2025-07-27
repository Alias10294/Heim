#ifndef HEIM_COMPONENT_HARMONIZER_HPP
#define HEIM_COMPONENT_HARMONIZER_HPP

#include <cstddef>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>
#include "any_harmonized.hpp"
#include "composition.hpp"

namespace heim
{
template<typename Entity>
requires std::unsigned_integral<Entity>
class harmonizer
{
public:
  using entity_type = Entity;

private:
  using harmonized_container_type = std::vector<any_harmonized>;
  using index_container_type = std::unordered_map<std::type_index, std::size_t>;

public:
  template<typename Composition>
  requires specialization_of<Composition, composition>
  [[nodiscard]]
  constexpr
  static std::type_index type_index()
  noexcept
  {
    return std::type_index(typeid(Composition));
  }
  template<typename ...Compositions>
  requires (sizeof...(Compositions) > 1)
        && (specialization_of<Compositions, composition> && ...)
  [[nodiscard]]
  constexpr
  static std::type_index type_index()
  noexcept
  {
    return std::type_index(typeid(harmonized<entity_type, Compositions ...>));
  }

  template<typename Composition>
  requires specialization_of<Composition, composition>
  [[nodiscard]]
  constexpr
  std::size_t index() const
  noexcept
  {
    return composition_indexes_.find(type_index<Composition>())->second;
  }
  template<typename ...Compositions>
  requires (sizeof...(Compositions) > 1)
        && (specialization_of<Compositions, composition> && ...)
  [[nodiscard]]
  constexpr
  std::size_t index() const
  noexcept
  {
    return harmonized_indexes_.find(type_index<Compositions ...>())->second;
  }


  template<typename Composition>
  requires specialization_of<Composition, composition>
  [[nodiscard]]
  constexpr
  bool holds() const
  noexcept
  {
    return composition_indexes_.contains(type_index<Composition>());
  }
  template<typename ...Compositions>
  requires (sizeof...(Compositions) > 1)
        && (specialization_of<Compositions, composition> && ...)
  [[nodiscard]]
  constexpr
  bool holds() const
  noexcept
  {
    return harmonized_indexes_.contains(type_index<Compositions ...>());
  }


  template<typename ...Compositions>
  requires (sizeof...(Compositions) > 1)
        && (specialization_of<Compositions, composition> && ...)
  [[nodiscard]]
  constexpr
  harmonized<entity_type, Compositions ...>       &get()
  noexcept
  {
    return harmonized_[index<Compositions ...>()].template
        get<entity_type, Compositions ...>();
  }
  template<typename ...Compositions>
  requires (sizeof...(Compositions) > 1)
        && (specialization_of<Compositions, composition> && ...)
  [[nodiscard]]
  constexpr
  harmonized<entity_type, Compositions ...> const &get() const
  noexcept
  {
    return harmonized_[index<Compositions ...>()].template
        get<entity_type, Compositions ...>();
  }



  template<typename ...Compositions>
  requires (sizeof...(Compositions) > 1)
        && (specialization_of<Compositions, composition> && ...)
  constexpr
  bool harmonize(Compositions &...compositions)
  {
    if (holds<Compositions...>())
      return false;
    if ((holds<Compositions>() || ...))
      return false;

    (composition_indexes_.emplace(
        type_index<Compositions>(),
        harmonized_.size()),
     ...);
    harmonized_indexes_.emplace(
        type_index<Compositions ...>(),
        harmonized_.size());

    try
    {
      harmonized_.emplace_back(
          std::in_place_type_t<entity_type>{},
          compositions...);
    }
    catch (...)
    {
      (composition_indexes_.erase(type_index<Compositions>()), ...);
      harmonized_indexes_.erase(type_index<Compositions ...>());
      throw;
    }
    return true;
  }

  template<typename ...Compositions>
  requires (sizeof...(Compositions) > 1)
        && (specialization_of<Compositions, composition> && ...)
  constexpr
  bool separate()
  noexcept
  {
    if (!holds<Compositions...>())
      return false;

    std::size_t const idx = index<Compositions ...>();
    std::size_t const back_idx = harmonized_indexes_.size() - 1;
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

    (composition_indexes_.erase(type_index<Compositions>()), ...);
    harmonized_indexes_.erase(type_index<Compositions ...>());

    harmonized_.pop_back();

    return true;
  }

private:
  harmonized_container_type harmonized_;

  index_container_type harmonized_indexes_;
  index_container_type composition_indexes_;
  // TODO: ^ maybe try inverting it to optimise harmonize and separate

};

}

#endif // HEIM_COMPONENT_HARMONIZER_HPP
