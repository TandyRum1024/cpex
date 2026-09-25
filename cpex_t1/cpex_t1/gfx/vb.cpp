/**
 * gfx::vb - OpenGL Vertex buffer abstraction
 * ZIK@MMXXVI
 */

#include <memory>
#include <stdexcept>

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

size_t VertAttribute::get_type_size() const {
    return typeSize;
}

void VertAttribute::set_attribute_pointer() {
    glVertexAttribPointer(location, dim, type, isNormalized, layoutStride * typeSize, (void*) (layoutOff * typeSize));
    glEnableVertexAttribArray(location);
}

VertFormat::VertFormat(std::initializer_list<VertAttribute> attributes):
    size(0) {
        for (auto &&attrib: attributes) {
            add_attribute(attrib);
        }
    }

size_t VertFormat::get_size() const {
    return size;
}

void VertFormat::add_attribute(VertAttribute attrib) {
    attribs.push_back(attrib);
    size += attrib.get_type_size();
}

void VertFormat::set_attribute_pointers() {
    for (auto &&attrib: attribs) {
        attrib.set_attribute_pointer();
    }
}

Vb::Vb():
    vao(0),
    format(nullptr),
    bufferHandles({0}),
    buffers(),
    bufferElementSizes({0}) {
        //glGenVertexArrays(1, &vao);
        // std::cout << "[GFX] Vb@" << this << " created!" << std::endl;
        // zcl::logger("VB")->info("Create VB @{}", (void*)this);
    }

Vb::~Vb() {
    // zcl::logger("VB")->info("Destroy VB @{}", (void*)this);
    release_resources();
}

Vb::Vb(Vb &&other):
    vao(std::exchange(other.vao, 0)), // (replace GL resources with dummy)
    format(std::move(other.format)),
    bufferHandles(std::exchange(other.bufferHandles, {0})),  // (replace GL resources with dummy)
    buffers(std::move(other.buffers)),
    bufferElementSizes(std::exchange(other.bufferElementSizes, {0})) {
        // zcl::logger("VB")->info("Move VB @{} <- @{} (objs: [{}] VS [{}])", (void*)this, (void*)&other, zcl::str::to_str(objs), zcl::str::to_str(other.objs));
        // for (size_t i = 0; i < _VB_BUFF_OBJ_SZ; i++) {
        //     objs[i] = other.objs[i];
        //     other.objs[i] = 0;
        // }
        // std::cout << "[GFX] Vb@" << this << " <- Vb@" << &other << " moved!" << std::endl;
    }

Vb& Vb::operator=(Vb &&other) {
    // zcl::logger("VB")->info("Move VB @{} <- @{} (objs: [{}] VS [{}])", (void*)this, (void*)&other, zcl::str::to_str(objs), zcl::str::to_str(other.objs));

    if (this == &other) {
        // Self assignment, no need to move
        return *this;
    }
    
    // std::cout << "[GFX] Vb@" << this << " <- Vb@" << &other << " moved!" << std::endl;

    // We need manual replacement with dummy value. `GLuint` does not support normal move operations,
    // Instead they simply copy which could cause nasty issues should they be freed in destructors in say, temporary values...
    // So for now let's swap them.
    // Since move assignment makes the current VB unused, and it needs to be freed somehow. For that we can use rvalue which will be destroyed most of the time.
    std::swap(vao, other.vao);
    std::swap(format, other.format);
    std::swap(bufferHandles, other.bufferHandles);
    std::swap(buffers, other.buffers);
    std::swap(bufferElementSizes, other.bufferElementSizes);

    // zcl::logger("VB")->info("\tAfter move VB @{} <- @{} (objs: [{}] VS [{}])", (void*)this, (void*)&other, zcl::str::to_str(objs), zcl::str::to_str(other.objs));

    // other.release_resources(); // automatically called on destruction
    return *this;
}

