/**
 * CPEX - T1, C++ (re)primer and OpenGL basics
 * ZIK@MMXXVI
 */

#define DEBUG_INIT_AND_QUIT false

#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <memory>

#include <app.hpp>

// LIBRARIES //
#include <zcl/zcl.hpp>
#include <gfx/vb.hpp>
#include <gfx/math.hpp>
#include <gfx/transform.hpp>
#include <zmd2/zmd2.hpp>

// EXTERNAL LIBRARIES //
// ----------------------------
// ImGui
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
// GLM
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
// STB
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
// ----------------------------
// EXTERNAL LIBRARIES //

using Zmd2 = mdl::zmd2::Model;

CpexApp::CpexApp(const std::string windowTitle, bool isGlDebug):
    zap::OpenGlApp(windowTitle, isGlDebug) {}
CpexApp::CpexApp():
    CpexApp("WINDOW TITLE", false) {}
CpexApp::~CpexApp() {
    free_resources();
}
CpexApp::CpexApp(CpexApp &&other):
    OpenGlApp(std::move(other)),
    time(std::exchange(other.time, 0.0)),
    tf(other.tf),
    model(std::move(other.model)),
    imGuiContext(other.imGuiContext),
    texManager(gfx::TextureManager())
    {
    std::swap(windowTitle, other.windowTitle);
    std::swap(window, other.window);

    other.imGuiContext = nullptr;
}
CpexApp& CpexApp::operator=(CpexApp &&other) {
    if (this == &other) {
        // Self assignment, no need to move
        return *this;
    }   

    std::swap(time, other.time);
    std::swap(tf, other.tf);

    std::swap(model, other.model);
    std::swap(imGuiContext, other.imGuiContext);

    OpenGlApp::operator=(std::move(other));

    return *this;
}

void CpexApp::free_imgui() {
    if (imGuiContext) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext(imGuiContext);
    }
    imGuiContext = nullptr;
}

