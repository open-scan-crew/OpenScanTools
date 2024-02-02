// ImGui - standalone example application for GLFW + OpenGL2, using legacy fixed pipeline
// If you are new to ImGui, see examples/README.txt and documentation at the top of imgui.cpp.
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan graphics context creation, etc.)

// **DO NOT USE THIS CODE IF YOUR CODE/ENGINE IS USING MODERN OPENGL (SHADERS, VBO, VAO, etc.)**
// **Prefer using the code in the opengl3_example/ folder**
// See imgui_impl_glfw.cpp for details.

// NOTE(nico) le world space est right-handed, Z up

// FIXME(nico) bug, parfois crash au démarrage, j'ai l'impression qu'il y a un pb de synchro/mutex sur '_transfertsScheduled' de StreamCPU

#include <imgui.h>
#include "imgui_impl_glfw.h"

#include <sstream>
#include <stdio.h>
#include <GLFW/glfw3.h>

//--------------------------------------------------------

#define NLL_MATH_DEFINITIONS
#include "nll_math.h"

#include "nll_camera.h"

#include "algo.h"
#ifdef _USE_FAKE_OCTREE_
#include "Octree_fake.h"
#else
#include "ScanReal.h"
#endif

#define PI_ 3.1415926535f

void ImGui_Vec2(const char *title, float const v[2]) {
    ImGui::Text("%s %f, %f", title, v[0], v[1]);
}

void ImGui_Vec3(const char *title, nllVec3 v) {
    ImGui::Text("%s %f, %f, %f", title, v[0], v[1], v[2]);
}

void ImGui_Vec4(const char *title, const float v[4]) {
    ImGui::Text("%s %f, %f, %f, %f", title, v[0], v[1], v[2], v[3]);
}

nllCamera buildInitialCamera() {

    float const camPos[] = { 0.f, -0.5f, 1.5f }; //{ 0, 10*sinf(test), -2 };
    float const camTgt[] = { 0, 0, 1.5f };

    float const znear = 0.1f;

    float const world_up[] = {0.f, 0.f, 1.f};

    nllCamera camera;
    camera.znear = znear;
    nllVec3Assign(camera.pos, camPos);
    nllVec3Sub(camera.z, camTgt, camPos);
    nllVec3Normalize(camera.z);
    // NOTE(nico) we assume we never have |camera.z| == world_up
#if NLL_CAM_WS_RIGHT_HANDED
	nllVec3Cross(camera.x, camera.z, world_up);
	nllVec3Normalize(camera.x);
	nllVec3Cross(camera.y, camera.x, camera.z);
#else
	nllVec3Cross(camera.x, world_up, camera.z);
    nllVec3Normalize(camera.x);
    nllVec3Cross(camera.y, camera.z, camera.x);
#endif
	// TODO(nico) check that camera.xyz forms a left-handed y-up coordinate system

    return camera;
}

int rayPlaneHit(float *out_t, nllVec3 const rPos, nllVec3 const rDir) {

    if (rDir[1] == 0) return 0;
    float dy = -rPos[1];
    *out_t = dy / rDir[1];
    return 1;
}

//--------------------------------------------------------
//--------------------------------------------------------

void drawWorld(PermanentResources& pRes, nllMat44 const projM, nllMat44 const viewM, const float frustumPlanes[6][4], nllVec3 const camPos, bool isDebugView) {

    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadMatrixf(viewM);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadMatrixf(projM);

    static float hack_date = 0.f;
    hack_date += 1.f/60.f;

    glColor4f(1.f, 1.f, 1.f, 1.f);
    glBegin(GL_LINES);
    for (int x=-1; x<=1; x++) {
        glVertex3f((float)x, -1, 0.01f);
        glVertex3f((float)x, 1, 0.01f);
    }
    for (int y=-1; y<=1; y++) {
        glVertex3f(-1, (float)y, 0.01f);
        glVertex3f( 1, (float)y, 0.01f);
    }
    glEnd();
    
    glColor4f(.3f, .3f, .3f, 1.f);
    glBegin(GL_LINES);
    for (int x=-10; x<=10; x++) {
        glVertex3f((float)x, -10, 0);
        glVertex3f((float)x, 10, 0);
    }
    for (int y=-10; y<=10; y++) {
        glVertex3f(-10, (float)y, 0);
        glVertex3f( 10, (float)y, 0);
    }
    glEnd();

    //--

    //glDepthMask(GL_FALSE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    //--

    Viewport viewport;
    memcpy(viewport.camera.position.coord, camPos, 3*sizeof(float));
    memcpy(viewport.camera.frustumPlanes, frustumPlanes, 6*4*sizeof(float));

	static unsigned frameId = 0;
	unsigned const SWAPCHAIN_LENGTH = 3;
	frameId = (frameId + 1) % SWAPCHAIN_LENGTH;

    frame(pRes, viewport, frameId, isDebugView);

    //--

    //glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glDisable(GL_DEPTH_TEST);
}