void Vb::release_resources() {
    for (auto &&handle: bufferHandles) {
        if (handle) {
            glDeleteBuffers(1, &handle);
            handle = 0;
        }
    }
    for (auto &&buff: buffers) {
        buff.clear();
    }
    
    if (vao) {
        glDeleteVertexArrays(1, &vao);
        vao = 0;
    }

    format = nullptr;
}

void Vb::bind() {
    // Setup VAO
    if (!vao) {
        glGenVertexArrays(1, &vao);
        zcl::logger("VB")->trace("VB @{} VAO GENERATING => {}", (void*) this, vao);
    }
    glBindVertexArray(vao);
    zcl::logger("VB")->trace("VB @{} VAO BOUND => {}", (void*) this, vao);
}

void Vb::set_format(std::shared_ptr<VertFormat> format) {
    this->format = format;
}

void Vb::clear_buffer(VB_BUFFER target) {
    auto& buff = buffers[target];
    buff.clear();
    bufferElementSizes[target] = 0;
}

void Vb::clear_buffer_all() {
    clear_buffer(VB_BUFFER_VBO);
    clear_buffer(VB_BUFFER_EBO);
}

void Vb::set_buffer_indices(const std::span<unsigned int> &indices) {
    clear_buffer(VB_BUFFER_EBO);
    append_buffer_indices(indices, 0);
}

void Vb::append_buffer_indices(const std::span<unsigned int> &indices, unsigned int appendBase) {
    if (appendBase == 0) {
        append_buffer<unsigned int>(VB_BUFFER_EBO, indices);
    }
    else {
        auto preprocessed = std::vector<unsigned int>();
    
        preprocessed.resize(indices.size());
        std::transform(indices.begin(), indices.end(), preprocessed.begin(), [appendBase] (unsigned int idx) {
            return idx + appendBase;
        });
        append_buffer<unsigned int>(VB_BUFFER_EBO, preprocessed);
    }
}

unsigned int Vb::get_buffer_size(VB_BUFFER target) const {
    return bufferElementSizes[target];
}

unsigned int Vb::get_indices_num() const {
    return get_buffer_size(VB_BUFFER_EBO);
}

void Vb::build() {
    zcl::logger("VB")->trace("VB @{} BUILDING...", (void*) this);
    // Build & bind VAO
    bind();

    zcl::logger("VB")->trace("\t[VB @{}] BUFFERS...", (void*) this);
    // A] VBO
    auto& buffVbo = buffers[VB_BUFFER_VBO];
    
    if (buffVbo.size() > 0) {
        auto& buffHandle = bufferHandles[VB_BUFFER_VBO];
        if (!buffHandle) {
            zcl::logger("VB")->trace("VB @{} VBO NOT SET!", (void*) this);
            glGenBuffers(1, &buffHandle);
            zcl::logger("VB")->trace("VB @{} VBO GENERATING => {}", (void*) this, buffHandle);
        }
        
        // Copy data to buffer
        glBindBuffer(GL_ARRAY_BUFFER, buffHandle);
        glBufferData(GL_ARRAY_BUFFER, sizeof(uint8_t) * buffVbo.size(), buffVbo.data(), GL_STATIC_DRAW);
    }

    // A] EBO
    auto& buffEbo = buffers[VB_BUFFER_EBO];
    
    if (buffEbo.size() > 0) {
        auto& buffHandle = bufferHandles[VB_BUFFER_EBO];
        if (!buffHandle) {
            zcl::logger("VB")->trace("VB @{} EBO NOT SET!", (void*) this);
            glGenBuffers(1, &buffHandle);
            zcl::logger("VB")->trace("VB @{} EBO GENERATING => {}", (void*) this, buffHandle);
        }
        
        // Copy data to buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffHandle);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint8_t) * buffEbo.size(), buffEbo.data(), GL_STATIC_DRAW);
    }

    zcl::logger("VB")->trace("\t[VB @{}] ATTRIBUTE POINTERS...", (void*) this);
    format->set_attribute_pointers();
}

void Vb::submit(GLenum mode, int indicesStartOff, GLsizei indicesCount) {
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

void Vb::submit() {
    submit(GL_TRIANGLES, 0, get_indices_num());
}
