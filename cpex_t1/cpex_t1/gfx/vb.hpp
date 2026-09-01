/**
 * gfx::vb - OpenGL Vertex buffer abstraction
 * ZIK@MMXXVI
 */

#ifndef __CPEX_GFX_MESH_GUARD
#define __CPEX_GFX_MESH_GUARD
#include <vector>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

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
        
        GLuint vao = 0;
        VertFormat format;
        bool isFormatSet;
        /** Contains VBO, EBO. Indexed by `VB_BUFF_OBJ` */
        GLuint objs[_VB_BUFF_OBJ_SZ] = { 0, 0 };
        std::vector<V> verts;
        std::vector<unsigned int> indices;
        
        void release_resources() {
            for (auto &&obj: objs) {
                if (obj) {
                    glDeleteBuffers(1, &obj);
                    obj = 0;
                }
            }
            
            glDeleteVertexArrays(1, &vao);
            vao = 0;

            verts.clear();
            indices.clear();
        }
        
    public:
        // `Vb() = default;` does not work since its template / generic class, it will give deleted constructor as a default one
        Vb():
            vao(0),
            objs{0, 0},
            isFormatSet(false) {
            // std::cout << "[GFX] Vb@" << this << " created!" << std::endl;
        }
        
        ~Vb() {
            // std::cout << "[GFX] Vb@" << this << " destroyed!" << std::endl;
            release_resources();
        }

        Vb(const Vb &other) = delete; // (RAII) Disable copy
        Vb& operator=(const Vb &other) = delete; // (RAII) Disable copy

        Vb(Vb &&other): // (RAII) Move
            verts(std::move(other.verts)),
            indices(std::move(other.indices)),
            format(other.format),
            isFormatSet(other.isFormatSet),
            // (replace GL resources with dummy)
            // objs(std::exchange(other.objs, { 0, 0 })),
            vao(std::exchange(other.vao, 0)) {
            
            for (size_t i = 0; i < _VB_BUFF_OBJ_SZ; i++) {
                objs[i] = other.objs[i];
                other.objs[i] = 0;
            }
            // std::cout << "[GFX] Vb@" << this << " <- Vb@" << &other << " moved!" << std::endl;
        }
        
        Vb& operator=(Vb &&other) { // (RAII) Move
            if (this == &other) {
                // Self assignment, no need to move
                return *this;
            }
            
            // std::cout << "[GFX] Vb@" << this << " <- Vb@" << &other << " moved!" << std::endl;
            release_resources();

            std::swap(verts, other.verts);
            std::swap(indices, other.indices);
            std::swap(format, other.format);
            std::swap(isFormatSet, other.isFormatSet);
            std::swap(objs, other.objs);
            std::swap(vao, other.vao);
            return *this;
        }

        void set_format(VertFormat format);
        void push_back_verts(V vert);
        void push_back_verts(std::vector<V> appendVerts);
        void push_back_indices(unsigned int idx);
        void push_back_indices(std::vector<unsigned int> appendIndices);
        void build();
        void submit(GLenum mode, GLint startOff = 0, GLsizei vertsNum = 0);
        void submit();
    };

    // DEFINITIONS (INCLUSION MODEL FOR TEMPLATE CLASSES!) //
    template <typename V>
    void Vb<V>::set_format(VertFormat format) {
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
    void Vb<V>::build() {
        if (!isFormatSet) {
            throw std::runtime_error("Vertex format not set!");
        }

        glGenBuffers(_VB_BUFF_OBJ_SZ, objs);
        
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
        format.set_attribute_pointers();
    }

    template <typename V>
    void Vb<V>::submit(GLenum mode, GLint startOff, GLsizei vertsNum) {
        glBindVertexArray(vao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, objs[VB_BUFF_EBO]);
        glDrawElements(mode, vertsNum, GL_UNSIGNED_INT, (const void *) startOff);
        // glDrawArrays(mode, startOff, vertsNum);
    }
    
    template <typename V>
    void Vb<V>::submit() {
        submit(GL_TRIANGLES, 0, indices.size());
    }
    // DEFINITIONS (INCLUSION MODEL FOR TEMPLATE CLASSES!) //
}
#endif