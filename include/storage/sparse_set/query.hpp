#ifndef HEIM_SPARSE_SET_BASED_QUERY_HPP
#define HEIM_SPARSE_SET_BASED_QUERY_HPP

#include <cstddef>
#include "query_expression.hpp"

namespace heim::sparse_set_based
{
/*!
 * @brief A type used to iterate over a given storage's elements following a certain query
 *   expression.
 *
 * @details Implements
 *
 * @tparam Storage    The storage type.
 * @tparam Expression The query expression type.
 */
template<
    typename Storage,
    typename Expression = heim::query_expression<>>
class query;

template<
    typename Storage,
    typename Expression>
class query
{
public:
  using storage_type    = Storage;
  using expression_type = Expression;

  static_assert(
      specializes_query_expression_v<expression_type>,
      "expression_type must be a specialization of query_expression.");


  using entity_type = typename storage_type::entity_type;

private:
  struct query_traits
  {
    template<typename Component>
    using tag_value_of
    = storage_type::component_info_sequence_traits::template tag_value_of<Component>;


    using include_sequence = typename expression_type::include_sequence;
    using exclude_sequence = typename expression_type::exclude_sequence;

    using value_include_sequence = typename include_sequence::template filter<tag_value_of>;
    using value_exclude_sequence = typename exclude_sequence::template filter<tag_value_of>;
  };

public:
  using value_type
  = typename type_sequence<entity_type>
      ::template concatenate<
          typename query_traits::value_include_sequence
              ::template map<std::remove_cvref>>
      ::tuple;

  using reference
  = typename type_sequence<entity_type const &>
      ::template concatenate<
          typename query_traits::value_include_sequence
              ::template map<std::add_lvalue_reference>>
      ::tuple;

  using const_reference
  = typename type_sequence<entity_type const &>
      ::template concatenate<
          typename query_traits::value_include_sequence
              ::template map<std::add_lvalue_reference>
              ::template map<std::add_const>>
      ::tuple;

private:
  template<bool IsConst>
  class generic_iterator
  {
  public:
    using difference_type = std::ptrdiff_t;


    static constexpr bool is_const = IsConst;

    using iterator_category = std::input_iterator_tag;
    using iterator_concept  = std::forward_iterator_tag;

    using value_type
    = typename query::value_type;

    using reference
    = std::conditional_t<
        is_const,
        typename query::const_reference,
        typename query::reference>;

    struct pointer
    {
    private:
      reference m_ref;

    public:
      explicit constexpr
      pointer(reference &&)
      noexcept;


      constexpr
      reference *
      operator->() const
      noexcept;
    };


    friend query;
    friend generic_iterator<!is_const>;

  private:
    storage_type   *m_storage;
    difference_type m_index;
  };

private:
  storage_type *m_storage;

public:
  explicit constexpr
  query(storage_type *storage)
  noexcept;

  explicit constexpr
  query(storage_type &storage)
  noexcept;

  constexpr
  query()
  noexcept;

  constexpr
  query(query const &)
  = default;

  constexpr
  query(query &&)
  = default;

  constexpr
  ~query()
  = default;

  constexpr
  query &
  operator=(query const &)
  = default;

  constexpr
  query &
  operator=(query &&)
  = default;
};



template<
    typename Storage,
    typename Expression>
template<bool IsConst>
constexpr
query<Storage, Expression>
    ::generic_iterator<IsConst>
    ::pointer
    ::pointer(reference &&ref)
noexcept
  : m_ref(ref)
{ }



template<
    typename Storage,
    typename Expression>
template<bool IsConst>
constexpr
typename query<Storage, Expression>
    ::template generic_iterator<IsConst>
    ::reference *
query<Storage, Expression>
    ::generic_iterator<IsConst>
    ::pointer
    ::operator->() const
noexcept
{
  return std::addressof(m_ref);
}



template<
    typename Storage,
    typename Expression>
constexpr
query<Storage, Expression>
    ::query(Storage* storage)
noexcept
  : m_storage(storage)
{ }

template<
    typename Storage,
    typename Expression>
constexpr
query<Storage, Expression>
    ::query(Storage &storage)
noexcept
  : query(&storage)
{ }

template<
    typename Storage,
    typename Expression>
constexpr
query<Storage, Expression>
    ::query()
noexcept
  : query(nullptr)
{ }


}

#endif // HEIM_SPARSE_SET_BASED_QUERY_HPP