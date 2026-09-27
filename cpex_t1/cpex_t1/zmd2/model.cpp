/**
 * zmd2::model - `.zmd2` model.
 * ZIK@MMXXVI
 */

#include <stdexcept>
#include <sstream>

#include <zmd2/model.hpp>

// LIBRARIES //
#include <zcl/zcl.hpp>

using namespace zmd2;

Model::Model(std::string id):
    id(id) {}

std::shared_ptr<MeshGroup> Model::reserve_meshgroup(const std::string &materialId) {
    if (meshGroupsByMaterialId.contains(materialId)) {
        return meshGroupsByMaterialId.at(materialId);
    }

    zcl::logger("ZMD2")->warn("MODEL {} MESHGROUP RESERVING => {}", id, materialId);

    // Make a new meshgroup entry
    // auto mesh = std::make_shared<gfx::Vb>();
    auto meshGroup = std::make_shared<MeshGroup>(MeshGroup { .material = nullptr, .mesh = nullptr });

    meshGroups.push_back(meshGroup);
    // materials.push_back(material);
    // meshes.push_back(mesh);

    meshGroupsByMaterialId[materialId] = meshGroup;

    return meshGroup;
}

std::shared_ptr<MeshGroup> Model::find_meshgroup_by_material_id(const std::string &materialId) const {
    auto res = meshGroupsByMaterialId.find(materialId);
    
    if (res == meshGroupsByMaterialId.end()) {
        return nullptr;
    }
    return (*res).second;
}

void Model::submit(gfx::TextureManager &texManager) const {
    // zcl::logger("ZMD2")->warn("SUBMIT MODEL {}...", id);

    for (auto &&meshGroup: meshGroups) {
        auto material = meshGroup->material;
        auto mesh = meshGroup->mesh;

        if (!material || !mesh) {
            if (material) {
                zcl::logger("ZMD2")->error("MODEL {} HAS NO MESH FOR MESHGROUP {}!", id, material->get_id());
            }
            else {
                zcl::logger("ZMD2")->error("MODEL {} HAS NO MESH OR MATERIAL!", id);
            }
            continue;
        }

        // zcl::logger("ZMD2")->warn("MODEL {} SUBMIT! ({}, {} verts)", id, material->get_id(), mesh->get_indices_num());
        material->apply_material(texManager);
        mesh->submit(GL_TRIANGLES, 0);
    }
}

std::shared_ptr<Model> zmd2::load_model_from(const std::string &id, std::istream &in, std::streampos begin, std::streampos end) {
    const static char HEADER_MAGIC[5] = "ZMD2";
    auto seekPrev = in.tellg();

    if (begin == -1) {
        begin = 0;
    }

    if (end == -1) {
        in.seekg(0, std::ios_base::end);
        end = in.tellg();
    }

    in.seekg(begin, std::ios_base::beg);

    // Header
    char headerMagic[5];
    uint8_t headerCompressed;

    in >> headerMagic;
    in >> headerCompressed;
    zcl::logger("ZMD2")->warn("FILE SZ: {} ({}-{})", end - begin, 0 + begin, 0 + end);
    zcl::logger("ZMD2")->warn("HEADER (MAGIC): {} vs {} / {}", HEADER_MAGIC, headerMagic, std::strcmp(HEADER_MAGIC, headerMagic));
    zcl::logger("ZMD2")->warn("HEADER (COMPRESSED): {}", headerCompressed);

    auto dirOff = in.tellg();
    size_t buffSz = end - dirOff;
    std::istream inflated = std::istream(in.rdbuf());
    std::shared_ptr<zcl::zlib::inflated_streambuf> inflatedStreamBuf = std::shared_ptr<zcl::zlib::inflated_streambuf>(nullptr);

    // (inflate if needed)
    if (headerCompressed) {
        // std::stringstream tmp;
        // zcl::zlib::inflate_stream_to(in, tmp, in.tellg(), end);
        auto sb = zcl::zlib::inflated_streambuf(in.rdbuf(), dirOff, end);
        // inflatedStreamBuf = std::make_shared<zcl::zlib::inflated_streambuf>(sb);

        // inflated.set_rdbuf(&(*inflatedStreamBuf));

        zcl::stream::byteistream inflated2 = zcl::stream::byteistream(&sb);
        zcl::logger("ZMD2")->warn("BEFORE READ: ({}/{})", inflated2.eof(), inflated2.fail());
        uint8_t test = 0;
        // inflated2.read(reinterpret_cast<char*>(&test), 1);
        inflated2 >> test;
        zcl::logger("ZMD2")->warn("AFTER READ {}: {} ({}/{})", test, 0 + inflated2.tellg(), inflated2.eof(), inflated2.fail());
    }
    else {
        inflated.set_rdbuf(in.rdbuf());
        // inflated.seekg(dirOff, std::ios_base::beg);

        dirOff = 0;
    }

    // Read offsets
    /*
    uint32_t    dirRigOff = 0, dirRigSz = 0,
                dirPartsOff = 0, dirPartsSz = 0,
                dirMetaOff = 0, dirMetaSz = 0,
                dirExtraOff = 0, dirExtraSz = 0;

    inflated.seekg(0, std::ios_base::end);
    auto inflatedSz = inflated.tellg();
    inflated.seekg(dirOff, std::ios_base::beg);
    auto inflatedSeek = inflated.tellg();

    zcl::logger("ZMD2")->warn("INFLATED SEEK: {} / {}", 0 + inflatedSeek, 0 + inflatedSz);
    inflated >> dirRigOff;
    inflated >> dirRigSz;

    inflated >> dirPartsOff;
    inflated >> dirPartsSz;

    inflated >> dirMetaOff;
    inflated >> dirMetaSz;

    inflated >> dirExtraOff;
    inflated >> dirExtraSz;

    dirRigOff -= dirOff;
    dirPartsOff -= dirOff;
    dirMetaOff -= dirOff;
    dirExtraOff -= dirOff;
    zcl::logger("ZMD2")->warn("DIRECTORY: rig({0:#X}, {1}) / parts({2:#X}, {3}) / meta({4:#X}, {5}) / extra({6:#X}, {7})", dirRigOff, dirRigSz, dirPartsOff, dirPartsSz, dirMetaOff, dirMetaSz, dirExtraOff, dirExtraSz);
    */

    auto model = Model(id);
    return std::make_shared<Model>(model);
}

