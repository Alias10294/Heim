#pragma once

#include <vector>
#include <limits>
#include <cstddef>

#include "common.hpp"

namespace heim
{
    /**
     * @brief An optimised associative container for the storing and access of components. 
     * 
     * Bases its implementation off of sparse sets, providing great cache-friendliness and 
     * promising O(1) insertion, deletion and access to components. 
     * This comes at the cost of memory overhead, having to contain a large vector of 
     * indexes, though will some day be optimised maybe with pagination. 
     * 
     * @tparam component The component type to hold. 
     */
    template<typename component>
    class composition
    {
    public:
        /// @brief A placeholder for null ids in the sparse vector.
        static constexpr std::size_t null_idx = std::numeric_limits<std::size_t>::max();



        /**
         * @brief Emplaces the component in the composition. 
         * 
         * @tparam ...args The type of arguments needed for the creation of the component.
         * @param e The entity to emplace a component for. 
         * @param a The arguments needed for the creation of the component.
         * 
         * @note Uses emplace_back, but components may be rearranged when deletions occur. 
         */
        template<typename... args>
        constexpr void emplace(const entity e, args&&... a)
        {
            if (contains(e))
                return;
            if (e >= sparse_.size())
                reserve(e + 1);
            
            sparse_.at(e) = entities_.size();
            entities_.emplace_back(e);
            components_.emplace_back(std::forward<args>(a)...);
        }

        /**
         * @brief Erases the component of the given entity. 
         * 
         * Uses the swap-and-pop method, thus rearranging components in the composition. 
         * 
         * @param e The entity to erase a component to. 
         */
        constexpr void erase(const entity e)
        {
            if (!contains(e))
                return;
            
            entities_[sparse_[e]] = entities_.back();
            components_[sparse_[e]] = components_.back();
            sparse_[entities_.back()] = sparse_[e];

            sparse_[e] = null_idx;
            entities_.pop_back();
            components_.pop_back();
        }



        /**
         * @brief Retrieves the index in the dense vector of the given entity. 
         * 
         * @param e The entity to retrieve the index of. 
         * @return The index of the given entity. 
         */
        constexpr std::size_t index(const entity e) const noexcept
        {
            return sparse_[e];
        }

        /**
         * @brief Retrieves the entity associated to the given index. 
         * 
         * @param idx The index to retrieve the corresponding entity of. 
         * @return The entity corresponding to the given index. 
         */
        constexpr entity composed(std::size_t idx) const noexcept
        {
            return entities_[idx];
        }

        /**
         * @brief Retrieves the component associated to the given entity. 
         * 
         * @param e The entity to retrieve the component of. 
         * @return The componenet of the given entity. 
         */
        [[nodiscard]] constexpr component& get(const entity e) noexcept
        {
            return components_[sparse_[e]];
        }
        /**
         * @brief Retrieves the component associated to the given entity. 
         * 
         * @param e The entity to retrieve the component of. 
         * @return The componenet of the given entity. 
         */
        [[nodiscard]] constexpr const component& get(const entity e) const noexcept
        {
            return components_[sparse_[e]];
        }

        /**
         * @brief Returns the number of components in the composition.
         */
        constexpr std::size_t size() const noexcept
        {
            return components_.size();
        }

        /**
         * @brief Increases the capacity of the composition, allocating memory
         * for a given amount of components. 
         * 
         * @param n The new capacity of the composition. 
         */
        constexpr void reserve(std::size_t n)
        {
            sparse_.resize(n, null_idx);
            entities_.reserve(n);
            components_.reserve(n);
        }



        /**
         * @brief Checks if the entity has a component in the composition. 
         * 
         * @param e The entity to search for. 
         * @return @c true if the entity has a component, @c false otherwise. 
         */
        constexpr bool contains(const entity e) const noexcept
        {
            return e < sparse_.size() && 
                   sparse_[e] < entities_.size() && 
                   entities_[sparse_[e]] == e;
        }


    
    private:
        std::vector<std::size_t>    sparse_;
        std::vector<entity>         entities_;
        std::vector<component>      components_;
    
    };



    /**
     * @brief The type-erased version of the compisition. 
     * 
     * Allows for generalized and dynamic containment of all the compositions
     * in the composer. 
     */
    struct composition_erased
    {
        composition_erased(const composition_erased&) = delete;
        composition_erased& operator=(const composition_erased&) = delete;

        composition_erased(
            void*       self_,
            void        (*destroy_)  (void*),
            void        (*erase_)    (void*, const entity),
            std::size_t (*index_)    (void*, const entity),
            entity      (*composed_) (void*, const std::size_t),
            void*       (*get_)      (void*, const entity),
            std::size_t (*size_)     (void*),
            void        (*reserve_)  (void*, const std::size_t),
            bool        (*contains_) (void*, const entity)
        ) noexcept : 
            self(self_),
            destroy(destroy_),
            erase(erase_),
            index(index_),
            composed(composed_),
            get(get_),
            size(size_),
            reserve(reserve_),
            contains(contains_)
        { }

        composition_erased(composition_erased&& o) noexcept : 
            self(o.self), 
            destroy(o.destroy), 
            erase(o.erase), 
            index(o.index), 
            composed(o.composed), 
            get(o.get), 
            size(o.size), 
            reserve(o.reserve), 
            contains(o.contains)
        { 
            o.destroy = [](void*){ };
        }

        ~composition_erased()
        {
            destroy(self);
        }



        /// @brief The composition to hold.
        void*       self;
        void        (*destroy)  (void*);

        void        (*erase)    (void*, const entity);

        std::size_t (*index)    (void*, const entity);
        entity      (*composed) (void*, const std::size_t);
        void*       (*get)      (void*, const entity);
        std::size_t (*size)     (void*);
        void        (*reserve)  (void*, const std::size_t);

        bool        (*contains) (void*, const entity);

    };

    /**
     * @brief Returns a new type-erased composition. 
     * 
     * @tparam component The composition to make a composition for. 
     * @return The new type-erased composition for the given component. 
     */
    template<typename component>
    composition_erased make_erased()
    {
        using comp = composition<component>;
        comp* c = new comp();

        return composition_erased
        {
            c, 
            +[](void* s)
            { 
                delete static_cast<comp*>(s); 
            }, 

            +[](void* s, const entity e)
            {
                static_cast<comp*>(s)->erase(e);
            }, 

            +[](void* s, const entity e)
            {
                return static_cast<comp*>(s)->index(e);
            }, 
            +[](void* s, const std::size_t idx)
            {
                return static_cast<comp*>(s)->composed(idx);
            }, 
            +[](void* s, const entity e)
            {
                return static_cast<void*>(&static_cast<comp*>(s)->get(e));
            }, 
            +[](void* s)
            {
                return static_cast<comp*>(s)->size();
            }, 
            +[](void* s, const std::size_t n)
            {
                static_cast<comp*>(s)->reserve(n);
            }, 
            
            +[](void* s, const entity e)
            {
                return static_cast<comp*>(s)->contains(e);
            }
        };
    }
}