//--------------------------------------------------------
//--------------------------------------------------------

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error %d: %s\n", error, description);
}

int main(int, char**)
{
    // Setup window
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
        return 1;
    GLFWwindow* window = glfwCreateWindow(1280, 720, "ImGui OpenGL2 example", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup ImGui binding
    ImGui_ImplGlfwGL2_Init(window, true);

    // Setup style
    ImGui::StyleColorsClassic();
    //ImGui::StyleColorsDark();

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them. 
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple. 
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'extra_fonts/README.txt' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //ImGuiIO& io = ImGui::GetIO();
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../extra_fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../extra_fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../extra_fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../extra_fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

    ImVec4 clear_color = ImVec4(0.045f, 0.055f, 0.060f, 1.00f);

    nllCameraNavFPS_state nState = {};
    nState.camera = buildInitialCamera();

	//-- Permanent Resources
#ifdef _USE_FAKE_OCTREE_
	ScanFake scans[] = { ScanFake(Position(-6.f, 2.5f, 0.f), 2.f, Color(1.f, 0, 0)),
						 ScanFake(Position(0.f, 2.5f, 0.f), 5.f, Color(0, 1.f, 0)),
						 ScanFake(Position(6.f, 0.f, 0.f), 5.f, Color(0, .5f, 1.f)) };

	IScan *iscans[] = { &scans[0], &scans[1], &scans[2] };
	AllScans allScans{ 3, iscans };
#else

#if 1

	std::vector<std::string> scanPaths;
	std::vector<IScan*> scans;			// FIXME(nico) memory leak

	std::ifstream listing_file("listing.txt");
	std::string line;
	while (std::getline(listing_file, line)) {

		scanPaths.push_back(line);
		scans.push_back(new ScanReal(line));
	}
	AllScans allScans{ (int)scans.size(), scans.data() };

#else
	std::string scan001("D:\\dev_v2\\work\\taglabs\\data_e57\\Scan_003.tlb");
	ScanReal scans[] = { ScanReal(scan001) };
	IScan *iscans[] = { &scans[0] };
	AllScans allScans{ 1, iscans };
#endif

#endif

	AllNodesInFlight allNodesInFlight;
	NodeCache nodeCache;
	CPU cpu(allScans);
	GPU gpu;

	PermanentResources pRes = { allScans, nodeCache, allNodesInFlight, cpu, gpu };
	//--


    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();
        ImGui_ImplGlfwGL2_NewFrame();

        // 1. Show a simple window.
        // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets automatically appears in a window called "Debug".
        {
            static float f = 0.0f;
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        }

        // Rendering

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        static float test = 0;
        //test += 1.f / 60.f;

        // NOTE world space is left-handed, Y pointing 'up', XZ being the 'ground' plane
        // camera

        nllCamera camera = nState.camera;
        float projM[16];
        float viewM[16];
        float const znear = camera.znear;
        float const aspect_ratio = ((float)display_w) / display_h;

        nllMat44Frustum(projM, -znear*aspect_ratio, znear*aspect_ratio, -znear, znear, znear, 100e3f);
        nllMat44EyeFromWorld(viewM, camera.pos, camera.x, camera.y, camera.z);

        ImGui_Vec3("camPos", camera.pos);
        ImGui_Vec3("X", camera.x);
        ImGui_Vec3("Y", camera.y);
        ImGui_Vec3("Z", camera.z);

        // navigation

        nllCameraNavFPS_settings nSettings = {};
        //nSettings.maxSpeed = 2.f + 2.f*logf(fabsf(camera.pos[1]));
		nSettings.maxSpeed = 2.f;
        if (nSettings.maxSpeed < 1.f) nSettings.maxSpeed = 1.f;
        nSettings.maxRotationSpeed = 1.f;
        nSettings.minPitchAngleRadians = -80.f * PI_ / 180.f;
        nSettings.maxPitchAngleRadians = +70.f * PI_ / 180.f;
		nSettings.maxSpeed *= 10.f;
		nSettings.maxRotationSpeed *= 2.f;

        if (ImGui::IsKeyDown('R')) {
            nState = {};
            nState.camera = buildInitialCamera();
        }
        
        nllCameraNavFPS_input nInput = {};
        if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_RightArrow))) nInput.rotationFactors[0] -= 1.f;
        if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_LeftArrow)))  nInput.rotationFactors[0] += 1.f;
        if (ImGui::IsKeyDown('Q')) nInput.moveFactors[0] -= 1.f;
        if (ImGui::IsKeyDown('D')) nInput.moveFactors[0] += 1.f;
        if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_PageUp)))   nInput.moveFactors[1] += 1.f;
        if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_PageDown))) nInput.moveFactors[1] -= 1.f;
        if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_UpArrow)))   nInput.moveFactors[2] += 1.f;
        if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_DownArrow))) nInput.moveFactors[2] -= 1.f;
		
		ImGuiIO& io = ImGui::GetIO();
		const float mx = 2.f * io.MousePos.x / display_w - 1.f;
		const float my = -2.f * io.MousePos.y / display_h + 1.f;
		nInput.mousePos[0] = mx * aspect_ratio;
        nInput.mousePos[1] = my;
        nInput.mousePressed = io.MouseDown[0];
        
        float dt = 1.f / 60.f;
        nState = nllCameraNavFPS_apply(&nSettings, &nInput, &nState, dt);       // FIXME(nico) should be before projM & viewM calc
        
        // draw
    
        float frustumPlanes[6][4];
        {
            nllMat44 clipFromWorldM;
            nllMat44Mul(clipFromWorldM, projM, viewM);

            nllFrustumPlanesFromMat44(frustumPlanes, clipFromWorldM);
        }

        drawWorld(pRes, projM, viewM, frustumPlanes, camera.pos, false);