void CpexApp::on_setup() {
    set_vsync(true);

    // Relative path
    auto assetPath = zcl::io::get_exec_path().parent_path() / "data";
    _logger->info("Detected asset path: {}", assetPath.string());

    // Init managers
    gfx::TextureManager::init();
    assetManager.set_base_path(assetPath);
    mdl::zmd2::init(assetManager, assetPath);

    // Setup scene
    time = 0;

    // Assets
    auto texChecker = assetManager.load_texture("checker.png");
    auto texCat1 = assetManager.load_texture("sprtest.png");
    auto texCat2 = assetManager.load_texture("tex_test.png");
    auto texFace1 = assetManager.load_texture("tex_test1.png");
    auto texFace2 = assetManager.load_texture("tex_test2.png");

    auto shdBase = assetManager.load_shader("base");
    auto shdTest = assetManager.load_shader("test");

    auto matBase = std::make_shared<gfx::Material>("base");
    auto matCubeBase = gfx::material_make_inherited(matBase, "matCubeBase");
    auto matCube1 = gfx::material_make_inherited(matCubeBase, "cube1");
    auto matCube2 = gfx::material_make_inherited(matCubeBase, "cube2");

    matBase->set_shader(shdBase);
    matBase->add_uniforms(
        gfx::UniformVec4("uTint", {1.0, 1.0, 1.0, 1.0}),
        gfx::UniformMat4("uMatModel", glm::mat4(1.0f)),
        gfx::UniformMat4("uMatView", glm::mat4(1.0f)),
        gfx::UniformMat4("uMatProjection", glm::mat4(1.0f)),
        gfx::UniformSampler("uAlbedo", texChecker, GL_NEAREST, GL_REPEAT)
    );

    matCubeBase->add_uniforms(
        gfx::UniformMat4("uMatModel", glm::mat4(1.0f))
    );
    matCube1->add_uniforms(
        gfx::UniformSampler("uAlbedo", texFace1, GL_NEAREST, GL_MIRRORED_REPEAT)
    );
    matCube2->add_uniforms(
        gfx::UniformSampler("uAlbedo", texFace2, GL_NEAREST, GL_MIRRORED_REPEAT)
    );

    assetManager.add_material(matBase->get_id(), matBase);
    assetManager.add_material(matCube1->get_id(), matCube1);
    assetManager.add_material(matCube2->get_id(), matCube2);

    // assetManager.load_model_zmd2("mdl_char.zmd2");
    /*
    if (auto file = std::ifstream(assetPath / "mdl_char.zmd2", std::ios_base::binary); file) {
        auto time = std::chrono::steady_clock::now();

        auto mdl = zmd2::load_model_from("char", file);

        std::chrono::duration<double, std::milli> duration = std::chrono::steady_clock::now() - time;
        _logger->info("Loading took: {}ms", duration.count());

        time = std::chrono::steady_clock::now();

        auto mdlConverted = Zmd2(mdl);

        mdlConverted.load_and_find_embedded_assets(assetManager, matBase, texChecker);
        mdlConverted.update_refs();

        duration = std::chrono::steady_clock::now() - time;
        _logger->info("Converting and updating took: {}ms", duration.count());
    }
    */

    // Build models
    auto floorSize = 4.0;
    auto vertsFloor = std::vector {
        gfx::VertPosUv(glm::vec3(-0.5, -0.5, 0.0) * glm::vec3(floorSize), glm::vec2(0.0, 0.0) * glm::vec2(floorSize)),
        gfx::VertPosUv(glm::vec3(0.5, -0.5, 0.0) * glm::vec3(floorSize), glm::vec2(1.0, 0.0) * glm::vec2(floorSize)),
        gfx::VertPosUv(glm::vec3(-0.5, 0.5, 0.0) * glm::vec3(floorSize), glm::vec2(0.0, 1.0) * glm::vec2(floorSize)),
        gfx::VertPosUv(glm::vec3(0.5, 0.5, 0.0) * glm::vec3(floorSize), glm::vec2(1.0, 1.0) * glm::vec2(floorSize)),
    };
    auto indices = std::vector<unsigned int> {
        0, 1, 2,
        1, 2, 3,
    };

    auto vertsCube1 = std::vector {
        gfx::VertPosUv({-0.5, -0.5, -0.5}, {0.0, 0.0}),
        gfx::VertPosUv({0.5, -0.5, -0.5}, {1.0, 0.0}),
        gfx::VertPosUv({-0.5, 0.5, -0.5}, {0.0, 1.0}),
        gfx::VertPosUv({0.5, 0.5, -0.5}, {1.0, 1.0}),

        gfx::VertPosUv({-0.5, -0.5, -0.5}, {0.0, 0.0}),
        gfx::VertPosUv({-0.5, 0.5, -0.5}, {1.0, 0.0}),
        gfx::VertPosUv({-0.5, -0.5, 0.5}, {0.0, 1.0}),
        gfx::VertPosUv({-0.5, 0.5, 0.5}, {1.0, 1.0}),

        gfx::VertPosUv({0.5, -0.5, -0.5}, {0.0, 0.0}),
        gfx::VertPosUv({0.5, 0.5, -0.5}, {1.0, 0.0}),
        gfx::VertPosUv({0.5, -0.5, 0.5}, {0.0, 1.0}),
        gfx::VertPosUv({0.5, 0.5, 0.5}, {1.0, 1.0}),
    };

    auto vertsCube2 = std::vector {
        gfx::VertPosUv({-0.5, -0.5, 0.5}, {0.0, 0.0}),
        gfx::VertPosUv({0.5, -0.5, 0.5}, {1.0, 0.0}),
        gfx::VertPosUv({-0.5, 0.5, 0.5}, {0.0, 1.0}),
        gfx::VertPosUv({0.5, 0.5, 0.5}, {1.0, 1.0}),

        gfx::VertPosUv({-0.5, -0.5, -0.5}, {0.0, 0.0}),
        gfx::VertPosUv({0.5, -0.5, -0.5}, {1.0, 0.0}),
        gfx::VertPosUv({-0.5, -0.5, 0.5}, {0.0, 1.0}),
        gfx::VertPosUv({0.5, -0.5, 0.5}, {1.0, 1.0}),

        gfx::VertPosUv({-0.5, 0.5, -0.5}, {0.0, 0.0}),
        gfx::VertPosUv({0.5, 0.5, -0.5}, {1.0, 0.0}),
        gfx::VertPosUv({-0.5, 0.5, 0.5}, {0.0, 1.0}),
        gfx::VertPosUv({0.5, 0.5, 0.5}, {1.0, 1.0}),
    };

    auto indicesCube = std::vector<unsigned int> {
        0, 1, 2,
        1, 2, 3,

        4, 5, 6,
        5, 6, 7,

        8, 9, 10,
        9, 10, 11,
    };
    
    auto vbChecker = std::make_shared<gfx::Vb>();

    vbChecker->set_format(gfx::VertPosUv::format);
    vbChecker->set_buffer<gfx::VertPosUv>(gfx::Vb::VB_BUFFER_VBO, vertsFloor);
    vbChecker->set_buffer_indices(indices);
    vbChecker->build();

    auto vbCube1 = std::make_shared<gfx::Vb>();
    auto vbCube2 = std::make_shared<gfx::Vb>();

    vbCube1->set_format(gfx::VertPosUv::format);
    vbCube1->set_buffer<gfx::VertPosUv>(gfx::Vb::VB_BUFFER_VBO, vertsCube1);
    vbCube1->set_buffer_indices(indicesCube);
    vbCube1->build();
    vbCube2->set_format(gfx::VertPosUv::format);
    vbCube2->set_buffer<gfx::VertPosUv>(gfx::Vb::VB_BUFFER_VBO, vertsCube2);
    vbCube2->set_buffer_indices(indicesCube);
    vbCube2->build();

    // Model
    model = std::make_shared<Zmd2>("test");
    auto partCube = std::make_shared<mdl::zmd2::PartModel>("cube");
    auto partFloor = std::make_shared<mdl::zmd2::PartModel>("floor");

    matBase = assetManager.get_material("base");
    matCube1 = assetManager.get_material("cube1");
    matCube2 = assetManager.get_material("cube2");

    auto matMeshCube1 = partCube->reserve_matmesh("cube1", matCube1);
    auto matMeshCube2 = partCube->reserve_matmesh("cube2", matCube2);
    
    matMeshCube1->mesh = vbCube1;
    matMeshCube2->mesh = vbCube2;
    
    auto matMeshFloor = partFloor->reserve_matmesh("base", matBase);

    matMeshFloor->mesh = vbChecker;
    
    model->add_part(partCube);
    model->add_part(partFloor);
    
    // Setup ImGui
    imGuiContext = ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    // Misc.
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (auto mat = assetManager.get_material("hello"); mat) {
        _logger->info("MATERIAL {}", mat->get_id());
        for (auto&& uniform: mat->get_uniforms()) {
            _logger->info("\t {} @ {}", uniform->get_name(), fmt::ptr(&(*uniform)));
        }
    }

    if (auto mat = assetManager.get_material("hello2"); mat) {
        _logger->info("MATERIAL {}", mat->get_id());
        for (auto&& uniform: mat->get_uniforms()) {
            _logger->info("\t {} @ {}", uniform->get_name(), fmt::ptr(&(*uniform)));
        }
    }

    _logger->info("Setup done");
    
    if (DEBUG_INIT_AND_QUIT) {
        exit(0);
    }
    // assert(false);
}

