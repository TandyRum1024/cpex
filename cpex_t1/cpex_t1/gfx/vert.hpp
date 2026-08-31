/**
 * gfx::vert - Vertex data holding classes.
 * ZIK@MMXXVI
 */

#ifndef __CPEX_GFX_VERT_GUARD
#define __CPEX_GFX_VERT_GUARD

namespace gfx {
    #pragma pack(push, 1)
    /** Example struct for vertices data, containing xyz positions and uv coordinates. */
    struct VertPosUv {
        double x;
        double y;
        double z;
        
        double u;
        double v;

    public:
        VertPosUv(double x, double y, double z, double u, double v);
    };
    #pragma pack(pop)
}
#endif