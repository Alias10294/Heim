#ifndef HEIM_GLIMPSE_GENERIC_GLIMPSE_HPP
#define HEIM_GLIMPSE_GENERIC_GLIMPSE_HPP

#include <tuple>
#include "entity.hpp"

namespace heim
{
template<typename    Entity,
         typename ...Ts>
requires entity<Entity>
class generic_glimpse
{
public:
  using entity_type          = Entity;
  using container_tuple_type = std::tuple<Ts ...>;

private:
  template<bool IsConst>
  class generic_iterator
  {
  public:
    constexpr
    static std::size_t is_const = IsConst;

  public:
    constexpr
    generic_iterator()
    = default;
    constexpr
    generic_iterator(generic_iterator const &)
    = default;
    constexpr
    generic_iterator(generic_iterator &&)
    noexcept
    = default;

    constexpr
    ~generic_iterator()
    noexcept
    = default;


    constexpr
    generic_iterator& operator=(generic_iterator const &)
    = default;
    constexpr
    generic_iterator& operator=(generic_iterator &&)
    noexcept
    = default;

  };

public:
  using iterator       = generic_iterator<false>;
  using const_iterator = generic_iterator<true>;

public:
  constexpr
  generic_glimpse()
  = default;
  constexpr
  generic_glimpse(generic_glimpse const &)
  = default;
  constexpr
  generic_glimpse(generic_glimpse &&)
  noexcept
  = default;

  constexpr
  ~generic_glimpse()
  noexcept
  = default;


  constexpr
  generic_glimpse& operator=(generic_glimpse const &)
  = default;
  constexpr
  generic_glimpse& operator=(generic_glimpse &&)
  noexcept
  = default;

private:


};

}

#endif // HEIM_GLIMPSE_GENERIC_GLIMPSE_HPP
