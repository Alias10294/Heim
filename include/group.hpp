#ifndef HEIM_GROUP_HPP
#define HEIM_GROUP_HPP

#include "core/type_sequence.hpp"
#include "map.hpp"

namespace heim
{
template<typename ...Maps>
class group
{
  static_assert(
      sizeof...(Maps) > 1,
      "heim::group: Maps ... must be a sequence of more than one type. ");
  static_assert(
      (std::is_same_v<Maps, std::remove_cvref_t<Maps>> && ...),
      "heim::group: Maps ... must each be cv-unqualified non-reference "
      "types. ");
  static_assert(
      (is_map_v<Maps> && ...),
      "heim::group: Maps ... must each specialize heim::map. ");
  static_assert(
      type_sequence<typename Maps::entity_type ...>::unique_t::size_v == 1,
      "heim::group: Maps ... must share the same entity_type. ");
  static_assert(
      std::is_same_v<
          type_sequence<Maps ...>,
          typename type_sequence<Maps ...>::unique_t>,
      "heim::group: Maps ... must be a set of distinct types. ");

public:
  using map_sequence
  = type_sequence<Maps ...>;

  using entity_type
  = typename type_sequence<typename Maps::entity_type ...>
    ::template get_t<0>;

private:
  std::tuple<Maps ...> maps_;
  std::size_t          size_ = 0;

private:
  template<typename Map>
  [[nodiscard]]
  constexpr
  Map       &get()
  noexcept
  {
    static_assert(
        map_sequence::template contains_v<Map>,
        "heim::group::get(): Map does not appear in map_sequence. ");

    return get<Map>();
  }

  template<typename Map>
  [[nodiscard]]
  constexpr
  Map const &get() const 
  noexcept
  {
    static_assert(
        map_sequence::template contains_v<Map>,
        "heim::group::get() const: Map does not appear in map_sequence. ");

    return get<Map>();
  }
  


  constexpr
  void notify(entity_type const e)
  {
    (get<Maps>().swap(
        e,
        std::get<0>(*(get<Maps>().begin() + size_))), ...);
  }


  constexpr
  void include(entity_type const e)
  {
    if ((get<Maps>().contains(e) && ...))
    {
      notify(e);
      ++size_;
    }
  }

  constexpr
  void exclude(entity_type const e)
  {
    --size_;
    notify(e);
  }

public:
  [[nodiscard]]
  constexpr
  std::size_t size() const
  noexcept
  {
    return size_;
  }

  template<typename Map>
  [[nodiscard]]
  constexpr
  typename Map::size_type size() const
  noexcept
  {
    static_assert(
        map_sequence::template contains_v<Map>,
        "heim::group::size() const: Map does not appear in map_sequence. ");

    return get<Map>().size();
  }


  template<typename Map>
  [[nodiscard]]
  constexpr
  bool empty() const
  noexcept
  {
    static_assert(
        map_sequence::template contains_v<Map>,
        "heim::group::empty() const: Map does not appear in map_sequence. ");

    return get<Map>().empty();
  }


  template<typename Map>
  [[nodiscard]]
  constexpr
  typename Map::size_type max_size() const
  noexcept
  {
    static_assert(
        map_sequence::template contains_v<Map>,
        "heim::group::max_size() const: Map does not appear in map_sequence. ");

    return get<Map>().max_size();
  }


  template<typename Map>
  [[nodiscard]]
  constexpr
  typename Map::size_type capacity() const
  noexcept
  {
    static_assert(
        map_sequence::template contains_v<Map>,
        "heim::group::capacity() const: Map does not appear in map_sequence. ");

    return get<Map>().capacity();
  }


  template<typename Map>
  constexpr
  void reserve(typename Map::size_type const n)
  {
    static_assert(
        map_sequence::template contains_v<Map>,
        "heim::group::reserve(typename Map::size_type const): Map does not "
        "appear in map_sequence. ");

    get<Map>().reserve(n);
  }


  template<typename Map>
  constexpr
  void shrink_to_fit()
  {
    static_assert(
        map_sequence::template contains_v<Map>,
        "heim::group::shrink_to_fit(): Map does not appear in map_sequence. ");

    get<Map>().shrink_to_fit();
  }



  template<typename Map>
  [[nodiscard]]
  constexpr
  typename Map::iterator       begin()
  noexcept
  {
    static_assert(
        map_sequence::template contains_v<Map>,
        "heim::group::begin(): Map does not appear in map_sequence. ");

    return get<Map>().begin();
  }

  template<typename Map>
  [[nodiscard]]
  constexpr
  typename Map::const_iterator begin() const
  noexcept
  {
    static_assert(
        map_sequence::template contains_v<Map>,
        "heim::group::begin() const: Map does not appear in map_sequence. ");

    return get<Map>().begin();
  }


