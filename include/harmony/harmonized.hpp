#ifndef HEIM_HARMONY_HARMONIZED_HPP
#define HEIM_HARMONY_HARMONIZED_HPP

#include <algorithm>
#include <concepts>
#include "component/composition.hpp"
#include "utils/specialization_of.hpp"

namespace heim
{
template<typename    Entity,
         typename ...Compositions>
requires  std::unsigned_integral<Entity>
      && (specialization_of<Compositions, composition> && ...)
class harmonized
{
public:
  using entity_type = Entity;

private:
  template<typename Composition>
  requires specialization_of<Composition, composition>
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



    constexpr
    void operator()(entity_type const e, std::ptrdiff_t len)
    {
      composition_.swap(e, composition_.begin() + len);
    }

  private:
    composition_type &composition_;

  };

public:
  using observer_tuple_type = std::tuple<observer<Compositions>...>;

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



  constexpr
  void include(entity_type const e)
  {
    notify(e, std::index_sequence_for<Compositions...>{});
    ++length_;
  }

  constexpr
  void exclude(entity_type const e)
  {
    --length_;
    notify(e, std::index_sequence_for<Compositions...>{});
  }

private:
  template<typename    Head,
           typename ...Tail>
  requires  specialization_of<Head, composition>
        && (specialization_of<Tail, composition> && ...)
  constexpr
  auto &min(Head &head, Tail &...tail)
  {
    auto* min = &head;
    ((min = tail.size() < min->size() ? &tail : min), ...);
    return *min;
  }


  constexpr
  void harmonize(Compositions const &...compositions)
  {
    auto &pivot = min(compositions...);

    if (pivot.empty())
      return;

    auto it = pivot.end();
    while (--it >= pivot.begin() + length_)
    {
      entity_type const e = (*it).entity;
      if ((compositions.contains(e) && ...))
        include(e);
    }
  }



  template<std::size_t ...Is>
  constexpr
  void notify(entity_type const e, std::index_sequence<Is...>)
  {
    (std::get<Is>(observers_)(e, length_), ...);
  }

private:
  observer_tuple_type observers_;
  std::ptrdiff_t      length_;

};

}

#endif // HEIM_HARMONY_HARMONIZED_HPP
