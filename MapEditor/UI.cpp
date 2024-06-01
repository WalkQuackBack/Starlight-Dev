#include "UI.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_internal.h"
#include "imgui_stdlib.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

#include <GLFW/glfw3.h>

#include "ActorMgr.h"
#include "Bfres.h"
#include "Camera.h"
#include "Editor.h"
#include "EditorConfig.h"
#include "FramebufferMgr.h"
#include "Logger.h"
#include "PreferencesConfig.h"
#include "ProjectRebuilder.h"
#include "SceneMgr.h"
#include "ShaderMgr.h"
#include "TextureMgr.h"
#include "UIAINBEditor.h"
#include "UIActorTool.h"
#include "UIConsole.h"
#include "UIMSBTEditor.h"
#include "UIMapView.h"
#include "UIOutliner.h"
#include "UIProperties.h"
#include "UITools.h"
#include "ZStdFile.h"

#include "PopupAINBElementSelector.h"
#include "PopupAddAINBNode.h"
#include "PopupAddActor.h"
#include "PopupAddLink.h"
#include "PopupAddRail.h"
#include "PopupCredits.h"
#include "PopupEditorAINBActorLinks.h"
#include "PopupExportMod.h"
#include "PopupGeneralConfirm.h"
#include "PopupGeneralInputPair.h"
#include "PopupGeneralInputString.h"
#include "PopupLoadScene.h"
#include "PopupSettings.h"

#include "ImGuizmo.h"

GLFWwindow* Window;
ImVec4 ClearColor = ImVec4(0.18f, 0.21f, 0.25f, 1.00f);
bool FirstFrame = true;

