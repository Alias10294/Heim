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
  /**
   * @return An iterator to the first any_harmony in the harmonizer.
   */
  [[nodiscard]]
  constexpr
  std::vector<any_harmony>::iterator       begin()
  noexcept
  {
    return harmonies_.begin();
  }
  /**
   * @return A const iterator to the first any_harmony in the harmonizer.
   */
  [[nodiscard]]
  constexpr
  std::vector<any_harmony>::const_iterator begin() const
  noexcept
  {
    return harmonies_.begin();
  }

  /**
   * @return An iterator to after the last any_harmony in the harmonizer.
   *
   * @note This returned iterator only acts as a sentinel, and is not to be
   *     dereferenced.
   */
  [[nodiscard]]
  constexpr
  std::vector<any_harmony>::iterator       end()
  noexcept
  {
    return harmonies_.end();
  }
  /**
   * @return A const iterator to after the last any_harmony in the harmonizer.
   *
   * @note This returned const iterator only acts as a sentinel, and is not to
   *     be dereferenced.
   */
  [[nodiscard]]
  constexpr
  std::vector<any_harmony>::const_iterator end() const
  noexcept
  {
    return harmonies_.end();
  }


  /**
   * @return A const iterator to the first any_harmony in the harmonizer.
   */
  [[nodiscard]]
  constexpr
  std::vector<any_harmony>::const_iterator cbegin() const
  noexcept
  {
    return harmonies_.cbegin();
  }

  /**
   * @return A const iterator to after the last any_harmony in the harmonizer.
   *
   * @note This returned const iterator only acts as a sentinel, and is not to
   *     be dereferenced.
   */
  [[nodiscard]]
  constexpr
  std::vector<any_harmony>::const_iterator cend() const
  noexcept
  {
    return harmonies_.cend();
  }



  /**
   * @tparam Harmony The type of harmony to get the index of.
   * @return The index of the given harmony type.
   */
  template<typename Harmony>
  requires harmony_specialization<Harmony>
  [[nodiscard]]
  constexpr
  static std::size_t index()
  noexcept
  {
    static std::size_t idx = next_harmony_index_++;
    return idx;
  }
  /**
   * @tparam T The type of container to get the index of.
   * @return The index of the given container type.
   */
  template<typename T>
  requires observable<T>
  [[nodiscard]]
  constexpr
  static std::size_t index()
  noexcept
  {
    static std::size_t idx = next_container_index_++;
    return idx;
  }


  /**
   * @tparam Harmony The type of harmony to check for.
   * @return @c true if an instance of the given harmony is in the harmonizer,
   *     @c false otherwise.
   */
  template<typename Harmony>
  requires harmony_specialization<Harmony>
  [[nodiscard]]
  constexpr
  bool harmonizes() const
  noexcept
  {
    std::size_t const idx = index<Harmony>();

    return harmonies_.size() > idx
        && harmonies_[idx].has_value();
  }
  /**
   * @tparam T The type of container to check for.
   * @return @c true if any harmony harmonizes a container of this type,
   *    @c false otherwise.
   */
  template<typename T>
  requires observable<T>
  [[nodiscard]]
  constexpr
  bool harmonizes() const
  noexcept
  {
    std::size_t const idx = index<T>();

    return container_indexes_.size() > idx
        && container_indexes_[idx] != null_idx_;
  }


  /**
   * @tparam Harmony The type of the harmony to get a reference of.
   * @return A reference to the contained harmony of this type.
   */
  template<typename Harmony>
  requires harmony_specialization<Harmony>
  [[nodiscard]]
  constexpr
  Harmony       &get()
  noexcept
  {
    return harmonies_[index<Harmony>()].template get<Harmony>();
  }
  /**
   * @tparam Harmony The type of the harmony to get a const reference of.
   * @return A const reference to the contained harmony of this type.
   */
  template<typename Harmony>
  requires harmony_specialization<Harmony>
  [[nodiscard]]
  constexpr
  Harmony const &get() const
  noexcept
  {
    return harmonies_[index<Harmony>()].template get<Harmony>();
  }



  /**
   * @tparam Harmony The type of harmony to harmonize.
   * @param harmony The harmony to add to the harmonizer.
   * @return @c true if a harmony has been added to the harmonizer, @c false
   *     otherwise.
   */
  template<typename Harmony>
  requires harmony_specialization<Harmony>
  constexpr
  bool harmonize(Harmony &&harmony)
  {
    using container_tuple_type = typename Harmony::container_tuple_type;

    if (harmonizes<Harmony>())
      return false;
    if (harmonizes_for<Harmony>(
        std::make_index_sequence<std::tuple_size_v<container_tuple_type>>{}))
      return false;

    std::size_t const idx = index<Harmony>();

    if (harmonies_.size() <= idx)
      harmonies_.resize(idx + 1);
    harmonies_[idx].emplace<Harmony>(std::forward<Harmony>(harmony));

    try
    {
      std::size_t const new_size = max_index<Harmony>(
          std::make_index_sequence<std::tuple_size_v<container_tuple_type>>{});
      if (container_indexes_.size() <= new_size)
        container_indexes_.resize(new_size + 1, null_idx_);

      harmonize_for<Harmony>(
          std::make_index_sequence<std::tuple_size_v<container_tuple_type>>{});
    }
    catch (...)
    {
      harmonies_[idx].reset();
      throw;
    }

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
    harmonies_       .swap(other.harmonies_);
    container_indexes_.swap(other.container_indexes_);
  }

