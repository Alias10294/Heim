#pragma once

#include "common.hpp"
#include "composer.hpp"
#include "glimpse.hpp"
#include "summoner.hpp"

namespace heim
{
    /**
     * @brief The world.
     */
    class world
    {
    public:
        /**
         * @brief Summons a new entity in the game. 
         * 
         * @return The new entity to be used. 
         */
        entity summon()
        {
            return summoner_.summon();
        }

        /**
         * @brief Destroy the entity and all of its components. 
         */
        void destroy(const entity e)
        {
            summoner_.banish(e);
            composer_.clear(e);
        }



        template<typename component>
        using predicate = composer::predicate<component>;
        /**
         * @brief Compose an entity of a new component
         * 
         * @tparam component The type of the component to compose with.
         * @param predicate The predicate used to establish order in the composition. 
         */
        template<typename component>
        constexpr void compose(predicate<component> cmp = nullptr) noexcept
        {
            composer_.compose<component>(cmp);
        }



        /**
         * @brief Compose an entity of a new component
         * 
         * @tparam component The type of the new component. 
         * @tparam ...args The type of the arguments for the new component. 
         * @param e The entity to compose. 
         * @param a ...a The arguments for the new component.
         */
        template<typename component, typename... args>
        constexpr void compose(const entity e, args&&... a)
        {
            composer_.compose<component>(e, std::forward<args>(a)...);
        }

        /**
         * @brief Erase the component(s) of given type(s) of the given entity.
         * 
         * @tparam ...components The types of the components to erase. 
         * @param e The entity to erase components of. 
         */
        template<typename... components>
        constexpr void erase(const entity e)
        {
            composer_.erase<components...>(e);
        }



        /**
         * @brief Returns a glimpse of the world concerning the given component types. 
         * 
         * @tparam ...components The components to catch a glimpse at. 
         * @return The glimpse of the components. 
         */
        template<typename... components>
        basic_glimpse<components...> glimpse() noexcept
        {
            return composer_.glimpse<components...>();
        }



        /**
         * @brief Retrieves the component of given type of the given entity. 
         * 
         * @tparam component The type of the component to get. 
         * @param e The entity to retrieve the component of.
         * @return The component of the entity. 
         */
        template<typename component>
        component& get(const entity e) noexcept
        { 
            return composer_.get<component>(e);
        }

        /**
         * @brief Checks if the given entity has component(s) of given type(s). 
         * 
         * @tparam ...components The type(s) of the component(s) to check. 
         * @param e The entity to check. 
         * @return true if the entity has all of the components, false otherwise. 
         */
        template<typename... components>
        constexpr bool has(const entity e)
        { 
            return composer_.has<components...>(e);
        }

    private:
        summoner summoner_{};
        composer composer_{};

    };
}