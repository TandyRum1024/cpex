/**
 * gfx::vb - OpenGL Vertex buffer abstraction
 * ZIK@MMXXVI
 */

#include <gfx/vb.hpp>

using namespace gfx;

VertAttribute::VertAttribute(GLuint location, int dim, GLenum type, size_t typeSize, GLsizei layoutStride, int layoutOff, GLboolean isNormalized):
            location(location),
            dim(dim),
            type(type),
            typeSize(typeSize),
            layoutStride(layoutStride),
            layoutOff(layoutOff),
            isNormalized(isNormalized) {}

void VertAttribute::set_attribute_pointer() {
    glVertexAttribPointer(location, dim, type, isNormalized, layoutStride * typeSize, (void*) (layoutOff * typeSize));
    glEnableVertexAttribArray(location);
}

VertFormat::VertFormat(std::initializer_list<VertAttribute> attributes) {
    for (auto &&attrib: attributes) {
        add_attribute(attrib);
    }
}

void VertFormat::add_attribute(VertAttribute attrib) {
    attribs.push_back(attrib);
}

void VertFormat::set_attribute_pointers() {
    for (auto &&attrib: attribs) {
        attrib.set_attribute_pointer();
    }
}