  template<typename Map>
  [[nodiscard]]
  constexpr
  typename Map::iterator       end()
  noexcept
  {
    static_assert(
        map_sequence::template contains_v<Map>,
        "heim::group::end(): Map does not appear in map_sequence. ");

    return get<Map>().end();
  }

  template<typename Map>
  [[nodiscard]]
  constexpr
  typename Map::const_iterator end() const
  noexcept
  {
    static_assert(
        map_sequence::template contains_v<Map>,
        "heim::group::end() const: Map does not appear in map_sequence. ");

    return get<Map>().end();
  }


  template<typename Map>
  [[nodiscard]]
  constexpr
  typename Map::const_iterator cbegin() const
  noexcept
  {
    static_assert(
        map_sequence::template contains_v<Map>,
        "heim::group::cbegin() const: Map does not appear in map_sequence. ");

    return get<Map>().cbegin();
  }


  template<typename Map>
  [[nodiscard]]
  constexpr
  typename Map::const_iterator cend() const
  noexcept
  {
    static_assert(
        map_sequence::template contains_v<Map>,
        "heim::group::cend() const: Map does not appear in map_sequence. ");

    return get<Map>().cend();
  }



  template<typename Map>
  [[nodiscard]]
  constexpr
  bool contains(entity_type const e) const
  noexcept
  {
    static_assert(
        map_sequence::template contains_v<Map>,
        "heim::group::contains(entity_type const) const: Map does not appear "
        "in map_sequence. ");

    return get<Map>().contains(e);
  }


  template<typename Map>
  [[nodiscard]]
  constexpr
  typename Map::iterator       find(entity_type const e)
  noexcept
  {
    static_assert(
        map_sequence::template contains_v<Map>,
        "heim::group::find(entity_type const): Map does not appear in "
        "map_sequence. ");

    return get<Map>().find(e);
  }

  template<typename Map>
  [[nodiscard]]
  constexpr
  typename Map::const_iterator find(entity_type const e) const
  noexcept
  {
    static_assert(
        map_sequence::template contains_v<Map>,
        "heim::group::find(entity_type const) const: Map does not appear in "
        "map_sequence. ");

    return get<Map>().find(e);
  }


  template<typename Map>
  [[nodiscard]]
  constexpr
  typename Map::component_type       &operator[](entity_type const e)
  noexcept
  {
    static_assert(
        map_sequence::template contains_v<Map>,
        "heim::group::operator[](entity_type const): Map does not appear in "
        "map_sequence. ");

    return get<Map>()[e];
  }

  template<typename Map>
  [[nodiscard]]
  constexpr
  typename Map::component_type const &operator[](entity_type const e) const
  noexcept
  {
    static_assert(
        map_sequence::template contains_v<Map>,
        "heim::group::operator[](entity_type const) const: Map does not appear "
        "in map_sequence. ");

    return get<Map>()[e];
  }


  template<typename Map>
  [[nodiscard]]
  constexpr
  typename Map::component_type       &at(entity_type const e)
  {
    static_assert(
        map_sequence::template contains_v<Map>,
        "heim::group::at(entity_type const): Map does not appear in "
        "map_sequence. ");

    return get<Map>().at(e);
  }

  template<typename Map>
  [[nodiscard]]
  constexpr
  typename Map::component_type const &at(entity_type const e) const
  {
    static_assert(
        map_sequence::template contains_v<Map>,
        "heim::group::at(entity_type const) const: Map does not appear in "
        "map_sequence. ");

    return get<Map>().at(e);
  }



  template<typename Map>
  constexpr
  void clear()
  noexcept
  {
    static_assert(
        map_sequence::template contains_v<Map>,
        "heim::group::clear(): Map does not appear in map_sequence. ");

    get<Map>().clear();
  }

  template<
      typename    Map,
      typename ...Args>
  constexpr
  bool emplace_back(entity_type const e, Args &&...args)
  {
    static_assert(
        map_sequence::template contains_v<Map>,
        "heim::group::emplace_back(entity_type const, Args &&...): Map does "
        "not appear in map_sequence. ");

    bool const ret = get<Map>().emplace_back(e, std::forward<Args>(args)...);

    if (ret && (get<Maps>().contains(e) && ...))
      include(e);

    return ret;
  }

  template<typename Map>
  constexpr
  bool pop_swap(entity_type const e)
  noexcept
  {
    static_assert(
        map_sequence::template contains_v<Map>,
        "heim::group::pop_swap(entity_type const): Map does not appear in "
        "map_sequence. ");

    bool const ret = get<Map>().pop_swap(e);

    if (ret)
      exclude(e);

    return ret;
  }

};

}

#endif // HEIM_GROUP_HPP