private:
  /// @cond INTERNAL

  /**
   * @tparam Harmony The type of harmony to get the max index of container.
   *     from.
   * @tparam Is      The indexes of the tuple of containers of the harmony.
   * @return The largest index between each container of the harmony.
   */
  template<typename Harmony,
           std::size_t ...Is>
  requires harmony_specialization<Harmony>
  [[nodiscard]]
  constexpr
  static std::size_t max_index(std::index_sequence<Is ...>)
  noexcept
  {
    using container_tuple_type = typename Harmony::container_tuple_type;

    std::size_t max_idx =
        index<std::tuple_element_t<0, container_tuple_type>>();

    ((max_idx =
         max_idx < index<std::tuple_element_t<Is, container_tuple_type>>()
       ? index<std::tuple_element_t<Is, container_tuple_type>>()
       : max_idx),
     ...);

    return max_idx;
  }



  /**
   * @tparam Harmony The type of harmony to get the max index of container
   *     from.
   * @tparam Is      The indexes of the tuple of containers of the harmony.
   * @return @c true if any of the containers' type is harmonized by any
   *     harmony in the harmonizer, @c false otherwise.
   */
  template<typename    Harmony,
           std::size_t ...Is>
  [[nodiscard]]
  constexpr
  bool harmonizes_for(std::index_sequence<Is ...>) const
  noexcept
  {
    return (harmonizes<
        std::tuple_element_t<Is, typename Harmony::container_tuple_type>>()
     || ...);
  }



  /**
   * @brief Updates the indexes of each of the container's types to reflect
   *     which harmony index they are linked to.
   *
   * @tparam Harmony The type of harmony to get the max index of container.
   *     from.
   * @tparam Is      The indexes of the tuple of containers of the harmony.
   */
  template<typename       Harmony,
           std::size_t ...Is>
  requires harmony_specialization<Harmony>
  constexpr
  void harmonize_for(std::index_sequence<Is ...>)
  noexcept
  {
    using container_tuple_type = typename Harmony::container_tuple_type;

    ((container_indexes_[
         index<std::tuple_element_t<Is, container_tuple_type>>()] =
             index<Harmony>()),
     ...);
  }

private:
  inline static std::size_t next_harmony_index_  = 0;
  inline static std::size_t next_container_index_ = 0;

  std::vector<any_harmony>  harmonies_;
  std::vector<std::size_t>  container_indexes_;

};


/**
 * @brief Swaps the contents of @code lhs@endcode and @code rhs@endcode.
 *
 * @param lhs The first  harmonizer whose contents to swap.
 * @param rhs The second harmonizer whose contents to swap.
 */
constexpr
void swap(harmonizer &lhs, harmonizer &rhs)
noexcept
{
  lhs.swap(rhs);
}

}

#endif // HEIM_HARMONY_HARMONIZER_HPP