#if 0		// FIXME(nico) control with imgui

        // top

        int topw=display_w/3, toph=display_h/3;
        int topx=display_w-topw, topy=0;
        {
            glViewport(topx, topy, topw, toph);
            glEnable(GL_SCISSOR_TEST);
            glScissor(topx, topy, topw, toph);

            glClearColor(0,0,0,0);
            glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

            const nllVec3 orthoPos = {0,0,0};
            const nllVec3 orthoX = {1,0,0};
            const nllVec3 orthoY = {0,0,1};
            const nllVec3 orthoZ = {0,-1,0};
            nllMat44 orthoM, orthoVM;
            nllMat44Ortho(orthoM, -11*aspect_ratio, 11*aspect_ratio, -11, 11, -100, 100);
            nllMat44EyeFromWorld(orthoVM, orthoPos, orthoX, orthoY, orthoZ);
            
            drawWorld(pRes, orthoM, orthoVM, frustumPlanes, camera.pos, true);
            
            glDisable(GL_SCISSOR_TEST);
        }

        // 
        
        topx-=topw;
        {
            glViewport(topx, topy, topw, toph);
            glEnable(GL_SCISSOR_TEST);
            glScissor(topx, topy, topw, toph);

            glClearColor(0,0,0,0);
            glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

            const nllVec3 orthoPos = {0,0,0};
            const nllVec3 orthoX = {0,0,-1};
            const nllVec3 orthoY = {0,1,0};
            const nllVec3 orthoZ = {1,0,0};
            nllMat44 orthoM, orthoVM;
            nllMat44Ortho(orthoM, -11*aspect_ratio, 11*aspect_ratio, -11, 11, -100, 100);
            nllMat44EyeFromWorld(orthoVM, orthoPos, orthoX, orthoY, orthoZ);
            
            drawWorld(pRes, orthoM, orthoVM, frustumPlanes, camera.pos, true);
            
            glDisable(GL_SCISSOR_TEST);
        }

		//

#endif

        glViewport(0, 0, display_w, display_h);

        //glUseProgram(0); // You may want this if using this code in an OpenGL 3+ context where shaders may be bound, but prefer using the GL3+ code.
        ImGui::Render();
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplGlfwGL2_Shutdown();
    glfwTerminate();

    return 0;
}
