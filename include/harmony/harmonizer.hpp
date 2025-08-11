#ifndef HEIM_HARMONY_HARMONIZER_HPP
#define HEIM_HARMONY_HARMONIZER_HPP

#include <limits>
#include <tuple>
#include <utility>
#include <vector>
#include "any_harmony.hpp"
#include "harmony.hpp"

namespace heim
{
class harmonizer
{
private:
  constexpr
  static std::size_t null_idx_ = std::numeric_limits<std::size_t>::max();

public:
  constexpr
  void swap(harmonizer &other)
  noexcept
  {
    harmonies_          .swap(other.harmonies_);
    composition_indexes_.swap(other.composition_indexes_);
  }



  [[nodiscard]]
  constexpr
  std::vector<any_harmony>::iterator       begin()
  noexcept
  {
    return harmonies_.begin();
  }
  [[nodiscard]]
  constexpr
  std::vector<any_harmony>::const_iterator begin() const
  noexcept
  {
    return harmonies_.begin();
  }

  [[nodiscard]]
  constexpr
  std::vector<any_harmony>::iterator       end()
  noexcept
  {
    return harmonies_.end();
  }
  [[nodiscard]]
  constexpr
  std::vector<any_harmony>::const_iterator end() const
  noexcept
  {
    return harmonies_.end();
  }


  [[nodiscard]]
  constexpr
  std::vector<any_harmony>::const_iterator cbegin() const
  noexcept
  {
    return harmonies_.cbegin();
  }
  [[nodiscard]]
  constexpr
  std::vector<any_harmony>::const_iterator cend() const
  noexcept
  {
    return harmonies_.cend();
  }



  template<typename Harmony>
  requires core::specialization_of<Harmony, harmony>
  [[nodiscard]]
  constexpr
  static std::size_t index()
  noexcept
  {
    static std::size_t idx = next_harmony_index_++;
    return idx;
  }
  template<typename Composition>
  requires core::specialization_of<Composition, composition>
  [[nodiscard]]
  constexpr
  static std::size_t index()
  noexcept
  {
    static std::size_t idx = next_composition_index_++;
    return idx;
  }


  template<typename Harmony>
  requires core::specialization_of<Harmony, harmony>
  [[nodiscard]]
  constexpr
  bool harmonizes() const
  noexcept
  {
    std::size_t const idx = index<Harmony>();

    return harmonies_.size() > idx
        && harmonies_[idx].has_value();
  }
  template<typename Composition>
  requires core::specialization_of<Composition, composition>
  [[nodiscard]]
  constexpr
  bool harmonizes() const
  noexcept
  {
    std::size_t const idx = index<Composition>();

    return composition_indexes_.size() > idx
        && composition_indexes_[idx] != null_idx_;
  }



  template<typename Harmony>
  requires core::specialization_of<Harmony, harmony>
  constexpr
  bool harmonize(Harmony &&harmony)
  {
    using compositions_type = typename Harmony::compositions_type;

    if (harmonizes<Harmony>())
      return false;
    if (harmonizes_for<Harmony>(
        std::make_index_sequence<std::tuple_size_v<compositions_type>>{}))
      return false;

    std::size_t const idx = index<Harmony>();

    if (harmonies_.size() <= idx)
      harmonies_.resize(idx + 1);
    harmonies_[idx].emplace<Harmony>(std::forward<Harmony>(harmony));

    try
    {
      std::size_t const new_size = max_index<Harmony>(
          std::make_index_sequence<std::tuple_size_v<compositions_type>>{});
      if (composition_indexes_.size() <= new_size)
        composition_indexes_.resize(new_size + 1, null_idx_);

      harmonize_for<Harmony>(
          std::make_index_sequence<std::tuple_size_v<compositions_type>>{});
    }
    catch (...)
    {
      harmonies_[idx].reset();
      throw;
    }

    return true;
  }

private:
  template<typename Harmony,
           std::size_t ...Is>
  requires core::specialization_of<Harmony, harmony>
  [[nodiscard]]
  constexpr
  static std::size_t max_index(std::index_sequence<Is ...>)
  noexcept
  {
    using compositions_type = typename Harmony::compositions_type;

    std::size_t max_idx = index<std::tuple_element_t<0, compositions_type>>();

    ((max_idx < index<std::tuple_element_t<Is, compositions_type>>()
       ? max_idx = index<std::tuple_element_t<Is, compositions_type>>()
       : void()),
     ...);

    return max_idx;
  }



  template<typename    Harmony,
           std::size_t ...Is>
  [[nodiscard]]
  constexpr
  bool harmonizes_for(std::index_sequence<Is ...>) const
  noexcept
  {
    return (harmonizes<
        std::tuple_element_t<Is, typename Harmony::compositions_type>>()
     || ...);
  }



  template<typename       Harmony,
           std::size_t ...Is>
  requires core::specialization_of<Harmony, harmony>
  constexpr
  void harmonize_for(std::index_sequence<Is ...>)
  noexcept
  {
    using compositions_type = typename Harmony::compositions_type;

    ((composition_indexes_[index<std::tuple_element_t<Is, compositions_type>>()]
         = index<Harmony>()),
     ...);
  }

private:
  inline static std::size_t next_harmony_index_     = 0;
  inline static std::size_t next_composition_index_ = 0;

  std::vector<any_harmony>  harmonies_;
  std::vector<std::size_t>  composition_indexes_;

};

}

#endif // HEIM_HARMONY_HARMONIZER_HPP
