#ifndef HEIM_HARMONY_HARMONY_HPP
#define HEIM_HARMONY_HARMONY_HPP

#include <algorithm>
#include "core/entity.hpp"
#include "core/specialization_of.hpp"
#include "arranger.hpp"

namespace heim
{
template<typename    Entity,
         typename ...Ts>
requires  core::entity<Entity>
      && (sizeof...(Ts) > 1)
      && (arrangeable<Ts>                                  && ...)
      && (std::is_same_v<typename Ts::entity_type, Entity> && ...)
class harmony
{
public:
  using entity_type         = Entity;
  using arranged_tuple_type = std::tuple<Ts ...>;

public:
  constexpr
  harmony(harmony const &)
  = default;
  constexpr
  harmony(harmony &&)
  noexcept
  = default;
  constexpr
  harmony(Ts &...arranged)
    : arrangements_{arranger<Ts>(arranged)...},
      length_   {0}
  {
    harmonize(arranged...);
  }

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
  noexcept(noexcept(arrange(0, std::index_sequence_for<Ts...>{})))
  {
    arrange(e, std::index_sequence_for<Ts...>{});
    ++length_;
  }

  /**
   * @brief Excludes @code e@endcode of the group of common entities.
   *
   * @param e The entity to exclude.
   */
  constexpr
  void exclude(entity_type const e)
  noexcept(noexcept(arrange(0, std::index_sequence_for<Ts...>{})))
  {
    --length_;
    arrange(e, std::index_sequence_for<Ts...>{});
  }

private:
  /// @cond INTERNAL

  template<typename    Head,
           typename ...Tail>
  requires  arrangeable<Head>
        &&  std::is_same_v<typename Head::entity_type, entity_type>
        && (arrangeable<Tail>                                       && ...)
        && (std::is_same_v<typename Tail::entity_type, entity_type> && ...)
  constexpr
  auto &pivot(Head const &head, Tail const &...tail)
  noexcept
  {
    auto *min = &head;
    ((min = tail.size() < min->size() ? &tail : min), ...);
    return *min;
  }


  constexpr
  void harmonize(Ts &...arranged)
  noexcept(noexcept(include(0)))
  {
    auto &p = pivot(arranged...);

    if (p.empty())
      return;

    auto it = p.end();
    while (it != p.begin() + length_)
    {
      entity_type const e = std::get<0>(*--it);
      if ((arranged.contains(e) && ...))
        include(e);
    }
  }



  template<std::size_t ...Is>
  constexpr
  void arrange(entity_type const e, std::index_sequence<Is...>)
  noexcept((noexcept(std::get<Is>(arrangements_)(
      std::declval<entity_type>(),
      std::declval<std::ptrdiff_t>()))
   && ...))
  {
    (std::get<Is>(arrangements_)(e, length_), ...);
  }

  /// @endcond

private:
  std::tuple<arranger<Ts> ...> arrangements_;
  std::ptrdiff_t                  length_;

};

}

#endif // HEIM_HARMONY_HARMONY_HPP