void UI::Initialize()
{
    glfwSetErrorCallback([](int Error, const char* Description) {
        Logger::Error("GLFW", Description);
    });
    if (!glfwInit())
        return;

    Window = glfwCreateWindow(1280, 720, "Starlight - The Legend of Zelda: Tears of the Kingdom", nullptr, nullptr);
    if (Window == nullptr)
        return;

    glfwMakeContextCurrent(Window);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    gladLoadGL();

    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigWindowsMoveFromTitleBarOnly = true;
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;

    ImGui::StyleColorsDark();

    ImGuiStyle& Style = ImGui::GetStyle();

    Style.WindowPadding = ImVec2(2, 2);
    Style.FrameRounding = 3;

    /*
    * Dark
        Style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
Style.Colors[ImGuiCol_WindowBg] = ImVec4(0.17f, 0.17f, 0.17f, 0.94f);
Style.Colors[ImGuiCol_ChildBg] = ImVec4(0.30f, 0.30f, 0.30f, 0.00f);
Style.Colors[ImGuiCol_Border] = ImVec4(1.00f, 1.00f, 1.00f, 0.50f);
Style.Colors[ImGuiCol_PopupBg] = ImVec4(0.2f, 0.2f, 0.2f, 0.94f);
Style.Colors[ImGuiCol_FrameBg] = ImVec4(0.09f, 0.09f, 0.09f, 0.40f);
Style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
Style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.42f, 0.42f, 0.42f, 0.67f);
Style.Colors[ImGuiCol_TitleBg] = ImVec4(0.147f, 0.147f, 0.147f, 1.000f);
Style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
Style.Colors[ImGuiCol_CheckMark] = ImVec4(0.37f, 0.53f, 0.71f, 1.00f);
Style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.53f, 0.54f, 0.54f, 1.00f);
Style.Colors[ImGuiCol_Header] = ImVec4(0.37f, 0.37f, 0.37f, 0.31f);
Style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.46f, 0.46f, 0.46f, 0.80f);
Style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
Style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.82f, 0.82f, 0.82f, 0.78f);
Style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.53f, 0.53f, 0.53f, 1.00f);
Style.Colors[ImGuiCol_Separator] = ImVec4(0.21f, 0.21f, 0.21f, 1.00f);
Style.Colors[ImGuiCol_Tab] = ImVec4(0.16f, 0.16f, 0.16f, 0.86f);
Style.Colors[ImGuiCol_TabHovered] = ImVec4(0.22f, 0.22f, 0.22f, 0.80f);
Style.Colors[ImGuiCol_TabActive] = ImVec4(0.27f, 0.27f, 0.27f, 1.00f);
Style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.12f, 0.12f, 0.12f, 0.98f);
Style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
Style.Colors[ImGuiCol_DockingPreview] = ImVec4(0.34f, 0.34f, 0.34f, 0.70f);
Style.Colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
Style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.24f, 0.45f, 0.68f, 0.35f);
Style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.4f, 0.4f, 0.4f, 0);
Style.Colors[ImGuiCol_Button] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
Style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.17f, 0.17f, 0.17f, 0.94f);
    */

    /*
    Style.Colors[ImGuiCol_Text] = ImVec4(0.8f, 0.83f, 0.96f, 1.0f);
Style.Colors[ImGuiCol_WindowBg] = ImVec4(0.16f, 0.18f, 0.21f, 0.94f);
Style.Colors[ImGuiCol_ChildBg] = ImVec4(0.16f, 0.18f, 0.21f, 0.0f);
Style.Colors[ImGuiCol_Border] = ImVec4(0.13f, 0.14f, 0.16f, 0.50f);
Style.Colors[ImGuiCol_PopupBg] = ImVec4(0.2f, 0.21f, 0.25f, 0.94f);
Style.Colors[ImGuiCol_FrameBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.4f);
Style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
Style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.16f, 0.18f, 0.21f, 0.67f);
Style.Colors[ImGuiCol_TitleBg] = ImVec4(0.14f, 0.15f, 0.18f, 1.0f);
Style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.13f, 0.14f, 0.17f, 1.0f);
Style.Colors[ImGuiCol_CheckMark] = ImVec4(0.37f, 0.53f, 0.71f, 1.0f);
Style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.53f, 0.54f, 0.54f, 1.0f);
Style.Colors[ImGuiCol_Header] = ImVec4(0.32f, 0.34f, 0.38f, 0.31f);
Style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.46f, 0.46f, 0.46f, 0.8f);
Style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
Style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.82f, 0.82f, 0.82f, 0.78f);
Style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.53f, 0.53f, 0.53f, 1.0f);
Style.Colors[ImGuiCol_Separator] = ImVec4(0.21f, 0.21f, 0.21f, 1.0f);
Style.Colors[ImGuiCol_Tab] = ImVec4(0.11f, 0.12f, 0.15f, 0.86f);
Style.Colors[ImGuiCol_TabHovered] = ImVec4(0.19f, 0.19f, 0.22f, 0.8f);
Style.Colors[ImGuiCol_TabActive] = ImVec4(0.16f, 0.18f, 0.21f, 1.0f);
Style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.13f, 0.13f, 0.14f, 0.98f);
Style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.16f, 0.18f, 0.21f, 1.0f);
Style.Colors[ImGuiCol_DockingPreview] = ImVec4(0.12f, 0.12f, 0.14f, 0.7f);
Style.Colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.1f, 0.1f, 0.11f, 1.0f);
Style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.24f, 0.45f, 0.68f, 0.35f);
Style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.66f, 1.0f, 0.0f);
Style.Colors[ImGuiCol_Button] = ImVec4(0.24f, 0.25f, 0.29f, 1.0f);*/

    Style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    Style.Colors[ImGuiCol_WindowBg] = ImVec4(0.17f, 0.17f, 0.17f, 0.94f);
    Style.Colors[ImGuiCol_ChildBg] = ImVec4(0.30f, 0.30f, 0.30f, 0.00f);
    Style.Colors[ImGuiCol_Border] = ImVec4(1.00f, 1.00f, 1.00f, 0.50f);
    Style.Colors[ImGuiCol_PopupBg] = ImVec4(0.2f, 0.2f, 0.2f, 0.94f);
    Style.Colors[ImGuiCol_FrameBg] = ImVec4(0.09f, 0.09f, 0.09f, 0.40f);
    Style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    Style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.42f, 0.42f, 0.42f, 0.67f);
    Style.Colors[ImGuiCol_TitleBg] = ImVec4(0.147f, 0.147f, 0.147f, 1.000f);
    Style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
    Style.Colors[ImGuiCol_CheckMark] = ImVec4(0.37f, 0.53f, 0.71f, 1.00f);
    Style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.53f, 0.54f, 0.54f, 1.00f);
    Style.Colors[ImGuiCol_Header] = ImVec4(0.37f, 0.37f, 0.37f, 0.31f);
    Style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.46f, 0.46f, 0.46f, 0.80f);
    Style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    Style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.82f, 0.82f, 0.82f, 0.78f);
    Style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.53f, 0.53f, 0.53f, 1.00f);
    Style.Colors[ImGuiCol_Separator] = ImVec4(0.21f, 0.21f, 0.21f, 1.00f);
    Style.Colors[ImGuiCol_Tab] = ImVec4(0.16f, 0.16f, 0.16f, 0.86f);
    Style.Colors[ImGuiCol_TabHovered] = ImVec4(0.22f, 0.22f, 0.22f, 0.80f);
    Style.Colors[ImGuiCol_TabActive] = ImVec4(0.27f, 0.27f, 0.27f, 1.00f);
    Style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.12f, 0.12f, 0.12f, 0.98f);
    Style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    Style.Colors[ImGuiCol_DockingPreview] = ImVec4(0.34f, 0.34f, 0.34f, 0.70f);
    Style.Colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    Style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.24f, 0.45f, 0.68f, 0.35f);
    Style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.4f, 0.4f, 0.4f, 0);
    Style.Colors[ImGuiCol_Button] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
    Style.Colors[ImGuiCol_MenuBarBg] = Style.Colors[ImGuiCol_WindowBg];

    glfwSetKeyCallback(Window, UIMapView::GLFWKeyCallback);

    ImGui_ImplGlfw_InitForOpenGL(Window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /*
    GLFWimage Icons[2];
    TextureMgr::Texture* WindowIcon = TextureMgr::GetTexture("LogoWhiteYellow64x64");
    TextureMgr::Texture* WindowIconSmall = TextureMgr::GetTexture("LogoWhiteYellow64x64");
    Icons[0].width = WindowIcon->Width;
    Icons[0].height = WindowIcon->Height;
    Icons[0].pixels = WindowIcon->Pixels.data();
    Icons[1].width = WindowIconSmall->Width;
    Icons[1].height = WindowIconSmall->Height;
    Icons[1].pixels = WindowIconSmall->Pixels.data();
    glfwSetWindowIcon(Window, 2, Icons);
    */

    UIMapView::Initialize(Window);
    UIOutliner::Initalize(UIMapView::CameraView);

    Logger::Info("Renderer", "Initialized OpenGL and ImGui");
}

