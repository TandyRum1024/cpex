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
        GLuint vao;
        GLuint vbo;
        std::vector<V> verts;
        
    public:
        VertFormat format;

        // `Vb() = default;` does not work since its template / generic class, it will give deleted constructor as a default one
        Vb();
        ~Vb();

        void push_back(V vert);
        void push_back(std::vector<V> appendVerts);
        void build();
        void submit(GLenum mode, GLint startOff = 0, GLsizei vertsNum = 0);
        void submit(GLenum mode = GL_TRIANGLES);
    };

    // DEFINITIONS (INCLUSION MODEL FOR TEMPLATE CLASSES!)
    template <typename V>
    Vb<V>::Vb() {}

    template <typename V>
    Vb<V>::~Vb() {
        if (vao) {
            glDeleteBuffers(1, &vao);
            vao = 0;
        }
        if (vbo) {
            glDeleteBuffers(1, &vbo);
            vbo = 0;
        }
    }

    template <typename V>
    void Vb<V>::push_back(V vert) {
        verts.push_back(vert);
    }

    template <typename V>
    void Vb<V>::push_back(std::vector<V> appendVerts) {
        verts.insert(verts.end(), appendVerts.begin(), appendVerts.end());
    }

    template <typename V>
    void Vb<V>::build() {
        // Setup VAO
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        // Setup VBO, copy data
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(V) * verts.size(), verts.data(), GL_STATIC_DRAW);

        // Link attributes to locations
        format.set_attribute_pointers();
    }

    template <typename V>
    void Vb<V>::submit(GLenum mode, GLint startOff, GLsizei vertsNum) {
        glBindVertexArray(vao);
        glDrawArrays(mode, startOff, vertsNum);
    }

    template <typename V>
    void Vb<V>::submit(GLenum mode) {
        submit(mode, 0, verts.size());
    }
}
#endif