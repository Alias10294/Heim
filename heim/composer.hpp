#pragma once

#include <vector>
#include <unordered_map>
#include <typeindex>

#include "common.hpp"
#include "composition.hpp"
#include "glimpse.hpp"

namespace heim
{
    /**
     * @brief Manages the components of the world. 
     */
    class composer
    {
    public:
        /**
         * @brief Compose an entity of a new component
         * 
         * @tparam component The type of the new component. 
         * @tparam ...args The type of the arguments for the new component. 
         * @param e The entity to compose. 
         * @param a ...a The arguments for the new component.
         */
        template<typename comp, typename... args>
        constexpr void compose(const entity e, args&&... a)
        { 
            using component = std::remove_cvref_t<comp>;

            if (!composes<component>())
            {
                indexes_[type_index<component>()] = compositions_.size();
                compositions_.push_back(make_erased<component>());
            }
            component c(std::forward<args>(a)...);
            composition_erased& ce = compositions_[index<component>()];
            static_cast<composition<component>*>(ce.self)->emplace(e, c);
        }

        /**
         * @brief Erase the component(s) of given type(s) of the given entity.
         * 
         * @tparam ...components The types of the components to erase. 
         * @param e The entity to erase components of. 
         */
        template<typename... comps>
        constexpr void erase(const entity e)
        { 
            (erase_one<std::remove_cvref_t<comps>>(e), ...);
        }

        /**
         * @brief Clears the given entity of any component. 
         * 
         * @param e The entity to clear.
         */
        constexpr void clear(const entity e)
        { 
            for (composition_erased& ce : compositions_)
                ce.erase(ce.self, e);
        }



        /**
         * @brief Returns a glimpse of the world concerning the given component types. 
         * 
         * @tparam ...components The components to catch a glimpse at. 
         * @return The glimpse of the components. 
         */
        template<typename... comps>
        [[nodiscard]] basic_glimpse<comps...> glimpse() noexcept
        { 
            return basic_glimpse<comps...>(get_composition<std::remove_cvref_t<comps>>()...);
        } 

        /**
         * @brief Retrieves the component of given type of the given entity. 
         * 
         * @tparam component The type of the component to get. 
         * @param e The entity to retrieve the component of.
         * @return The component of the entity. 
         */
        template<typename comp>
        [[nodiscard]] comp& get(const entity e) noexcept
        { 
            using component = std::remove_cvref_t<comp>;

            composition_erased& ce = compositions_[index<component>()];
            return *static_cast<component*>(ce.get(ce.self, e));
        }
        /**
         * @brief Retrieves the component of given type of the given entity. 
         * 
         * @tparam component The type of the component to get. 
         * @param e The entity to retrieve the component of.
         * @return The component of the entity. 
         */
        template<typename comp>
        [[nodiscard]] const comp& get(const entity e) const noexcept
        { 
            using component = std::remove_cvref_t<comp>;

            composition_erased& ce = compositions_[index<component>()];
            return *static_cast<component*>(ce.get(ce.self, e));
        }



        /**
         * @brief Checks if the given entity has component(s) of given type(s). 
         * 
         * @tparam ...components The type(s) of the component(s) to check. 
         * @param e The entity to check. 
         * @return true if the entity has all of the components, false otherwise. 
         */
        template<typename... comps>
        constexpr bool has(const entity e) const noexcept
        { 
            return (has_one<std::remove_cvref_t<comps>>(e) && ...);
        }

    private:
        std::vector<composition_erased> compositions_;
        std::unordered_map<std::type_index, std::size_t> indexes_;



        /**
         * @brief Returns the type index of the given component type. 
         * 
         * @tparam component The component to get the type index of. 
         * @return The type index of the component. 
         */
        template<typename component>
        constexpr std::type_index type_index() const noexcept
        {
            return std::type_index(typeid(component));
        }

        /**
         * @brief Returns the index of the composition of given component type. 
         * 
         * @tparam component The component type of the composition to get the index of. 
         * @return The index of the concerned composition. 
         */
        template<typename component>
        constexpr std::size_t index() const noexcept
        {
            return indexes_.at(type_index<component>());
        }



        /**
         * @brief Checks if the component of given type already has a composition. 
         * 
         * @tparam component The component type to check for. 
         * @returns true if the component already has a composition, false otherwise. 
         */
        template<typename component>
        constexpr bool composes() const
        {
            return indexes_.contains(type_index<component>());
        }



        /**
         * @brief Erase the component of given type of the given entity.
         * 
         * @tparam component The type of the component to erase. 
         * @param e The entity to erase the component of. 
         */
        template<typename component>
        constexpr void erase_one(const entity e)
        {
            if (!composes<component>())
                return;
            
            composition_erased& ce = compositions_[index<component>()];
            ce.erase(ce.self, e);
        }

        /**
         * @brief Retrieves the composition of the given component type. 
         * 
         * @tparam component The component type of the composition to retrieve. 
         * @return The corresponding composition. 
         */
        template<typename component>
        constexpr composition<component>& get_composition() noexcept
        {
            composition_erased& ce = compositions_[index<component>()];
            return *static_cast<composition<component>*>(ce.self);
        }

        /**
         * @brief Checks if the given entity has a component of given type. 
         * 
         * @tparam component The type of the component to check. 
         * @param e The entity to check. 
         * @return true if the entity has the component, false otherwise. 
         */
        template<typename component>
        constexpr bool has_one(const entity e) const noexcept
        {
            composition_erased& ce = compositions_[index<component>()];
            return ce.contains(ce.self, e);
        }

    };
}