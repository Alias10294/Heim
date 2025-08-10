#ifndef BASIC_GLIMPSE_HPP
#define BASIC_GLIMPSE_HPP

#include <algorithm>

#include "components/composition.hpp"
#include "core/specialization_of.hpp"

namespace heim
{
template<typename    Entity,
         typename ...Compositions>
requires  core::entity<Entity>
      && (core::specialization_of<Compositions, composition>         && ...)
      && (std::is_same_v<typename Compositions::entity_type, Entity> && ...)
class basic_glimpse
{
public:
  using entity_type = Entity;

private:
  template<bool IsConst>
  class generic_iterator
  {
  public:
    constexpr
    static bool is_const = IsConst;

  private:
    using composition_tuple_type = std::conditional_t<is_const,
        std::tuple<Compositions &...> const,
        std::tuple<Compositions &...>>;

  public:
    using proxy_type = std::tuple<
        Entity &,
        typename Compositions::component_type &...>;

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
    explicit
    constexpr
    generic_iterator(
        std::tuple<Compositions &...> const &compositions,
        std::size_t const idx = 0)
      : compositions_{compositions},
        idx_         {idx}
    {
      update_pivot();
    }

    constexpr
    ~generic_iterator()
    noexcept
    = default;


    constexpr
    generic_iterator &operator=(generic_iterator const &)
    = default;
    constexpr
    generic_iterator &operator=(generic_iterator &&)
    noexcept
    = default;



    constexpr
    void swap(generic_iterator &other)
    noexcept
    {
      std::swap(compositions_, other.compositions_);
    }

  private:
    template<std::size_t ...Is>
    constexpr
    void update_pivot_for(std::index_sequence<Is ...>) const
    noexcept
    {
      pivot_idx_             = 0;
      std::size_t pivot_size = std::get<pivot_idx_>(compositions_).size();

      ((std::get<Is>(compositions_).size() < pivot_size
           ? (pivot_idx_ = Is,
              pivot_size = std::get<Is>(compositions_).size())
           : void()),
           ...);
    }

    constexpr
    void update_pivot() const
    noexcept
    {
      update_pivot_for(std::make_index_sequence<
          std::tuple_size_v<composition_tuple_type>>{});
    }

  private:
    composition_tuple_type &compositions_;
    std::size_t             idx_;

    static std::size_t pivot_idx_;

  };

public:
  constexpr
  basic_glimpse()
  = default;
  constexpr
  basic_glimpse(basic_glimpse const &)
  = default;
  constexpr
  basic_glimpse(basic_glimpse &&)
  noexcept
  = default;
  explicit
  constexpr
  basic_glimpse(Compositions &... compositions)
  noexcept
    : compositions_{compositions...}
  { }

  constexpr
  ~basic_glimpse()
  noexcept
  = default;


  constexpr
  basic_glimpse &operator=(basic_glimpse const &)
  = default;
  constexpr
  basic_glimpse &operator=(basic_glimpse &&)
  noexcept
  = default;

private:
  std::tuple<Compositions &...> compositions_;

};

}

#endif // BASIC_GLIMPSE_HPP
