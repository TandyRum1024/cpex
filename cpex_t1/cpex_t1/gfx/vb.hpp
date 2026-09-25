/**
 * gfx::vb - OpenGL Vertex buffer abstraction
 * ZIK@MMXXVI
 */

#ifndef __CPEX_GFX_MESH_GUARD
#define __CPEX_GFX_MESH_GUARD

#include <stdint.h>
#include <array>
#include <vector>
#include <stdexcept>
#include <span>

// LIBRARIES //
#include <zcl/zcl.hpp>

// EXTERNAL LIBRARIES //
// ----------------------------
// OpenGL: GLAD
#include <glad/gl.h>
// OpenGL: GLFW
#include <GLFW/glfw3.h>
// ----------------------------
// EXTERNAL LIBRARIES //

namespace gfx {
    /** Contains all the neccessary information to manage a vertex attribute in VBO. */
    class VertAttribute {
        GLuint location;

        // Data layout interpretation
        GLsizei layoutStride;
        int layoutOff;
        
        // Vertex attribute interpretation
        int dim;
        GLenum type;
        size_t typeSize;
        GLboolean isNormalized;
    public:
        VertAttribute(GLuint location, int dim, GLenum type, size_t typeSize, GLsizei layoutStride, int layoutOff, GLboolean isNormalized = GL_FALSE);
        
        size_t get_type_size() const;
        void set_attribute_pointer();
    };

    /** List of VertAttribute, stored in order of locations in vertex shader. */
    class VertFormat {
        std::vector<VertAttribute> attribs;
        size_t size;

    public:
        VertFormat() = default;
        VertFormat(std::initializer_list<VertAttribute> attributes);

        size_t get_size() const;

        void add_attribute(VertAttribute attrib);
        void set_attribute_pointers();
    };

    /** Contains all the neccessary informations to render a mesh. */
    class Vb {
    public:
        enum VB_BUFFER {
            VB_BUFFER_VBO,
            VB_BUFFER_EBO,
            VB_BUFFER_SZ
        };

    private:
        GLuint vao;
        std::shared_ptr<VertFormat> format;
        /** Contains VBO, EBO. Indexed by enum `VB_BUFFER*` */
        std::array<GLuint, VB_BUFFER_SZ> bufferHandles;
        std::array<std::vector<uint8_t>, VB_BUFFER_SZ> buffers;
        std::array<unsigned int, VB_BUFFER_SZ> bufferElementSizes;
        // std::vector<unsigned int> indices;
        
        /** Releases OpenGL resources. */
        void release_resources();
        
    public:
        // `Vb() = default;` does not work since its template / generic class, it will give deleted constructor as a default one
        Vb();
        ~Vb();

        // Disable default copy ops, since `Vb` is move only (tied to OpenGL objects that are hard to copy)!

        Vb(const Vb &other) = delete;
        Vb& operator=(const Vb &other) = delete;

        // Only implement move ops for now

        Vb(Vb &&other);
        Vb& operator=(Vb &&other);

        /** Binds the VAO. */
        void bind();
        /** Sets vertex format. */
        void set_format(std::shared_ptr<VertFormat> format);
        /** Clears buffer data. */
        void clear_buffer(VB_BUFFER target);
        /** Clears all buffer data. */
        void clear_buffer_all();
        /** Sets buffer data. */
        template <typename V>
        void set_buffer(VB_BUFFER target, const std::span<V> &data);
        /** Sets indices data. Specialized case of `set_buffer()` */
        void set_buffer_indices(const std::span<unsigned int> &indices);
        /** Appends buffer data. */
        template <typename V>
        void append_buffer(VB_BUFFER target, const std::span<V> &data);
        /** Appends indices data. Specialized case of `append_buffer()` */
        void append_buffer_indices(const std::span<unsigned int> &indices, unsigned int appendBase = 0);
        /** Returns number of vertices. */
        unsigned int get_buffer_size(VB_BUFFER target = VB_BUFFER_VBO) const;
        /** Returns number of indices. (data stored in EBO) */
        unsigned int get_indices_num() const;
        /** Builds this vertex buffer. */
        void build();
        /** Renders this vertex buffer. */
        void submit(GLenum mode, int indicesStartOff, GLsizei indicesCount = -1);
        /** Renders this vertex buffer. */
        void submit();
    };

    // DEFINITIONS (INCLUSION MODEL FOR TEMPLATES!) //

    template <typename V>
    void Vb::set_buffer(VB_BUFFER target, const std::span<V> &data) {
        clear_buffer(target);
        append_buffer(target, data);
    }

    template <typename V>
    void Vb::append_buffer(VB_BUFFER target, const std::span<V> &data) {
        auto& buff = buffers[target];
        bufferElementSizes[target] += data.size();

        // Copy to internal buffer
        // `std::memcpy()` could be used, but apparently we can just use `reinterpret_cast`
        auto dataBytes = reinterpret_cast<uint8_t*>(data.data());
        auto dataBytesSz = data.size() * sizeof(V);
        auto prevSz = buff.size();
        auto newSz = prevSz + dataBytesSz;

        buff.resize(newSz);
        std::copy(dataBytes, dataBytes + dataBytesSz, buff.begin() + prevSz);
    }

    // DEFINITIONS (INCLUSION MODEL FOR TEMPLATES!) //
}
#endif