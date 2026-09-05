/**
 * gfx::vb - OpenGL Vertex buffer abstraction
 * ZIK@MMXXVI
 */

#ifndef __CPEX_GFX_MESH_GUARD
#define __CPEX_GFX_MESH_GUARD

#include <vector>
#include <stdexcept>

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

        void set_attribute_pointer();
    };

    /** List of VertAttribute, stored in order of locations in vertex shader. */
    class VertFormat {
        std::vector<VertAttribute> attribs;

    public:
        VertFormat() = default;
        VertFormat(std::initializer_list<VertAttribute> attributes);

        void add_attribute(VertAttribute attrib);
        void set_attribute_pointers();
    };

    /** Contains all the neccessary informations to render a mesh. */
    template <typename V>
    class Vb {
        enum VB_BUFF_OBJ {
            VB_BUFF_VBO,
            VB_BUFF_EBO,
            _VB_BUFF_OBJ_SZ
        };

        std::shared_ptr<spdlog::logger> _logger;
        
        GLuint vao;
        std::shared_ptr<VertFormat> format;
        bool isFormatSet;
        /** Contains VBO, EBO. Indexed by enum `VB_BUFF_*` */
        std::array<GLuint, _VB_BUFF_OBJ_SZ> objs;
        std::vector<V> verts;
        std::vector<unsigned int> indices;
        
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

        /** Sets vertex format. */
        void set_format(std::shared_ptr<VertFormat> format);
        /** Appends vertex. */
        void push_back_verts(V vert);
        /** Appends vertices. */
        void push_back_verts(std::vector<V> appendVerts);
        /** Appends index. */
        void push_back_indices(unsigned int idx);
        /** Appends indices. */
        void push_back_indices(std::vector<unsigned int> appendIndices);
        /** Returns number of vertices. */
        unsigned int get_vertices_num();
        /** Returns number of indices. */
        unsigned int get_indices_num();
        /** Builds the VBO/VAO/EBO. */
        void build();
        /** Renders this vertex buffer. */
        void submit(GLenum mode, int indicesStartOff, GLsizei indicesCount = -1);
        /** Renders this vertex buffer. */
        void submit();
    };

    // DEFINITIONS (INCLUSION MODEL FOR TEMPLATE CLASSES!) //

    template <typename V>
    Vb<V>::Vb():
        vao(0),
        format(nullptr),
        isFormatSet(false),
        objs({0}),
        verts(),
        indices()
        {
        // std::cout << "[GFX] Vb@" << this << " created!" << std::endl;
        // zcl::logger("VB")->info("Create VB @{}", (void*)this);
    }
        
    template <typename V>
    Vb<V>::~Vb() {
        // zcl::logger("VB")->info("Destroy VB @{}", (void*)this);
        release_resources();
    }

    template <typename V>
    Vb<V>::Vb(Vb &&other):
        verts(std::move(other.verts)),
        indices(std::move(other.indices)),
        format(std::move(other.format)),
        isFormatSet(std::exchange(other.isFormatSet, false)),
        // (replace GL resources with dummy)
        objs(std::exchange(other.objs, { 0 })),
        vao(std::exchange(other.vao, 0))
        {
        // zcl::logger("VB")->info("Move VB @{} <- @{} (objs: [{}] VS [{}])", (void*)this, (void*)&other, zcl::str::to_str(objs), zcl::str::to_str(other.objs));
        // for (size_t i = 0; i < _VB_BUFF_OBJ_SZ; i++) {
        //     objs[i] = other.objs[i];
        //     other.objs[i] = 0;
        // }
        // std::cout << "[GFX] Vb@" << this << " <- Vb@" << &other << " moved!" << std::endl;
    }

    template <typename V>
    Vb<V>& Vb<V>::operator=(Vb &&other) {
        // zcl::logger("VB")->info("Move VB @{} <- @{} (objs: [{}] VS [{}])", (void*)this, (void*)&other, zcl::str::to_str(objs), zcl::str::to_str(other.objs));

        if (this == &other) {
            // Self assignment, no need to move
            return *this;
        }
        
        // std::cout << "[GFX] Vb@" << this << " <- Vb@" << &other << " moved!" << std::endl;

        std::swap(verts, other.verts);
        std::swap(indices, other.indices);
        std::swap(format, other.format);
        std::swap(isFormatSet, other.isFormatSet);
        std::swap(objs, other.objs);
        std::swap(vao, other.vao);

        // zcl::logger("VB")->info("\tAfter move VB @{} <- @{} (objs: [{}] VS [{}])", (void*)this, (void*)&other, zcl::str::to_str(objs), zcl::str::to_str(other.objs));

        // other.release_resources(); // automatically called on destruction
        return *this;
    }

    template <typename V>
    void Vb<V>::release_resources() {
        for (auto &&obj: objs) {
            if (obj) {
                glDeleteBuffers(1, &obj);
                obj = 0;
            }
        }
        
        if (vao) {
            glDeleteVertexArrays(1, &vao);
            vao = 0;
        }

        verts.clear();
        indices.clear();
    }

    template <typename V>
    void Vb<V>::set_format(std::shared_ptr<VertFormat> format) {
        this->format = format;
        isFormatSet = true;
    }

    template <typename V>
    void Vb<V>::push_back_verts(V vert) {
        verts.push_back(vert);
    }

    template <typename V>
    void Vb<V>::push_back_verts(std::vector<V> appendVerts) {
        verts.insert(verts.end(), appendVerts.begin(), appendVerts.end());
    }

    template <typename V>
    void Vb<V>::push_back_indices(unsigned int idx) {
        indices.push_back(idx);
    }

    template <typename V>
    void Vb<V>::push_back_indices(std::vector<unsigned int> appendIndices) {
        indices.insert(indices.end(), appendIndices.begin(), appendIndices.end());
    }

    template <typename V>
    unsigned int Vb<V>::get_vertices_num() {
        return verts.size();
    }

    template <typename V>
    unsigned int Vb<V>::get_indices_num() {
        return indices.size();
    }

    template <typename V>
    void Vb<V>::build() {
        if (!isFormatSet) {
            throw std::runtime_error("Vertex format not set!");
        }

        glGenBuffers(_VB_BUFF_OBJ_SZ, objs.data());
        
        // Setup VAO
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        
        // Setup VBO, copy data
        glBindBuffer(GL_ARRAY_BUFFER, objs[VB_BUFF_VBO]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(V) * verts.size(), verts.data(), GL_STATIC_DRAW);

        // Setup EBO, copy data
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, objs[VB_BUFF_EBO]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), indices.data(), GL_STATIC_DRAW);

        // Link attributes to locations
        format->set_attribute_pointers();
    }

    template <typename V>
    void Vb<V>::submit(GLenum mode, int indicesStartOff, GLsizei indicesCount) {
        if (!vao) {
            zcl::logger("VB")->error("VB @{} is not ready to submitted!", (void*) this);
            return;
        }

        auto totalCount = (indicesCount == -1) ? (get_indices_num() - indicesStartOff) : indicesCount;

        glBindVertexArray(vao);
        // byte offset (https://stackoverflow.com/questions/23177229/how-to-cast-int-to-const-glvoid)
        glDrawElements(mode, totalCount, GL_UNSIGNED_INT, (char*)(0) + (indicesStartOff * sizeof(unsigned int)));
        // glDrawArrays(mode, startOff, vertsNum);
    }
    
    template <typename V>
    void Vb<V>::submit() {
        submit(GL_TRIANGLES, 0, indices.size());
    }
    // DEFINITIONS (INCLUSION MODEL FOR TEMPLATE CLASSES!) //
}
#endif