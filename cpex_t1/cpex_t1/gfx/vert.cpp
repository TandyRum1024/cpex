/**
 * gfx::vert - Vertex data holding classes.
 * ZIK@MMXXVI
 */

#include <gfx/vert.hpp>

using namespace gfx;

VertPosUv::VertPosUv(double x, double y, double z, double u, double v):
            x(x),
            y(y),
            z(z),
            u(u),
            v(v) {}