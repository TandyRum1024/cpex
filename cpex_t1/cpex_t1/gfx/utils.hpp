/**
 * gfx::utils - OpenGL helpers (tables etc)
 * ZIK@MMXXVI
 */

#ifndef __CPEX_GFX_UTILS_GUARD
#define __CPEX_GFX_UTILS_GUARD

#include <string>

// EXTERNAL LIBRARIES //
// ----------------------------
// OpenGL: GLAD
#include <glad/gl.h>
// OpenGL: GLFW
#include <GLFW/glfw3.h>
// ----------------------------
// EXTERNAL LIBRARIES //

namespace gfx {
    /** Returns name of OpenGL texture format. Empty string if not recognized. */
    std::string get_name_from_gl_tex_format(const GLint &format);
}

#endif