void CpexApp::on_free_resource() {
    _logger->debug("Freeing resources...");
    free_imgui();
}

void CpexApp::on_shutdown() {
    _logger->debug("App shutdown...");
}

void CpexApp::on_loop_update(double dtMillis) {
    set_window_title(std::string("CT1 (DT: ") + std::to_string(dtMillis) + "ms)");

    time += dtMillis * 0.001;
    // time = fmod(time, 1.0);
}

void CpexApp::on_loop_render_begin(double dtMillis) {
    if (!imGuiContext) {
        return;
    }

    ImGui_ImplGlfw_NewFrame();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();
}

void CpexApp::on_loop_render(double dtMillis) {
    // _logger->info("Rrender begin");

    auto angle = time * glm::pi<double>();
    auto camPos = glm::vec3(
        glm::cos(angle) * 1,
        glm::sin(angle) * 1,
        1
    );
    auto matProj = glm::perspectiveFov(90.0, (double)windowWid, (double)windowHei, 0.001, 1024.0);
    auto matView = glm::lookAt(camPos, glm::vec3(0.0), glm::vec3(0.0, 0.0, 1.0));

    auto mdlTest = assetManager.load_model_zmd2("mdl_char.zmd2");
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    texManager.clear();
    
    // Floor
    if (auto part = model->find_part_by_id<mdl::zmd2::PartModel>("floor")) {
        if (auto material = part->find_matmesh_by_material_id("base")->material; material) {
            if (auto uniform = material->get_uniform<gfx::UniformVec4>("uTint")) {
                uniform->set_value({ (float) time, (float) time, (float) time, 1.0 });
            }
            if (auto uniform = material->get_uniform<gfx::UniformMat4>("uMatModel")) {
                auto tf = this->tf.to_mat4();
    
                uniform->set_value(tf);
            }
            if (auto uniform = material->get_uniform<gfx::UniformMat4>("uMatView")) {
                uniform->set_value(matView);
            }
            if (auto uniform = material->get_uniform<gfx::UniformMat4>("uMatProjection")) {
                uniform->set_value(matProj);
            }
        }
    }

    // Cube
    if (auto part = model->find_part_by_id<mdl::zmd2::PartModel>("cube")) {
        if (auto material = part->find_matmesh_by_material_id("cube1")->material; material) {
            if (auto uniform = material->get_uniform<gfx::UniformMat4>("uMatModel")) {
                auto tfCube = gfx::Transform(
                    glm::vec3(0, 0, 0.5),
                    glm::vec3(glm::radians(45.0), 0, time * -1.25 * glm::pi<double>()),
                    glm::vec3(0.5)
                );
                auto mat = tfCube.to_mat4();

                uniform->set_value(mat);
            }
        }
    }

    model->submit(texManager);

    mdlTest->submit(texManager);

    // model2->submit(texManager);
    // assert(false);
}

