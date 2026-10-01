/**
 * gfx::uniformset - Shader uniform collection for material uniform definitions & overrides
 * ZIK@MMXXVI
 */

#ifndef __CPEX_GFX_UNIFORMSET_GUARD
#define __CPEX_GFX_UNIFORMSET_GUARD

#include <type_traits>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <zcl/zcl.hpp>

#include <gfx/uniform.hpp>

// EXTERNAL LIBRARIES //
// ----------------------------
// OpenGL: GLAD
#include <glad/gl.h>
// ----------------------------
// EXTERNAL LIBRARIES //

namespace gfx {
    /** Collection of uniforms. */
    class UniformSet {
        std::vector<std::shared_ptr<Uniform>> uniforms;
        std::unordered_map<std::string, std::pair<size_t, std::shared_ptr<Uniform>>> uniformsByName;
        
    public:
        /** Adds an uniform. */
        template <typename T>
        void add_uniforms(T &&uniform);
        /** Adds an uniform. */
        template <typename T>
        void add_uniforms_ptr(std::shared_ptr<T> &uniformPtr);
        /** Adds uniforms. */
        template <typename U, typename...T>
        void add_uniforms(U &&uniform, T&&... uniformArgs);
        /** Adds uniforms. */
        template <typename U, typename...T>
        void add_uniforms_ptr(std::shared_ptr<U> &uniformPtr, std::shared_ptr<T>&... uniformPtrArgs);
        /** Adds uniforms from other uniformset. */
        void add_uniforms_from(const UniformSet &other);
        /** Clears all uniforms. */
        void clear();
        /** Returns an uniform with given name and type. `nullptr` if not found or wrong type. */
        template <typename T>
        std::shared_ptr<T> get_uniform(const std::string name) const;
        
        /** Returns uniform at certain position. */
        const std::vector<std::shared_ptr<Uniform>>& get_uniforms() const;
        /** Returns an iterator to the beginning. */
        std::vector<std::shared_ptr<Uniform>>::const_iterator begin();
        /** Returns an iterator to the end. */
        std::vector<std::shared_ptr<Uniform>>::const_iterator end();

        /** Number of uniforms. */
        size_t size() const;
    };

    // DEFINITIONS (INCLUSION MODEL FOR TEMPLATES!) //

    template <typename T>
    void UniformSet::add_uniforms(T &&uniform) {
        static_assert(std::is_base_of<Uniform, T>::value, "T must be type of Uniform!");
        
        std::shared_ptr<T> ptr = std::make_shared<T>(std::forward<T>(uniform));
        auto name = uniform.get_name();

        // Override if needed
        if (uniformsByName.contains(name)) {
            auto entry = uniformsByName[name];
            auto idx = entry.first;
            uniforms[idx] = ptr;
            entry.second = ptr;
        }
        else {
            size_t idx = uniforms.size();
            uniforms.push_back(ptr);
            uniformsByName[name] = std::pair(idx, ptr);
        }
    }

    template <typename T>
    void UniformSet::add_uniforms_ptr(std::shared_ptr<T> &uniformPtr) {
        static_assert(std::is_base_of<Uniform, T>::value, "T must be type of Uniform!");
        
        auto name = uniformPtr->get_name();

        // Override if needed
        if (uniformsByName.contains(name)) {
            auto entry = uniformsByName[name];
            auto idx = entry.first;
            uniforms[idx] = uniformPtr;
            entry.second = uniformPtr;
        }
        else {
            size_t idx = uniforms.size();
            uniforms.push_back(uniformPtr);
            uniformsByName[name] = std::pair(idx, uniformPtr);
        }
    }

    template <typename U, typename...T>
    void UniformSet::add_uniforms(U &&uniform, T&&... uniformArgs) {
        static_assert(std::is_base_of<Uniform, U>::value, "U must be type of Uniform!");
        
        std::shared_ptr<U> ptr = std::make_shared<U>(std::forward<U>(uniform));
        auto name = uniform.get_name();

        // Override if needed
        if (uniformsByName.contains(name)) {
            auto entry = uniformsByName[name];
            auto idx = entry.first;
            uniforms[idx] = ptr;
            entry.second = ptr;
        }
        else {
            size_t idx = uniforms.size();
            uniforms.push_back(ptr);
            uniformsByName[name] = std::pair(idx, ptr);
        }

        add_uniforms(uniform);
    }

    template <typename U, typename...T>
    void UniformSet::add_uniforms_ptr(std::shared_ptr<U> &uniformPtr, std::shared_ptr<T>&... uniformPtrArgs) {
        static_assert(std::is_base_of<Uniform, U>::value, "U must be type of Uniform!");
        auto name = uniformPtr->get_name();

        // Override if needed
        if (uniformsByName.contains(name)) {
            auto entry = uniformsByName[name];
            auto idx = entry.first;
            uniforms[idx] = uniformPtr;
            entry.second = uniformPtr;
        }
        else {
            size_t idx = uniforms.size();
            uniforms.push_back(uniformPtr);
            uniformsByName[name] = std::pair(idx, uniformPtr);
        }

        add_uniforms_ptr(uniformPtrArgs);
    }

    template <typename T>
    std::shared_ptr<T> UniformSet::get_uniform(const std::string name) const {
        static_assert(std::is_base_of<Uniform, T>::value, "T must be type of Uniform!");

        auto res = uniformsByName.find(name);
        auto resUni = (res != uniformsByName.end()) ? (res->second.second) : std::shared_ptr<Uniform>(nullptr);

        // use `dynamic_pointer_cast` instead of normal `dynamic_cast` for `shared_ptr`!
        return std::dynamic_pointer_cast<T>(resUni);
    }

    // DEFINITIONS (INCLUSION MODEL FOR TEMPLATES!) //
}

#endif