#pragma once

#include <tuple>

#include "common.hpp"
#include "composition.hpp"

namespace heim
{
    /**
     * @brief A glimpse of the current state of the components of given type of the world. 
     * @tparam ...components The types of the components of the glimpse. 
     */
    template<typename... comps>
    class basic_glimpse
    {
    public:
        using compositions_tuple = std::tuple<composition<std::remove_cvref_t<comps>>&...>;

        class iterator
        {
        public:
            constexpr iterator(compositions_tuple& compositions, const std::size_t ref_index = 0ULL) : 
                compositions_{compositions}, 
                ref_index_{ref_index}, 
                ref_size_{std::get<0>(compositions_).size()}
            { 
                advance_to_valid();
            }



            constexpr entity operator*() const noexcept
            { 
                return std::get<0>(compositions_).composed(ref_index_);
            }

            constexpr iterator& operator++()
            { 
                ++ref_index_;
                advance_to_valid();
                return *this;
            }

            constexpr bool operator==(const iterator& other) const noexcept
            { 
                return ref_index_ == other.ref_index_;
            }

            constexpr bool operator!=(const iterator& other) const noexcept
            { 
                return !(*this == other);
            }

        private:
            compositions_tuple& compositions_;

            std::size_t ref_index_;
            std::size_t ref_size_;



            /**
             * @brief Advances to the next entity that has all of the components of the glimpse. 
             */
            constexpr void advance_to_valid()
            { 
                auto valid = [&]()
                {
                    return std::apply([&](auto&... compositions)
                           { 
                               return (compositions.contains(std::get<0>(compositions_).composed(ref_index_)) && ...);
                           }, 
                           compositions_);
                };

                while (ref_index_ < ref_size_ && !valid())
                    ++ref_index_;
            }
        
        };



        constexpr basic_glimpse(composition<std::remove_cvref_t<comps>>&... compositions) noexcept : 
            compositions_{compositions...}
        { }



        constexpr iterator begin()
        { 
            return iterator(compositions_, 0);
        }
        constexpr iterator end()
        { 
            return iterator(compositions_, std::get<0>(compositions_).size());
        }


        
        /*
        constexpr std::vector<std::tuple<components&...>> each()
        {
            
        }
        */

    private:
        compositions_tuple compositions_;
    
    };
}