bool UI::ShouldWindowClose()
{
    return glfwWindowShouldClose(Window);
}

void UI::Render()
{
    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    glClearColor(ClearColor.x * ClearColor.w, ClearColor.y * ClearColor.w, ClearColor.z * ClearColor.w, ClearColor.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    ImGui::NewFrame();
    ImGuizmo::BeginFrame();

    ImGuiID DockSpace = ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

    UIProperties::DrawPropertiesWindow();
    UIMapView::DrawMapViewWindow();
    UIOutliner::DrawOutlinerWindow();
    UITools::DrawToolsWindow();
    UIConsole::DrawConsoleWindow();
    UIActorTool::DrawActorToolWindow();
#ifdef ADVANCED_MODE
    UIMSBTEditor::DrawMSBTEditorWindow();
#endif

    // if (!UIAINBEditor::AINB.Loaded) UIAINBEditor::LoadAINBFile("Logic/Dungeon001_1800.logic.root.ainb");
    UIAINBEditor::DrawAinbEditorWindow();

    if (ImGui::BeginMainMenuBar()) {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 6));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 4));
        ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
        if (ImGui::BeginMenu("Scene")) {
            if (ImGui::MenuItem("Load")) {
                PopupLoadScene::Open([](SceneMgr::Type Type, std::string Identifier) {
						SceneMgr::LoadScene(Type, Identifier);
; });
            }
            if (Editor::Identifier.empty())
                ImGui::BeginDisabled();
            if (ImGui::MenuItem("Reload")) {
                SceneMgr::Reload();
            }
            if (Editor::Identifier.empty())
                ImGui::EndDisabled();

            ImGui::Separator();
            if (ImGui::MenuItem("Rebuild project")) {
                PopupGeneralConfirm::Open("Do you really want to rebuild the project?\n(If unsure, don't do it)", []() {
                    ProjectRebuilder::RebuildProject();
                });
            }

            ImGui::EndMenu();
        }
        if (ImGui::MenuItem("Settings")) {
            PopupSettings::Open();
        }
        if (ImGui::BeginMenu("Window")) {
            if (ImGui::MenuItem("Console", "", UIConsole::Open)) {
                UIConsole::Open = !UIConsole::Open;
            }
            if (ImGui::MenuItem("Outliner", "", UIOutliner::Open)) {
                UIOutliner::Open = !UIOutliner::Open;
            }
            if (ImGui::MenuItem("Properties", "", UIProperties::Open)) {
                UIProperties::Open = !UIProperties::Open;
            }
            if (ImGui::MenuItem("Tools", "", UITools::Open)) {
                UITools::Open = !UITools::Open;
            }
            if (ImGui::MenuItem("Map View", "", UIMapView::Open)) {
                UIMapView::Open = !UIMapView::Open;
            }
            if (ImGui::MenuItem("AINB Editor", "", UIAINBEditor::Open)) {
                UIAINBEditor::Open = !UIAINBEditor::Open;
            }
            if (ImGui::MenuItem("Actor Tool", "", UIActorTool::Open)) {
                UIActorTool::Open = !UIActorTool::Open;
            }
#ifdef ADVANCED_MODE
            if (ImGui::MenuItem("MSTB Editor", "", UIActorTool::Open)) {
                UIMSBTEditor::Open = !UIMSBTEditor::Open;
                UpdatePreferences = true;
            }
#endif

            ImGui::EndMenu();
        }
        if (ImGui::MenuItem("Credits")) {
            PopupCredits::Open();
        }
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor();
        ImGui::EndMainMenuBar();
    }

    // ImGui::ShowDemoWindow();

    if (FirstFrame) {
        ImGui::DockBuilderRemoveNode(DockSpace);
        ImGui::DockBuilderAddNode(DockSpace, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(DockSpace, ImGui::GetMainViewport()->Size);

        ImGuiID DockLeft, DockMiddle, DockRight;
        ImGui::DockBuilderSplitNode(DockSpace, ImGuiDir_Left, 0.2f, &DockLeft, &DockMiddle);
        ImGui::DockBuilderSplitNode(DockMiddle, ImGuiDir_Right, 0.35f, &DockRight, &DockMiddle);

        ImGuiID DockOutlinerTop, DockOutlinerBottom;
        ImGui::DockBuilderSplitNode(DockLeft, ImGuiDir_Down, 0.4f, &DockOutlinerBottom, &DockOutlinerTop);
        ImGui::DockBuilderDockWindow("Outliner", DockOutlinerTop);
        ImGui::DockBuilderDockWindow("Tools", DockOutlinerBottom);

        ImGuiID DockMapViewTop, DockMapViewBottom;
        ImGui::DockBuilderSplitNode(DockMiddle, ImGuiDir_Down, 0.3f, &DockMapViewBottom, &DockMapViewTop);
        ImGui::DockBuilderDockWindow("Map View", DockMapViewTop);
        ImGui::DockBuilderDockWindow("AINB Editor", DockMapViewTop);
        ImGui::DockBuilderDockWindow("Actor Tool", DockMapViewTop);
#ifdef ADVANCED_MODE
        ImGui::DockBuilderDockWindow("MSBT Editor", DockMapViewTop);
#endif
        ImGui::DockBuilderDockWindow("Console", DockMapViewBottom);

        ImGui::DockBuilderDockWindow("Properties", DockRight);

        ImGui::DockBuilderFinish(DockSpace);

        ImGui::SetWindowFocus("Outliner");

        FirstFrame = false;
    }

    PopupAddActor::Render();
    PopupGeneralInputPair::Render();
    PopupAddRail::Render();
    PopupAddLink::Render();
    PopupSettings::Render();
    PopupGeneralConfirm::Render();
    PopupGeneralInputString::Render();
    PopupLoadScene::Render();
    PopupExportMod::Render();
    PopupAddAINBNode::Render();
    PopupEditorAINBActorLinks::Render();
    PopupAINBElementSelector::Render();
    PopupCredits::Render();

    PreferencesConfig::Frame();

    // Rendering
    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(Window);
}

void UI::Cleanup()
{
    UIAINBEditor::Delete();
    TextureMgr::Cleanup();
    BfresLibrary::Cleanup();
    GLTextureLibrary::Cleanup();
    ShaderMgr::Cleanup();
    FramebufferMgr::Cleanup();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(Window);
    glfwTerminate();

    Logger::Info("System", "Shutdown completed");
}