void imgui_draw_stopwatch_node(const std::shared_ptr<zcl::trace::StopwatchSplitNode> node) {
    if (!node) {
        return;
    }

    auto flags = ImGuiTreeNodeFlags_None
                        | (node->parent.expired() ? ImGuiTreeNodeFlags_DefaultOpen : 0)
                        | (node->children.empty() ? ImGuiTreeNodeFlags_Leaf : 0);
    auto totalTime = 0.0;
    
    if (ImGui::TreeNodeEx(fmt::format("{0} ({1:.2f}ms)###{0}", node->id, node->duration.count()).c_str(), flags)) {
        for (auto&& child: node->children) {
            totalTime += child->duration.count();
            imgui_draw_stopwatch_node(child);
        }
        ImGui::TreePop();
    }
    if (!node->children.empty()) {
        ImGui::TextColored(ImVec4(1, 1, 1, 0.5), fmt::format("({}: {:.2f}ms total sans overhead)", node->id, totalTime).c_str());
    }
}

void CpexApp::on_loop_debug_ui(double dtMillis) {
    if (!imGuiContext) {
        return;
    }

    // ImGui
    //ImGui::ShowDemoWindow(); // Show demo window! :)
    ImGui::SetNextWindowSize(ImVec2(256, 256), ImGuiCond_Once);
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    if (ImGui::Begin("Scene", nullptr, 0)) {
        auto& repo = zcl::trace::StopwatchRepository::get_instance();
        auto roots = repo.get_all_splits_and_childs();
        auto frameTime = zcl::trace::stopwatch_get("frame")->calc_duration().count();
        auto renderTime = zcl::trace::stopwatch_get("render_all")->calc_duration().count();

        ImGui::BulletText("time: %.2lf (dt: %.2lfms, FPS: %2.2lf)", time, dtMillis, (dtMillis == 0.0) ? 0 : (1000 / dtMillis));
        ImGui::BulletText("frame time: %.2lfms (%2.2lf%% | FPS: %2.2lf)", frameTime, (frameTime / 1000.0) * 100.0, (frameTime == 0.0) ? 0 : (1000 / frameTime));
        ImGui::BulletText("render time: %.2lfms (%2.2lf%% | FPS: %2.2lf)", renderTime, (renderTime / 1000.0) * 100.0, (renderTime == 0.0) ? 0 : (1000 / renderTime));
        
        ImGui::BulletText("[SECTIONS (%d)]", roots.size());
        for (auto&& root: roots) {
            imgui_draw_stopwatch_node(root);
        }

        ImGui::BulletText("TextureManager: %d/%d", texManager.get_allocated_num(), gfx::TextureManager::SLOTS_MAX);

        auto rot = glm::degrees(glm::eulerAngles(tf.rot));

        ImGui::DragFloat3("pos", glm::value_ptr(tf.pos));
        if (ImGui::DragFloat3("rot", glm::value_ptr(rot))) {
            tf.rot = glm::quat(glm::radians(rot));
        }
        ImGui::DragFloat3("scale", glm::value_ptr(tf.scale));
        
    }
    ImGui::End();

    ImGui::SetNextWindowSize(ImVec2(256, 256), ImGuiCond_Once);
    ImGui::SetNextWindowPos(ImVec2(windowWid - 256, 0), ImGuiCond_Always);
    if (ImGui::Begin("Models", nullptr, 0)) {

    }
    ImGui::End();
    
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void CpexApp::on_window_key(GLFWwindow* win, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(win, true);
    }
}