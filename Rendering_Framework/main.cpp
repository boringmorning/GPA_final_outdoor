#include "src\basic\SceneRenderer.h"
#include <GLFW\glfw3.h>
#include "SceneSetting.h"
#include "objLoader.h"
#include "src\imgui\imgui.h"
#include "src\imgui\imgui_impl_glfw.h"
#include "src\imgui\imgui_impl_opengl3.h"
#include <glm/glm.hpp>

#pragma comment (lib, "lib-vc2015\\glfw3.lib")
#pragma comment (lib, "assimp\\assimp-vc140-mtd.lib")

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

#define SHADOW_MAP_SIZE 7000

using namespace glm;
using namespace std;

// constant
const float shadow_range = 500.0f;
const float light_dist = 500.0f;
const vec3 water_pos = vec3(512.0, 107.0, 512.0);
const mat4 light_proj_matrix = ortho(-shadow_range, shadow_range, -shadow_range, shadow_range, 0.1f, light_dist * 2.f);
const mat4 scale_bias_matrix = translate(mat4(1.0f), vec3(0.5f, 0.5f, 0.5f)) * scale(mat4(1.0f), vec3(0.5f, 0.5f, 0.5f));
const vec3 light_dir = normalize(vec3(0.2, 0.6, 0.5));
const mat4 house_rotate_matrix[2] = {
	rotate(mat4(1.0f), radians(60.0f), vec3(0, 1, 0)),
	rotate(mat4(1.0f), radians(15.0f), vec3(0, 1, 0))
};
const mat4 house_model_matrix[2] = {
	translate(mat4(1.0f), vec3(631.0, 130.0, 468.0)) * house_rotate_matrix[0],
	translate(mat4(1.0f), vec3(656.0, 135.0, 483.0)) * house_rotate_matrix[1]
};
const mat3 tree_rotate_matrix = mat3(rotate(mat4(1.0f), radians(90.0f), vec3(1, 0, 0)));
const vec3 point_light_pos = vec3(636.48, 134.79, 495.98);

// variables
mat4 light_view_matrix;
mat4 light_vp_matrix;

mat4 airplane_light_mvp_matrix;
mat4 airplane_model_matrix;
mat4 airplane_mvp_matrix;
mat4 airplane_mv_matrix;
mat4 airplane_shadow_matrix;

mat4 vp_matrix;

mat4 shadow_sbpv_matrix;

mat4 house_light_mvp_matrix[2];
mat4 house_mvp_matrix[2];
mat4 house_mv_matrix[2];
mat4 house_shadow_matrix[2];

mat4 plant_shadow_matrix;
mat4 water_shadow_matrix;

vec3 view_point_light_pos;

// reflection
mat4 view_matrix2;

mat4 airplane_mvp_matrix2;
mat4 airplane_mv_matrix2;

mat4 vp_matrix2;

mat4 house_mvp_matrix2[2];
mat4 house_mv_matrix2[2];
mat4 house_shadow_matrix2[2];

vec3 view_point_light_pos2;


struct Model
{
	GLuint vao;			// vertex array object
	GLuint vbo;			// vertex buffer object
	GLuint vao2;
	GLuint vbo2;
	int vertexCount = 0;
	int vertexCount2 = 0;
	GLuint tex;
	GLuint tex2;
	GLuint normal_map;
	GLuint offset_vbo;
};

Depth depth;
Model airplane_model, house_model, water_model, baum_hd_pine_trunk, baum_hd_pine_leaves, baum_hd_trunk, baum_hd_leaves, grass0, grass2;

struct
{
	GLuint vao;
	GLuint vbo;
	GLuint ebo;
	int vertexCount = 0;
}sphere_model;

struct
{
	GLuint fbo;
	GLuint rbo;
	const int texCount = 3;
	GLuint tex[3];
}bloom;

struct
{
	GLuint fbo;
	GLuint rbo;
	GLuint tex;
}reflection;

struct
{
	GLuint fbo;
	GLuint rbo;
	GLuint tex;
}refraction;


int FRAME_WIDTH = 1024;
int FRAME_HEIGHT = 768;

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void mouseScrollCallback(GLFWwindow *window, double xoffset, double yoffset);
void cursorPosCallback(GLFWwindow* window, double x, double y);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

void initializeGL();
void resizeGL(GLFWwindow *window, int w, int h);
void paintGL();
void updateState();
void adjustCameraPositionWithTerrain();
void updateAirplane(const glm::mat4 &viewMatrix);
void initScene();
void initPlants();
void initLoadTree(const char *filename, Model &model, int plantNumber);
void initAirplane();
void initHouse();
void initWater();
void initDepth();
void initSphere();
void initGuassian();
void initView();
void renderDepth();
void renderWaterDepth();
void showDepthMap();
void renderPlants();
void renderAirplane();
void renderHouse();
void renderWater();
void renderSphere();
void renderView();

bool m_leftButtonPressed = false;
bool m_rightButtonPressed = false;
double cursorPos[2];

SceneRenderer *m_renderer = nullptr;
glm::vec3 m_lookAtCenter;
glm::vec3 m_eye;

// chen 
float camera_rotation = 0.0;
Shader *depth_prog = nullptr;
Shader *depth_prog2 = nullptr;
Shader *view_prog = nullptr;
Shader *airplane_prog = nullptr;
Shader *house_prog = nullptr;
Shader *water_prog = nullptr;
Shader *reflect_prog = nullptr;
Shader *refract_prog = nullptr;
Shader *sphere_prog = nullptr;
Shader *gaussian_prog = nullptr;
Shader *plants_prog = nullptr;

glm::mat4 view_matrix;
glm::mat4 proj_matrix;

// UI variables
bool stop = false;
bool activateBloom = false;
bool activatePlant = false;
bool activateWater = true;

enum Mode {
	ALL,
	WORLD_SPACE_VERTEX,
	WORLD_SPACE_NORMAL,
	AMBIENT_COLOR_MAP,
	DIFFUSE_COLOR_MAP,
	SPECULAR_COLOR_MAP
};
int mode = 0;
///////////////////////////


// lee
bool enable_normal_mapping = false;
bool enable_cel_shading = false;
///////////////////////////

struct {
	struct {
		GLuint light_mvp_matrix;
	}depth;

	struct {
		GLuint light_vp_matrix;
		GLuint isLeaves;
	}depth2;

	struct {
		GLuint vp_matrix;
		GLuint view_matrix;
		GLuint view_point_light_pos;
		GLuint shadow_matrix;
		GLuint tex;
		GLuint shadow_tex;
		GLuint mode;
		GLuint enable_cel_shading;
		GLuint isLeaves;
		GLuint plane;
	}plants;

	struct {
		GLuint m_matrix;
		GLuint mvp_matrix;
		GLuint mv_matrix;
		GLuint view_matrix;
		GLuint view_point_light_pos;
		GLuint shadow_matrix;
		GLuint tex;
		GLuint shadow_tex;
		GLuint mode;
		GLuint enable_cel_shading;
		GLuint plane;
	}airplane;

	struct {
		GLuint m_matrix;
		GLuint mvp_matrix;
		GLuint mv_matrix;
		GLuint view_matrix;
		GLuint view_point_light_pos;
		GLuint shadow_matrix;
		GLuint tex;
		GLuint shadow_tex;
		GLuint mode;
		GLuint normal_map;
		GLuint enable_normal_mapping;
		GLuint enable_cel_shading;
		GLuint plane;
	}house;

	struct {
		GLuint vp_matrix;
		GLuint mode;
		GLuint plane;
	}sphere;

	struct {
		GLuint ishorizontal;
	}gaussian;

	struct {
		GLuint activateBloom;
		GLuint tex;
		GLuint blurTex;
	}view;

	struct {
		GLuint mode;
		GLuint vp_matrix;
		GLuint reflectionTex;
		GLuint refractionTex;
		GLuint shadow_matrix;
		GLuint shadow_tex;
		GLuint camera_position;
	}water;

}uniforms;

void vsyncEnabled(GLFWwindow *window);
void vsyncDisabled(GLFWwindow *window);

PlantManager *m_plantManager = nullptr;
Terrain *m_terrain = nullptr;

// the airplane's transformation has been handled
glm::vec3 m_airplanePosition;
glm::mat4 m_airplaneRotMat;

int main(){
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow *window = glfwCreateWindow(FRAME_WIDTH, FRAME_HEIGHT, "rendering", nullptr, nullptr);

	if (window == nullptr){
		std::cout << "failed to create GLFW window\n";
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// load OpenGL function pointer
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	

	glfwSetKeyCallback(window, keyCallback);
	glfwSetScrollCallback(window, mouseScrollCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetCursorPosCallback(window, cursorPosCallback);
	glfwSetFramebufferSizeCallback(window, resizeGL);

	initializeGL();

	// be careful v-sync issue
	glfwSwapInterval(0);

	// Setup Dear ImGui context
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer backends
	const char* glsl_version = "#version 130";
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	vsyncDisabled(window);

	glfwTerminate();
	return 0;
}
void vsyncDisabled(GLFWwindow *window){
	float periodForEvent = 1.0 / 60.0;
	float accumulatedEventPeriod = 0.0;
	double previousTime = glfwGetTime();
	double previousTimeForFPS = glfwGetTime();
	int frameCount = 0;
	while (!glfwWindowShouldClose(window)){
		// Poll and handle events (inputs, window resize, etc.)
		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		glfwPollEvents();

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// ImGui interface
		{
			static float f = 0.0f;
			static int counter = 0;

			ImGui::Begin("Menu");                          // Create a window called "Hello, world!" and append into it.

			ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
			ImGui::InputFloat3("eye", (float*)&m_eye);
			ImGui::InputFloat3("lookAt center", (float*)&m_lookAtCenter);
			ImGui::SliderFloat("camera rotation", &camera_rotation, 0.0f, 360.0f);
			ImGui::Checkbox("Stop motion", &stop);
			ImGui::Text("Effect activate:");
			ImGui::Checkbox("Normal map", &enable_normal_mapping); ImGui::SameLine();
			ImGui::Checkbox("Bloom effect", &activateBloom); ImGui::SameLine();
			ImGui::Checkbox("Cel shading", &enable_cel_shading);
			ImGui::Checkbox("Plants", &activatePlant); ImGui::SameLine();
			ImGui::Checkbox("Water", &activateWater);
			ImGui::Text("Mode selection:");
			ImGui::SameLine();
			const char *items[] = { "all", "world space vertex", "world space normal", "ambient color map", "diffuse color map", "specular color map" };
			ImGui::SetNextItemWidth(100);
			ImGui::Combo(" ", &mode, items, IM_ARRAYSIZE(items));
			
			ImGui::End();
		}

		// measure speed
		double currentTime = glfwGetTime();
		float deltaTime = currentTime - previousTime;
		frameCount = frameCount + 1;
		if (currentTime - previousTimeForFPS >= 1.0){
			std::cout << "\rFPS: " << frameCount;
			frameCount = 0;
			previousTimeForFPS = currentTime;
		}
		previousTime = currentTime;

		// game loop
		accumulatedEventPeriod = accumulatedEventPeriod + deltaTime;
		if (accumulatedEventPeriod > periodForEvent){
			updateState();
			accumulatedEventPeriod = accumulatedEventPeriod - periodForEvent;
		}

		paintGL();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}
void vsyncEnabled(GLFWwindow *window){
	double previousTimeForFPS = glfwGetTime();
	int frameCount = 0;
	while (!glfwWindowShouldClose(window)){
		// measure speed
		double currentTime = glfwGetTime();
		frameCount = frameCount + 1;
		if (currentTime - previousTimeForFPS >= 1.0){
			std::cout << "\rFPS: " << frameCount;
			frameCount = 0;
			previousTimeForFPS = currentTime;
		}
		
		updateState();
		paintGL();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

void initializeGL(){
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	m_renderer = new SceneRenderer();
	m_renderer->initialize(FRAME_WIDTH, FRAME_HEIGHT);	

	m_eye = glm::vec3(531.0, 10.0, 300.0);
	m_lookAtCenter = glm::vec3(531.0, 0.0, 320.0);
	
	initScene();
	initPlants();
	initAirplane();
	initHouse();
	initWater();
	initDepth();
	initSphere();
	initGuassian();
	initView();

	mat4 rotation_matrix = rotate(mat4(1.0f), radians(camera_rotation), vec3(0, 0, 1));
	vec3 up = rotation_matrix * vec4(0.0, 1.0, 0.0, 1.0);
	view_matrix = lookAt(m_eye, m_lookAtCenter, up);
	proj_matrix = perspective(glm::radians(60.0f), FRAME_WIDTH * 1.0f / FRAME_HEIGHT, 0.1f, 1000.0f);
	m_renderer->setProjection(proj_matrix);

	// bloom effect object
	glGenFramebuffers(1, &bloom.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, bloom.fbo);
	glDeleteRenderbuffers(1, &bloom.rbo);
	glGenRenderbuffers(1, &bloom.rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, bloom.rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, FRAME_WIDTH, FRAME_HEIGHT);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, bloom.rbo);

	for (int i = 0; i < bloom.texCount; i++) {
		glDeleteTextures(1, &bloom.tex[i]);
		glGenTextures(1, &bloom.tex[i]);
		glBindTexture(GL_TEXTURE_2D, bloom.tex[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, FRAME_WIDTH, FRAME_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, bloom.tex[i], 0);
	}

	// reflection and refraction
	glGenFramebuffers(1, &reflection.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, reflection.fbo);
	glDeleteRenderbuffers(1, &reflection.rbo);
	glGenRenderbuffers(1, &reflection.rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, reflection.rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, FRAME_WIDTH, FRAME_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, reflection.rbo);
	glDeleteTextures(1, &reflection.tex);
	glGenTextures(1, &reflection.tex);
	glBindTexture(GL_TEXTURE_2D, reflection.tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, FRAME_WIDTH, FRAME_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, reflection.tex, 0);

	glGenFramebuffers(1, &refraction.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, refraction.fbo);
	glDeleteRenderbuffers(1, &refraction.rbo);
	glGenRenderbuffers(1, &refraction.rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, refraction.rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, FRAME_WIDTH, FRAME_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, refraction.rbo);
	glDeleteTextures(1, &refraction.tex);
	glGenTextures(1, &refraction.tex);
	glBindTexture(GL_TEXTURE_2D, refraction.tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, FRAME_WIDTH, FRAME_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, refraction.tex, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void resizeGL(GLFWwindow *window, int w, int h){
	FRAME_WIDTH = w;
	FRAME_HEIGHT = h;
	m_renderer->resize(w, h);
	proj_matrix = glm::perspective(glm::radians(60.0f), w * 1.0f / h, 0.1f, 1000.0f);
	m_renderer->setProjection(proj_matrix);

	// update proj_matrix related variables
	airplane_mvp_matrix = proj_matrix * airplane_mv_matrix;
	house_mvp_matrix[0] = proj_matrix * house_mv_matrix[0];
	house_mvp_matrix[1] = proj_matrix * house_mv_matrix[1];
	vp_matrix = proj_matrix * view_matrix;

	// refletion
	airplane_mvp_matrix2 = proj_matrix * airplane_mv_matrix2;
	vp_matrix2 = proj_matrix * view_matrix2;

	for (int i = 0; i < 2; i++) {
		house_mvp_matrix2[i] = proj_matrix * house_mv_matrix2[i];
	}

	// bloom effect object
	glBindFramebuffer(GL_FRAMEBUFFER, bloom.fbo);
	glDeleteRenderbuffers(1, &bloom.rbo);
	glGenRenderbuffers(1, &bloom.rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, bloom.rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, FRAME_WIDTH, FRAME_HEIGHT);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, bloom.rbo);
	
	for (int i = 0; i < bloom.texCount; i++) {
		glDeleteTextures(1, &bloom.tex[i]);
		glGenTextures(1, &bloom.tex[i]);
		glBindTexture(GL_TEXTURE_2D, bloom.tex[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, FRAME_WIDTH, FRAME_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, bloom.tex[i], 0);
	}
	
	// reflection and refraction
	glBindFramebuffer(GL_FRAMEBUFFER, reflection.fbo);
	glDeleteRenderbuffers(1, &reflection.rbo);
	glGenRenderbuffers(1, &reflection.rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, reflection.rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, FRAME_WIDTH, FRAME_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, reflection.rbo);
	glDeleteTextures(1, &reflection.tex);
	glGenTextures(1, &reflection.tex);
	glBindTexture(GL_TEXTURE_2D, reflection.tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, FRAME_WIDTH, FRAME_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, reflection.tex, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, refraction.fbo);
	glDeleteRenderbuffers(1, &refraction.rbo);
	glGenRenderbuffers(1, &refraction.rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, refraction.rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, FRAME_WIDTH, FRAME_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, refraction.rbo);
	glDeleteTextures(1, &refraction.tex);
	glGenTextures(1, &refraction.tex);
	glBindTexture(GL_TEXTURE_2D, refraction.tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, FRAME_WIDTH, FRAME_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, refraction.tex, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
}


void updateState(){	
	if (!stop) {
		m_lookAtCenter.z = m_lookAtCenter.z + 1;
		m_eye.z = m_eye.z + 1;
	}
	
	// adjust camera position with terrain
	adjustCameraPositionWithTerrain();
	// calculate camera 
	mat4 rotation_matrix = rotate(mat4(1.0f), radians(camera_rotation), vec3(0, 0, 1));
	vec3 up = rotation_matrix * vec4(0.0, 1.0, 0.0, 1.0);
	glm::mat4 vm = glm::lookAt(m_eye, m_lookAtCenter, up); 

	// [IMPORTANT] set camera information to renderer
	m_renderer->setView(vm, m_eye);
	// chen 1/3
	view_matrix = vm;
	for (int i = 0; i < 4; i++) {
		depth.chunkRotMat[i][3] = vec4(m_eye, 1.0);
	}

	// update airplane
	updateAirplane(vm);

	// update global matrix
	light_view_matrix = lookAt(light_dir * light_dist + m_lookAtCenter, m_lookAtCenter, vec3(0.0f, -1.0f, 0.0f));
	light_vp_matrix = light_proj_matrix * light_view_matrix;
	shadow_sbpv_matrix = scale_bias_matrix * light_vp_matrix;

	airplane_model_matrix = translate(mat4(1.0f), m_airplanePosition) * m_airplaneRotMat;
	airplane_mv_matrix = view_matrix * airplane_model_matrix;
	airplane_mvp_matrix = proj_matrix * airplane_mv_matrix;
	airplane_light_mvp_matrix = light_vp_matrix * airplane_model_matrix;
	airplane_shadow_matrix = shadow_sbpv_matrix * airplane_model_matrix;

	vp_matrix = proj_matrix * view_matrix;

	view_point_light_pos = vec3(view_matrix * vec4(point_light_pos, 1.0));

	plant_shadow_matrix = shadow_sbpv_matrix;
	water_shadow_matrix = shadow_sbpv_matrix;


	for (int i = 0; i < 2; i++) {
		house_mv_matrix[i] = view_matrix * house_model_matrix[i];
		house_mvp_matrix[i] = proj_matrix * house_mv_matrix[i];
		house_light_mvp_matrix[i] = light_vp_matrix * house_model_matrix[i];
		house_shadow_matrix[i] = shadow_sbpv_matrix * house_model_matrix[i];
	}

	// refletion
	vec3 m_eye2 = vec3(m_eye.x, m_eye.y - (m_eye.y - water_pos.y) * 2, m_eye.z);
	vec3 m_lookAtCenter2 = vec3(m_lookAtCenter.x, m_lookAtCenter.y - (m_lookAtCenter.y - water_pos.y) * 2, m_lookAtCenter.z);
	view_matrix2 = lookAt(m_eye2, m_lookAtCenter2, vec3(0.0, 1.0, 0.0));

	airplane_mv_matrix2 = view_matrix2 * airplane_model_matrix;
	airplane_mvp_matrix2 = proj_matrix * airplane_mv_matrix2;

	vp_matrix2 = proj_matrix * view_matrix2;

	view_point_light_pos2 = vec3(view_matrix2 * vec4(point_light_pos, 1.0));

	for (int i = 0; i < 2; i++) {
		house_mv_matrix2[i] = view_matrix2 * house_model_matrix[i];
		house_mvp_matrix2[i] = proj_matrix * house_mv_matrix2[i];
	}
}
void paintGL(){
	
	renderDepth();	
	glBindFramebuffer(GL_FRAMEBUFFER, bloom.fbo);
	m_renderer->renderPass(view_point_light_pos, shadow_sbpv_matrix, depth.tex, mode, enable_cel_shading, reflection.fbo, refraction.fbo, 
		view_point_light_pos2, view_matrix2, activateWater); // render terrain

	renderWaterDepth();
	
	renderAirplane();
	renderHouse();
	renderSphere();
	if (activatePlant)
		renderPlants();
	if (activateWater)
		renderWater();
	renderView();
}

void renderDepth() {
	glBindFramebuffer(GL_FRAMEBUFFER, depth.fbo);

	glEnable(GL_DEPTH_TEST);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(4.0f, 4.0f);

	if (activatePlant) {
		// leaves
		depth_prog2->useShader();
		glUniform1i(uniforms.depth2.isLeaves, true);
		glUniformMatrix4fv(uniforms.depth2.light_vp_matrix, 1, false, value_ptr(light_vp_matrix));

		glBindVertexArray(baum_hd_pine_leaves.vao);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, baum_hd_pine_leaves.tex);
		glDrawArraysInstanced(GL_TRIANGLES, 0, baum_hd_pine_leaves.vertexCount, m_plantManager->m_numPlantInstance[0]);

		glBindVertexArray(baum_hd_leaves.vao);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, baum_hd_leaves.tex);
		glDrawArraysInstanced(GL_TRIANGLES, 0, baum_hd_leaves.vertexCount, m_plantManager->m_numPlantInstance[1]);

		// trunk
		glUniform1i(uniforms.depth2.isLeaves, false);

		glBindVertexArray(baum_hd_pine_trunk.vao);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, baum_hd_pine_trunk.tex);
		glDrawArraysInstanced(GL_TRIANGLES, 0, baum_hd_pine_trunk.vertexCount, m_plantManager->m_numPlantInstance[0]);

		glBindVertexArray(baum_hd_trunk.vao);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, baum_hd_trunk.tex);
		glDrawArraysInstanced(GL_TRIANGLES, 0, baum_hd_trunk.vertexCount, m_plantManager->m_numPlantInstance[1]);
	}

	// airplane
	depth_prog->useShader();
	glBindVertexArray(airplane_model.vao);
	glUniformMatrix4fv(uniforms.depth.light_mvp_matrix, 1, false, value_ptr(airplane_light_mvp_matrix));
	glDrawArrays(GL_TRIANGLES, 0, airplane_model.vertexCount);

	// house
	glBindVertexArray(house_model.vao);
	for (int i = 0; i < 2; i++) {
		glUniformMatrix4fv(uniforms.depth.light_mvp_matrix, 1, false, value_ptr(house_light_mvp_matrix[i]));
		glDrawArrays(GL_TRIANGLES, 0, house_model.vertexCount);
	}

	depth_prog->disableShader();
	glViewport(0, 0, FRAME_WIDTH, FRAME_HEIGHT);
	glDisable(GL_POLYGON_OFFSET_FILL);
}

void renderWaterDepth() {
	// water
	glBindFramebuffer(GL_FRAMEBUFFER, depth.fbo);

	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(4.0f, 4.0f);

	glBindVertexArray(water_model.vao);
	glUniformMatrix4fv(uniforms.depth.light_mvp_matrix, 1, false, value_ptr(light_vp_matrix));
	glDrawArrays(GL_TRIANGLES, 0, water_model.vertexCount);

	depth_prog->disableShader();
	glViewport(0, 0, FRAME_WIDTH, FRAME_HEIGHT);
	glDisable(GL_POLYGON_OFFSET_FILL);
}

void showDepthMap() {
	view_prog->useShader();
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, depth.tex);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	view_prog->disableShader();
}
void renderPlants(){
	plants_prog->useShader();
	glUniformMatrix4fv(uniforms.plants.vp_matrix, 1, GL_FALSE, value_ptr(vp_matrix));
	glUniformMatrix4fv(uniforms.plants.view_matrix, 1, GL_FALSE, value_ptr(view_matrix));
	glUniformMatrix4fv(uniforms.plants.shadow_matrix, 1, GL_FALSE, value_ptr(plant_shadow_matrix));
	glUniform3fv(uniforms.plants.view_point_light_pos, 1, value_ptr(view_point_light_pos));
	glUniform1i(uniforms.plants.mode, mode);
	glUniform1i(uniforms.plants.enable_cel_shading, enable_cel_shading);
	glBindFramebuffer(GL_FRAMEBUFFER, bloom.fbo);
	glViewport(0, 0, FRAME_WIDTH, FRAME_HEIGHT);

	// bind shadow tex
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depth.tex);

	// Model baum_hd_pine_trunk, baum_hd_pine_leaves, baum_hd_trunk, baum_hd_leaves;
	glUniform1i(uniforms.plants.isLeaves, 0);

	glBindVertexArray(baum_hd_pine_trunk.vao);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, baum_hd_pine_trunk.tex);
	glDrawArraysInstanced(GL_TRIANGLES, 0, baum_hd_pine_trunk.vertexCount, m_plantManager->m_numPlantInstance[0]);

	glBindVertexArray(baum_hd_trunk.vao);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, baum_hd_trunk.tex);
	glDrawArraysInstanced(GL_TRIANGLES, 0, baum_hd_trunk.vertexCount, m_plantManager->m_numPlantInstance[1]);
	
	glBindVertexArray(baum_hd_pine_leaves.vao);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, baum_hd_pine_leaves.tex);
	glDrawArraysInstanced(GL_TRIANGLES, 0, baum_hd_pine_leaves.vertexCount, m_plantManager->m_numPlantInstance[0]);

	
	glBindVertexArray(baum_hd_leaves.vao);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, baum_hd_leaves.tex);
	glDrawArraysInstanced(GL_TRIANGLES, 0, baum_hd_leaves.vertexCount, m_plantManager->m_numPlantInstance[1]);
	
	// grass
	glUniform1i(uniforms.plants.isLeaves, 1);

	glBindVertexArray(grass0.vao);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, grass0.tex);
	glDrawArraysInstanced(GL_TRIANGLES, 0, grass0.vertexCount, m_plantManager->m_numPlantInstance[2]);

	// grass2: 2 texture
	glBindVertexArray(grass2.vao);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, grass2.tex);
	glDrawArraysInstanced(GL_TRIANGLES, 0, grass2.vertexCount, m_plantManager->m_numPlantInstance[3]);
	glBindVertexArray(grass2.vao2);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, grass2.tex2);
	glDrawArraysInstanced(GL_TRIANGLES, 0, grass2.vertexCount2, m_plantManager->m_numPlantInstance[3]);

	if (activateWater) {
		// reflection
		glEnable(GL_CLIP_DISTANCE0);
		glBindFramebuffer(GL_FRAMEBUFFER, reflection.fbo);
		glUniformMatrix4fv(uniforms.plants.vp_matrix, 1, GL_FALSE, value_ptr(vp_matrix2));
		glUniformMatrix4fv(uniforms.plants.view_matrix, 1, GL_FALSE, value_ptr(view_matrix2));
		glUniform3fv(uniforms.plants.view_point_light_pos, 1, value_ptr(view_point_light_pos2));
		glUniform4fv(uniforms.plants.plane, 1, value_ptr(vec4(0.0, 1.0, 0.0, -107.0)));

		// Model baum_hd_pine_trunk, baum_hd_pine_leaves, baum_hd_trunk, baum_hd_leaves;
		glUniform1i(uniforms.plants.isLeaves, 0);

		glBindVertexArray(baum_hd_pine_trunk.vao);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, baum_hd_pine_trunk.tex);
		glDrawArraysInstanced(GL_TRIANGLES, 0, baum_hd_pine_trunk.vertexCount, m_plantManager->m_numPlantInstance[0]);

		glBindVertexArray(baum_hd_trunk.vao);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, baum_hd_trunk.tex);
		glDrawArraysInstanced(GL_TRIANGLES, 0, baum_hd_trunk.vertexCount, m_plantManager->m_numPlantInstance[1]);

		glBindVertexArray(baum_hd_pine_leaves.vao);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, baum_hd_pine_leaves.tex);
		glDrawArraysInstanced(GL_TRIANGLES, 0, baum_hd_pine_leaves.vertexCount, m_plantManager->m_numPlantInstance[0]);


		glBindVertexArray(baum_hd_leaves.vao);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, baum_hd_leaves.tex);
		glDrawArraysInstanced(GL_TRIANGLES, 0, baum_hd_leaves.vertexCount, m_plantManager->m_numPlantInstance[1]);

		// grass
		glUniform1i(uniforms.plants.isLeaves, 1);

		glBindVertexArray(grass0.vao);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, grass0.tex);
		glDrawArraysInstanced(GL_TRIANGLES, 0, grass0.vertexCount, m_plantManager->m_numPlantInstance[2]);

		// grass2: 2 texture
		glBindVertexArray(grass2.vao);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, grass2.tex);
		glDrawArraysInstanced(GL_TRIANGLES, 0, grass2.vertexCount, m_plantManager->m_numPlantInstance[3]);
		glBindVertexArray(grass2.vao2);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, grass2.tex2);
		glDrawArraysInstanced(GL_TRIANGLES, 0, grass2.vertexCount2, m_plantManager->m_numPlantInstance[3]);

		glDisable(GL_CLIP_DISTANCE0);
	}

	plants_prog->disableShader();
}


void renderAirplane() {
	airplane_prog->useShader();
	glBindFramebuffer(GL_FRAMEBUFFER, bloom.fbo);
	glViewport(0, 0, FRAME_WIDTH, FRAME_HEIGHT);
	glBindVertexArray(airplane_model.vao);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, airplane_model.tex);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depth.tex);

	glUniformMatrix4fv(uniforms.airplane.m_matrix, 1, GL_FALSE, value_ptr(airplane_model_matrix));
	glUniformMatrix4fv(uniforms.airplane.mvp_matrix, 1, GL_FALSE, value_ptr(airplane_mvp_matrix));
	glUniformMatrix4fv(uniforms.airplane.mv_matrix, 1, GL_FALSE, value_ptr(airplane_mv_matrix));
	glUniformMatrix4fv(uniforms.airplane.view_matrix, 1, GL_FALSE, value_ptr(view_matrix));
	glUniformMatrix4fv(uniforms.airplane.shadow_matrix, 1, GL_FALSE, value_ptr(airplane_shadow_matrix));
	glUniform3fv(uniforms.airplane.view_point_light_pos, 1, value_ptr(view_point_light_pos));
	glUniform1i(uniforms.airplane.mode, mode);
	glUniform1i(uniforms.airplane.enable_cel_shading, enable_cel_shading);
	glDrawArrays(GL_TRIANGLES, 0, airplane_model.vertexCount);
	
	if (activateWater) {
		// refraction
		glEnable(GL_CLIP_DISTANCE0);
		glBindFramebuffer(GL_FRAMEBUFFER, refraction.fbo);
		glUniform4fv(uniforms.airplane.plane, 1, value_ptr(vec4(0.0, -1.0, 0.0, 107.0)));
		glDrawArrays(GL_TRIANGLES, 0, airplane_model.vertexCount);

		// reflection
		glUniform4fv(uniforms.airplane.plane, 1, value_ptr(vec4(0.0, 1.0, 0.0, -107.0)));
		glUniformMatrix4fv(uniforms.airplane.mvp_matrix, 1, GL_FALSE, value_ptr(airplane_mvp_matrix2));
		glUniformMatrix4fv(uniforms.airplane.mv_matrix, 1, GL_FALSE, value_ptr(airplane_mv_matrix2));
		glUniformMatrix4fv(uniforms.airplane.view_matrix, 1, GL_FALSE, value_ptr(view_matrix2));
		glUniform3fv(uniforms.airplane.view_point_light_pos, 1, value_ptr(view_point_light_pos2));
		glBindFramebuffer(GL_FRAMEBUFFER, reflection.fbo);
		glDrawArrays(GL_TRIANGLES, 0, airplane_model.vertexCount);
		glDisable(GL_CLIP_DISTANCE0);
	}

	airplane_prog->disableShader();
}

void renderHouse() {
	house_prog->useShader();
	glBindFramebuffer(GL_FRAMEBUFFER, bloom.fbo);
	glViewport(0, 0, FRAME_WIDTH, FRAME_HEIGHT);
	glBindVertexArray(house_model.vao);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, house_model.tex);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depth.tex);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, house_model.normal_map);
	glUniformMatrix4fv(uniforms.house.view_matrix, 1, GL_FALSE, value_ptr(view_matrix));
	glUniform3fv(uniforms.house.view_point_light_pos, 1, value_ptr(view_point_light_pos));
	glUniform1i(uniforms.house.mode, mode);
	glUniform1i(uniforms.house.enable_normal_mapping, enable_normal_mapping);
	glUniform1i(uniforms.house.enable_cel_shading, enable_cel_shading);

	for (int i = 0; i < 2; i++) {
		glUniformMatrix4fv(uniforms.house.m_matrix, 1, GL_FALSE, value_ptr(house_model_matrix[i]));
		glUniformMatrix4fv(uniforms.house.mvp_matrix, 1, GL_FALSE, value_ptr(house_mvp_matrix[i]));
		glUniformMatrix4fv(uniforms.house.mv_matrix, 1, GL_FALSE, value_ptr(house_mv_matrix[i]));
		glUniformMatrix4fv(uniforms.house.shadow_matrix, 1, GL_FALSE, value_ptr(house_shadow_matrix[i]));
		glDrawArrays(GL_TRIANGLES, 0, house_model.vertexCount);
	}

	if (activateWater) {
		// reflection
		glEnable(GL_CLIP_DISTANCE0);

		glBindFramebuffer(GL_FRAMEBUFFER, reflection.fbo);
		glUniformMatrix4fv(uniforms.house.view_matrix, 1, GL_FALSE, value_ptr(view_matrix2));
		glUniform3fv(uniforms.house.view_point_light_pos, 1, value_ptr(view_point_light_pos2));
		glUniform4fv(uniforms.house.plane, 1, value_ptr(vec4(0.0, 1.0, 0.0, -107.0)));

		for (int i = 0; i < 2; i++) {
			glUniformMatrix4fv(uniforms.house.m_matrix, 1, GL_FALSE, value_ptr(house_model_matrix[i]));
			glUniformMatrix4fv(uniforms.house.mvp_matrix, 1, GL_FALSE, value_ptr(house_mvp_matrix2[i]));
			glUniformMatrix4fv(uniforms.house.mv_matrix, 1, GL_FALSE, value_ptr(house_mv_matrix2[i]));
			glUniformMatrix4fv(uniforms.house.shadow_matrix, 1, GL_FALSE, value_ptr(house_shadow_matrix[i]));
			glDrawArrays(GL_TRIANGLES, 0, house_model.vertexCount);
		}

		glDisable(GL_CLIP_DISTANCE0);
	}

	house_prog->disableShader();
}

void renderWater() {
	water_prog->useShader();
	glBindFramebuffer(GL_FRAMEBUFFER, bloom.fbo);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glViewport(0, 0, FRAME_WIDTH, FRAME_HEIGHT);

	glUniform1i(uniforms.water.mode, mode);
	glUniformMatrix4fv(uniforms.water.vp_matrix, 1, GL_FALSE, value_ptr(vp_matrix));
	glUniformMatrix4fv(uniforms.water.shadow_matrix, 1, GL_FALSE, value_ptr(water_shadow_matrix));
	glUniform3fv(uniforms.water.camera_position, 1, value_ptr(m_eye));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, reflection.tex);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, refraction.tex);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, depth.tex);

	glBindVertexArray(water_model.vao);
	glDrawArrays(GL_TRIANGLES, 0, water_model.vertexCount);

	water_prog->disableShader();
}

void renderSphere() {
	glBindFramebuffer(GL_FRAMEBUFFER, bloom.fbo);
	sphere_prog->useShader();
	glUniformMatrix4fv(uniforms.sphere.vp_matrix, 1, GL_FALSE, value_ptr(vp_matrix));
	glUniform1i(uniforms.sphere.mode, mode);

	// clear bright texture
	const float black[4] = { 0, 0, 0, 1 };
	glClearTexImage(bloom.tex[1], 0, GL_RGBA, GL_FLOAT, black);
	// bind 2 output texture
	unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);

	glViewport(0, 0, FRAME_WIDTH, FRAME_HEIGHT);
	glBindVertexArray(sphere_model.vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphere_model.ebo);
	glDrawElements(GL_TRIANGLES, sphere_model.vertexCount, GL_UNSIGNED_INT, 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	if (activateWater) {
		// reflection
		glEnable(GL_CLIP_DISTANCE0);

		glBindFramebuffer(GL_FRAMEBUFFER, reflection.fbo);
		glUniformMatrix4fv(uniforms.sphere.vp_matrix, 1, GL_FALSE, value_ptr(vp_matrix2));
		glUniform4fv(uniforms.sphere.plane, 1, value_ptr(vec4(0.0, 1.0, 0.0, -107.0)));
		glDrawElements(GL_TRIANGLES, sphere_model.vertexCount, GL_UNSIGNED_INT, 0);

		glDisable(GL_CLIP_DISTANCE0);
	}

	sphere_prog->disableShader();

	// Guassian blur
	if (activateBloom && mode==0) {
		gaussian_prog->useShader();
		glBindFramebuffer(GL_FRAMEBUFFER, bloom.fbo);
		glDisable(GL_DEPTH_TEST);
		bool horizontal = true;
		int amount = 6;

		for (unsigned int i = 0; i < amount; i++)
		{
			glUniform1i(uniforms.gaussian.ishorizontal, horizontal);
			if (i % 2 == 0) {
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, bloom.tex[1]);
				glDrawBuffer(GL_COLOR_ATTACHMENT2);
				glClearTexImage(bloom.tex[2], 0, GL_RGBA, GL_FLOAT, black);
			}
			else {
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, bloom.tex[2]);
				glDrawBuffer(GL_COLOR_ATTACHMENT1);
				glClearTexImage(bloom.tex[1], 0, GL_RGBA, GL_FLOAT, black);
			}

			glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
			horizontal = !horizontal;
		}
		gaussian_prog->disableShader();
		glEnable(GL_DEPTH_TEST);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
	}
}

void renderView() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	view_prog->useShader();
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	glUniform1i(uniforms.view.activateBloom, activateBloom && (mode == 0));
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, bloom.tex[0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, bloom.tex[1]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	view_prog->disableShader();
}

////////////////////////////////////////////////
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods){}
void mouseScrollCallback(GLFWwindow *window, double xoffset, double yoffset) {}
void cursorPosCallback(GLFWwindow* window, double x, double y){
	cursorPos[0] = x;
	cursorPos[1] = y;
}
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods){	
}

void initView() {
	view_prog = new Shader("src\\shader\\view.vs.glsl", "src\\shader\\view.fs.glsl");
	uniforms.view.activateBloom = glGetUniformLocation(view_prog->getProgramID(), "activateBloom");
	uniforms.view.tex = glGetUniformLocation(view_prog->getProgramID(), "tex");
	uniforms.view.blurTex = glGetUniformLocation(view_prog->getProgramID(), "blurTex");

	view_prog->useShader();
	glUniform1i(uniforms.view.tex, 0);
	glUniform1i(uniforms.view.blurTex, 1);
	view_prog->disableShader();

}

void initGuassian() {
	gaussian_prog = new Shader("src\\shader\\gaussian.vs.glsl", "src\\shader\\gaussian.fs.glsl");
	uniforms.gaussian.ishorizontal = glGetUniformLocation(gaussian_prog->getProgramID(), "horizontal");
}

void initSphere() {
	sphere_prog = new Shader("src\\shader\\sphere.vs.glsl", "src\\shader\\sphere.fs.glsl");
	uniforms.sphere.vp_matrix = glGetUniformLocation(sphere_prog->getProgramID(), "vp_matrix");
	uniforms.sphere.mode = glGetUniformLocation(sphere_prog->getProgramID(), "mode");
	uniforms.sphere.plane = glGetUniformLocation(sphere_prog->getProgramID(), "plane");

	const float PI = 3.1415926535;
	const float radius = 2.0f;
	const int sectorCount = 36;
	const int stackCount = 18;
	const vec3 center = vec3(636.48, 134.79, 495.98);

	// clear memory of prev arrays
	vector<float> vertices, normals;

	float x, y, z, xy;                              // vertex position
	float nx, ny, nz, lengthInv = 1.0f / radius;    // vertex normal

	float sectorStep = 2 * PI / sectorCount;
	float stackStep = PI / stackCount;
	float sectorAngle, stackAngle;

	for (int i = 0; i <= stackCount; ++i)
	{
		stackAngle = PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
		xy = radius * cosf(stackAngle);             // r * cos(u)
		z = radius * sinf(stackAngle);              // r * sin(u)

		// add (sectorCount+1) vertices per stack
		// the first and last vertices have same position and normal, but different tex coords
		for (int j = 0; j <= sectorCount; ++j)
		{
			sectorAngle = j * sectorStep;           // starting from 0 to 2pi

			// vertex position (x, y, z)
			x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
			y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
			vertices.push_back(x + center.x);
			vertices.push_back(y + center.y);
			vertices.push_back(z + center.z);

			// normalized vertex normal (nx, ny, nz)
			nx = x * lengthInv;
			ny = y * lengthInv;
			nz = z * lengthInv;
			normals.push_back(nx);
			normals.push_back(ny);
			normals.push_back(nz);
		}
	}

	// generate CCW index list of sphere triangles
// k1--k1+1
// |  / |
// | /  |
// k2--k2+1
	vector<unsigned int> indices;
	unsigned int k1, k2;
	for (int i = 0; i < stackCount; ++i)
	{
		k1 = i * (sectorCount + 1);     // beginning of current stack
		k2 = k1 + sectorCount + 1;      // beginning of next stack

		for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
		{
			// 2 triangles per sector excluding first and last stacks
			// k1 => k2 => k1+1
			if (i != 0)
			{
				indices.push_back(k1);
				indices.push_back(k2);
				indices.push_back(k1 + 1);
			}

			// k1+1 => k2 => k2+1
			if (i != (stackCount - 1))
			{
				indices.push_back(k1 + 1);
				indices.push_back(k2);
				indices.push_back(k2 + 1);
			}
		}
	}

	sphere_model.vertexCount = indices.size();

	glGenVertexArrays(1, &sphere_model.vao);
	glBindVertexArray(sphere_model.vao);

	glGenBuffers(1, &sphere_model.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, sphere_model.vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float) + normals.size() * sizeof(float), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());
	glBufferSubData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), normals.size() * sizeof(float), normals.data());

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(vertices.size() * sizeof(float)));
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &sphere_model.ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphere_model.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
}

void initPlants(){
	plants_prog = new Shader("src\\shader\\plants.vs.glsl", "src\\shader\\plants.fs.glsl");
	uniforms.plants.vp_matrix = glGetUniformLocation(plants_prog->getProgramID(), "vp_matrix");
	uniforms.plants.view_matrix = glGetUniformLocation(plants_prog->getProgramID(), "view_matrix");
	uniforms.plants.shadow_matrix = glGetUniformLocation(plants_prog->getProgramID(), "shadow_matrix");
	uniforms.plants.tex = glGetUniformLocation(plants_prog->getProgramID(), "tex");
	uniforms.plants.shadow_tex = glGetUniformLocation(plants_prog->getProgramID(), "shadow_tex");
	uniforms.plants.view_point_light_pos = glGetUniformLocation(plants_prog->getProgramID(), "view_point_light_pos");
	uniforms.plants.mode = glGetUniformLocation(plants_prog->getProgramID(), "mode");
	uniforms.plants.enable_cel_shading = glGetUniformLocation(plants_prog->getProgramID(), "enable_cel_shading");
	uniforms.plants.isLeaves = glGetUniformLocation(plants_prog->getProgramID(), "isLeaves");
	uniforms.plants.plane = glGetUniformLocation(plants_prog->getProgramID(), "plane");

	plants_prog->useShader();
	glUniform1i(uniforms.plants.tex, 0);
	glUniform1i(uniforms.plants.shadow_tex, 1);
	plants_prog->disableShader();

	// Model airplane_model, house_model, water_model, baum_hd_pine_trunk, baum_hd_pine_leaves, baum_hd_trunk, baum_hd_leaves, grass0, grass2;
	initLoadTree("models/trees/baum_hd_pine_trunk.obj", baum_hd_pine_trunk, 0);
	initLoadTree("models/trees/baum_hd_pine_leaves.obj", baum_hd_pine_leaves, 0);
	initLoadTree("models/trees/baum_hd_trunk.obj", baum_hd_trunk, 1);
	initLoadTree("models/trees/baum_hd_leaves.obj", baum_hd_leaves, 1);
	initLoadTree("models/trees/grass.obj", grass0, 2);
	initLoadTree("models/trees/grass2.obj", grass2, 3);
}

void initLoadTree(const char *filename, Model &model, int plantNumber) {
	
	// load plants object
	tinyobj::attrib_t attrib;
	vector<tinyobj::shape_t> shapes;
	vector<tinyobj::material_t> materials;
	string warn;
	string err;
	vector<float> vertices, texcoords, normals, offsets;  // if OBJ preserves vertex order, you can use element array buffer for memory efficiency
	vector<float> vertices2, texcoords2, normals2;
	int index_offset = 0;
	bool ret;

	ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename, "models/trees/");
	if (!warn.empty()) {
		cout << warn << endl;
	}
	if (!err.empty()) {
		cout << err << endl;
	}
	
	if(!ret) {
		cout << "Failed to load OBJ file" << filename << endl;
		//exit(1);
	}

	for (int s = 0; s < shapes.size(); ++s) {
		index_offset = 0;
		for (int f = 0; f < shapes[s].mesh.num_face_vertices.size(); ++f) {
			int fv = shapes[s].mesh.num_face_vertices[f];
			unsigned mid = shapes[s].mesh.material_ids[f];
			for (int v = 0; v < fv; ++v) {
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
				if (plantNumber == 0 || plantNumber == 1) {
					// rotate laying tree
					vec3 vertex;
					vec3 normal;
					vertex.x = attrib.vertices[3 * idx.vertex_index + 0];
					vertex.y = attrib.vertices[3 * idx.vertex_index + 1];
					vertex.z = attrib.vertices[3 * idx.vertex_index + 2];
					normal.x = attrib.normals[3 * idx.normal_index + 0];
					normal.y = attrib.normals[3 * idx.normal_index + 1];
					normal.z = attrib.normals[3 * idx.normal_index + 2];
					vertex = tree_rotate_matrix * vertex;
					normal = tree_rotate_matrix * normal;
					vertices.push_back(vertex.x);
					vertices.push_back(vertex.y);
					vertices.push_back(vertex.z);
					normals.push_back(normal.x);
					normals.push_back(normal.y);
					normals.push_back(normal.z);
					texcoords.push_back(attrib.texcoords[2 * idx.texcoord_index + 0]);
					texcoords.push_back(attrib.texcoords[2 * idx.texcoord_index + 1]);
				}
				else {
					if (plantNumber == 3 && mid == 1) {
						vertices2.push_back(attrib.vertices[3 * idx.vertex_index + 0]);
						vertices2.push_back(attrib.vertices[3 * idx.vertex_index + 1]);
						vertices2.push_back(attrib.vertices[3 * idx.vertex_index + 2]);
						normals2.push_back(attrib.normals[3 * idx.normal_index + 0]);
						normals2.push_back(attrib.normals[3 * idx.normal_index + 1]);
						normals2.push_back(attrib.normals[3 * idx.normal_index + 2]);
						texcoords2.push_back(attrib.texcoords[2 * idx.texcoord_index + 0]);
						texcoords2.push_back(attrib.texcoords[2 * idx.texcoord_index + 1]);
					}
					else {
						vertices.push_back(attrib.vertices[3 * idx.vertex_index + 0]);
						vertices.push_back(attrib.vertices[3 * idx.vertex_index + 1]);
						vertices.push_back(attrib.vertices[3 * idx.vertex_index + 2]);
						normals.push_back(attrib.normals[3 * idx.normal_index + 0]);
						normals.push_back(attrib.normals[3 * idx.normal_index + 1]);
						normals.push_back(attrib.normals[3 * idx.normal_index + 2]);
						texcoords.push_back(attrib.texcoords[2 * idx.texcoord_index + 0]);
						texcoords.push_back(attrib.texcoords[2 * idx.texcoord_index + 1]);
					}
				}
			}
			index_offset += fv;
			if (plantNumber == 3 && mid == 1)
				model.vertexCount2 += fv;
			else
				model.vertexCount += fv;
		}
	}
	for(int i = 0; i < m_plantManager->m_numPlantInstance[plantNumber] * 3; i++){
		offsets.push_back(m_plantManager->m_plantInstancePositions[plantNumber][i]);
	}
	glGenVertexArrays(1, &model.vao);
	glBindVertexArray(model.vao);

	glGenBuffers(1, &model.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, model.vbo);

	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float) + texcoords.size() * sizeof(float) + normals.size() * sizeof(float) + offsets.size() * sizeof(float), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());
	glBufferSubData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), texcoords.size() * sizeof(float), texcoords.data());
	glBufferSubData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float) + texcoords.size() * sizeof(float), normals.size() * sizeof(float), normals.data());
	// m_plantManager->m_plantInstancePositions[plantNumber]
	glBufferSubData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float) + texcoords.size() * sizeof(float) + normals.size() * sizeof(float), offsets.size() * sizeof(float), offsets.data());

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(vertices.size() * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(vertices.size() * sizeof(float) + texcoords.size() * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(vertices.size() * sizeof(float) + texcoords.size() * sizeof(float) + normals.size() * sizeof(float)));
	glEnableVertexAttribArray(3);
	glVertexAttribDivisor(3, 1);

	if (plantNumber == 3) {
		glGenVertexArrays(1, &model.vao2);
		glBindVertexArray(model.vao2);

		glGenBuffers(1, &model.vbo2);
		glBindBuffer(GL_ARRAY_BUFFER, model.vbo2);

		glBufferData(GL_ARRAY_BUFFER, vertices2.size() * sizeof(float) + texcoords2.size() * sizeof(float) + normals2.size() * sizeof(float) + offsets.size() * sizeof(float), NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, vertices2.size() * sizeof(float), vertices2.data());
		glBufferSubData(GL_ARRAY_BUFFER, vertices2.size() * sizeof(float), texcoords2.size() * sizeof(float), texcoords2.data());
		glBufferSubData(GL_ARRAY_BUFFER, vertices2.size() * sizeof(float) + texcoords2.size() * sizeof(float), normals2.size() * sizeof(float), normals2.data());
		glBufferSubData(GL_ARRAY_BUFFER, vertices2.size() * sizeof(float) + texcoords2.size() * sizeof(float) + normals2.size() * sizeof(float), offsets.size() * sizeof(float), offsets.data());

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(vertices2.size() * sizeof(float)));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(vertices2.size() * sizeof(float) + texcoords2.size() * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(vertices2.size() * sizeof(float) + texcoords2.size() * sizeof(float) + normals2.size() * sizeof(float)));
		glEnableVertexAttribArray(3);
		glVertexAttribDivisor(3, 1);
	}
	
	texture_data tdata;
	glGenTextures(1, &model.tex);
	glBindTexture(GL_TEXTURE_2D, model.tex);
	tdata = loadImg(materials[0].diffuse_texname.c_str());
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tdata.width, tdata.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tdata.data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	if(materials.size() == 2){
		texture_data tdata;
		glGenTextures(1, &model.tex2);
		glBindTexture(GL_TEXTURE_2D, model.tex2);
		tdata = loadImg(materials[1].diffuse_texname.c_str());
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tdata.width, tdata.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tdata.data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		
	}

}


void initAirplane() {
	airplane_prog = new Shader("src\\shader\\airplane.vs.glsl", "src\\shader\\airplane.fs.glsl");
	uniforms.airplane.m_matrix = glGetUniformLocation(airplane_prog->getProgramID(), "m_matrix");
	uniforms.airplane.mvp_matrix = glGetUniformLocation(airplane_prog->getProgramID(), "mvp_matrix");
	uniforms.airplane.mv_matrix = glGetUniformLocation(airplane_prog->getProgramID(), "mv_matrix");
	uniforms.airplane.view_matrix = glGetUniformLocation(airplane_prog->getProgramID(), "view_matrix");
	uniforms.airplane.shadow_matrix = glGetUniformLocation(airplane_prog->getProgramID(), "shadow_matrix");
	uniforms.airplane.tex = glGetUniformLocation(airplane_prog->getProgramID(), "tex");
	uniforms.airplane.shadow_tex = glGetUniformLocation(airplane_prog->getProgramID(), "shadow_tex");
	uniforms.airplane.view_point_light_pos = glGetUniformLocation(airplane_prog->getProgramID(), "view_point_light_pos");
	uniforms.airplane.mode = glGetUniformLocation(airplane_prog->getProgramID(), "mode");
	uniforms.airplane.enable_cel_shading = glGetUniformLocation(airplane_prog->getProgramID(), "enable_cel_shading");
	uniforms.airplane.plane = glGetUniformLocation(airplane_prog->getProgramID(), "plane");

	airplane_prog->useShader();
	glUniform1i(uniforms.airplane.tex, 0);
	glUniform1i(uniforms.airplane.shadow_tex, 1);
	airplane_prog->disableShader();

	// load airplane object
	tinyobj::attrib_t attrib;
	vector<tinyobj::shape_t> shapes;
	vector<tinyobj::material_t> materials;
	string warn;
	string err;
	vector<float> vertices, texcoords, normals;  // if OBJ preserves vertex order, you can use element array buffer for memory efficiency
	int index_offset = 0;
	bool ret;

	ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, "models/airplane.obj", "models/");
	if (!warn.empty()) {
		cout << warn << endl;
	}
	if (!err.empty()) {
		cout << err << endl;
	}
	if (!ret) {
		exit(1);
	}

	for (int s = 0; s < shapes.size(); ++s) {
		index_offset = 0;
		for (int f = 0; f < shapes[s].mesh.num_face_vertices.size(); ++f) {
			int fv = shapes[s].mesh.num_face_vertices[f];
			for (int v = 0; v < fv; ++v) {
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
				vertices.push_back(attrib.vertices[3 * idx.vertex_index + 0]);
				vertices.push_back(attrib.vertices[3 * idx.vertex_index + 1]);
				vertices.push_back(attrib.vertices[3 * idx.vertex_index + 2]);
				texcoords.push_back(attrib.texcoords[2 * idx.texcoord_index + 0]);
				texcoords.push_back(attrib.texcoords[2 * idx.texcoord_index + 1]);
				normals.push_back(attrib.normals[3 * idx.normal_index + 0]);
				normals.push_back(attrib.normals[3 * idx.normal_index + 1]);
				normals.push_back(attrib.normals[3 * idx.normal_index + 2]);
			}
			index_offset += fv;
			airplane_model.vertexCount += fv;
		}
	}

	glGenVertexArrays(1, &airplane_model.vao);
	glBindVertexArray(airplane_model.vao);

	glGenBuffers(1, &airplane_model.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, airplane_model.vbo);

	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float) + texcoords.size() * sizeof(float) + normals.size() * sizeof(float), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());
	glBufferSubData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), texcoords.size() * sizeof(float), texcoords.data());
	glBufferSubData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float) + texcoords.size() * sizeof(float), normals.size() * sizeof(float), normals.data());

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(vertices.size() * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(vertices.size() * sizeof(float) + texcoords.size() * sizeof(float)));
	glEnableVertexAttribArray(2);

	// load airplane texture
	texture_data tdata;
	glGenTextures(1, &airplane_model.tex);
	glBindTexture(GL_TEXTURE_2D, airplane_model.tex);
	tdata = loadImg("models/textures/Airplane_smooth_DefaultMaterial_BaseMap.jpg");
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tdata.width, tdata.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tdata.data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void initHouse() {
	house_prog = new Shader("src\\shader\\house.vs.glsl", "src\\shader\\house.fs.glsl");

	uniforms.house.m_matrix = glGetUniformLocation(house_prog->getProgramID(), "m_matrix");
	uniforms.house.mvp_matrix = glGetUniformLocation(house_prog->getProgramID(), "mvp_matrix");
	uniforms.house.mv_matrix = glGetUniformLocation(house_prog->getProgramID(), "mv_matrix");
	uniforms.house.view_matrix = glGetUniformLocation(house_prog->getProgramID(), "view_matrix");
	uniforms.house.shadow_matrix = glGetUniformLocation(house_prog->getProgramID(), "shadow_matrix");
	uniforms.house.tex = glGetUniformLocation(house_prog->getProgramID(), "tex");
	uniforms.house.shadow_tex = glGetUniformLocation(house_prog->getProgramID(), "shadow_tex");
	uniforms.house.view_point_light_pos = glGetUniformLocation(house_prog->getProgramID(), "view_point_light_pos");
	uniforms.house.mode = glGetUniformLocation(house_prog->getProgramID(), "mode");
	uniforms.house.normal_map = glGetUniformLocation(house_prog->getProgramID(), "normal_map");
	uniforms.house.enable_normal_mapping = glGetUniformLocation(house_prog->getProgramID(), "enable_normal_mapping");
	uniforms.house.enable_cel_shading = glGetUniformLocation(house_prog->getProgramID(), "enable_cel_shading");
	uniforms.house.plane = glGetUniformLocation(house_prog->getProgramID(), "plane");

	house_prog->useShader();
	glUniform1i(uniforms.house.tex, 0);
	glUniform1i(uniforms.house.shadow_tex, 1);
	glUniform1i(uniforms.house.normal_map, 2);
	
	house_prog->disableShader();

	// load house object
	tinyobj::attrib_t attrib;
	vector<tinyobj::shape_t> shapes;
	vector<tinyobj::material_t> materials;
	string warn;
	string err;
	vector<float> vertices, texcoords, normals, tangents, bitangents;
	int index_offset = 0;
	bool ret;

	ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, "models/medievalHouse.obj", "models/");
	if (!warn.empty()) {
		cout << warn << endl;
	}
	if (!err.empty()) {
		cout << err << endl;
	}
	if (!ret) {
		exit(1);
	}

	for (int s = 0; s < shapes.size(); ++s) {
		index_offset = 0;
		for (int f = 0; f < shapes[s].mesh.num_face_vertices.size(); ++f) {
			int fv = shapes[s].mesh.num_face_vertices[f];

			for (int v = 0; v < fv; ++v) {
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
				vertices.push_back(attrib.vertices[3 * idx.vertex_index + 0]);
				vertices.push_back(attrib.vertices[3 * idx.vertex_index + 1]);
				vertices.push_back(attrib.vertices[3 * idx.vertex_index + 2]);
				texcoords.push_back(attrib.texcoords[2 * idx.texcoord_index + 0]);
				texcoords.push_back(attrib.texcoords[2 * idx.texcoord_index + 1]);
				normals.push_back(attrib.normals[3 * idx.normal_index + 0]);
				normals.push_back(attrib.normals[3 * idx.normal_index + 1]);
				normals.push_back(attrib.normals[3 * idx.normal_index + 2]);
			}

			// calculate tangent and bitangent for normal mapping
			tinyobj::index_t idx0 = shapes[s].mesh.indices[index_offset + 0];
			tinyobj::index_t idx1 = shapes[s].mesh.indices[index_offset + 1];
			tinyobj::index_t idx2 = shapes[s].mesh.indices[index_offset + 2];
			vec3 p0 = vec3(attrib.vertices[3 * idx0.vertex_index + 0], attrib.vertices[3 * idx0.vertex_index + 1], attrib.vertices[3 * idx0.vertex_index + 2]);
			vec3 p1 = vec3(attrib.vertices[3 * idx1.vertex_index + 0], attrib.vertices[3 * idx1.vertex_index + 1], attrib.vertices[3 * idx1.vertex_index + 2]);
			vec3 p2 = vec3(attrib.vertices[3 * idx2.vertex_index + 0], attrib.vertices[3 * idx2.vertex_index + 1], attrib.vertices[3 * idx2.vertex_index + 2]);
			vec3 uv0 = vec3(attrib.texcoords[2 * idx0.texcoord_index + 0], attrib.texcoords[2 * idx0.texcoord_index + 1], 0);
			vec3 uv1 = vec3(attrib.texcoords[2 * idx1.texcoord_index + 0], attrib.texcoords[2 * idx1.texcoord_index + 1], 0);
			vec3 uv2 = vec3(attrib.texcoords[2 * idx2.texcoord_index + 0], attrib.texcoords[2 * idx2.texcoord_index + 1], 0);
			vec3 n0 = vec3(attrib.normals[3 * idx0.normal_index + 0], attrib.normals[3 * idx0.normal_index + 1], attrib.normals[3 * idx0.normal_index + 2]);
			vec3 n1 = vec3(attrib.normals[3 * idx1.normal_index + 0], attrib.normals[3 * idx1.normal_index + 1], attrib.normals[3 * idx1.normal_index + 2]);
			vec3 n2 = vec3(attrib.normals[3 * idx2.normal_index + 0], attrib.normals[3 * idx2.normal_index + 1], attrib.normals[3 * idx2.normal_index + 2]);
			
			vec3 edge1 = p1 - p0;
			vec3 edge2 = p2 - p0;
			vec2 deltaUV1 = uv1 - uv0;
			vec2 deltaUV2 = uv2 - uv0;
			float oneoverdeterminent = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
			vec3 tangent = (edge1 * deltaUV2.y - edge2 * deltaUV1.y) * oneoverdeterminent;
			vec3 bitangent = (edge2 * deltaUV1.x - edge1 * deltaUV2.x) * oneoverdeterminent;
			for (int i = 0; i < 3; i++) {
				tangents.push_back(tangent.x);
				tangents.push_back(tangent.y);
				tangents.push_back(tangent.z);
				bitangents.push_back(bitangent.x);
				bitangents.push_back(bitangent.y);
				bitangents.push_back(bitangent.z);
			}

			index_offset += fv;
			house_model.vertexCount += fv;
		}
	}

	glGenVertexArrays(1, &house_model.vao);
	glBindVertexArray(house_model.vao);

	glGenBuffers(1, &house_model.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, house_model.vbo);

	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float) + texcoords.size() * sizeof(float) + normals.size() * sizeof(float) + tangents.size() * sizeof(float) + bitangents.size() * sizeof(float), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());
	glBufferSubData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), texcoords.size() * sizeof(float), texcoords.data());
	glBufferSubData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float) + texcoords.size() * sizeof(float), normals.size() * sizeof(float), normals.data());
	// tangent
	glBufferSubData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float) + texcoords.size() * sizeof(float) + normals.size() * sizeof(float), tangents.size() * sizeof(float), tangents.data());
	// bitangent
	glBufferSubData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float) + texcoords.size() * sizeof(float) + normals.size() * sizeof(float) + tangents.size() * sizeof(float), bitangents.size() * sizeof(float), bitangents.data());

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(vertices.size() * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(vertices.size() * sizeof(float) + texcoords.size() * sizeof(float)));
	glEnableVertexAttribArray(2);
	// tangent
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(vertices.size() * sizeof(float) + texcoords.size() * sizeof(float) + normals.size() * sizeof(float)));
	glEnableVertexAttribArray(3);
	// bitangent
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(vertices.size() * sizeof(float) + texcoords.size() * sizeof(float) + normals.size() * sizeof(float) + tangents.size() * sizeof(float)));
	glEnableVertexAttribArray(4);
	

	// load house texture
	texture_data tdata;
	glGenTextures(1, &house_model.tex);
	glBindTexture(GL_TEXTURE_2D, house_model.tex);
	tdata = loadImg("models/textures/Medieval_house_Diffuse.jpg");
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tdata.width, tdata.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tdata.data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glGenerateMipmap(GL_TEXTURE_2D);

	// load house normal map
	texture_data tdata_normal;
	glGenTextures(1, &house_model.normal_map);
	glBindTexture(GL_TEXTURE_2D, house_model.normal_map);
	tdata_normal = loadImg("models/textures/Medieval_house_Normal.png");
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tdata_normal.width, tdata_normal.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tdata_normal.data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

}

void initWater() {
	water_prog = new Shader("src\\shader\\water.vs.glsl", "src\\shader\\water.fs.glsl");

	uniforms.water.mode = glGetUniformLocation(water_prog->getProgramID(), "mode");
	uniforms.water.vp_matrix = glGetUniformLocation(water_prog->getProgramID(), "vp_matrix");
	uniforms.water.reflectionTex = glGetUniformLocation(water_prog->getProgramID(), "reflectionTex");
	uniforms.water.refractionTex = glGetUniformLocation(water_prog->getProgramID(), "refractionTex");
	uniforms.water.shadow_matrix = glGetUniformLocation(water_prog->getProgramID(), "shadow_matrix");
	uniforms.water.shadow_tex = glGetUniformLocation(water_prog->getProgramID(), "shadow_tex");
	uniforms.water.camera_position = glGetUniformLocation(water_prog->getProgramID(), "camera_position");

	water_prog->useShader();
	glUniform1i(uniforms.water.reflectionTex, 0);
	glUniform1i(uniforms.water.refractionTex, 1);
	glUniform1i(uniforms.water.shadow_tex, 2);
	water_prog->disableShader();

	// load airplane object
	tinyobj::attrib_t attrib;
	vector<tinyobj::shape_t> shapes;
	vector<tinyobj::material_t> materials;
	string warn;
	string err;
	vector<float> vertices, texcoords, normals;  // if OBJ preserves vertex order, you can use element array buffer for memory efficiency
	int index_offset = 0;
	bool ret;

	ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, "models/waterPlane.obj");
	if (!warn.empty()) {
		cout << warn << endl;
	}
	if (!err.empty()) {
		cout << err << endl;
	}
	if (!ret) {
		exit(1);
	}

	for (int s = 0; s < shapes.size(); ++s) {
		index_offset = 0;
		for (int f = 0; f < shapes[s].mesh.num_face_vertices.size(); ++f) {
			int fv = shapes[s].mesh.num_face_vertices[f];
			for (int v = 0; v < fv; ++v) {
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
				vertices.push_back(attrib.vertices[3 * idx.vertex_index + 0] + 512);
				vertices.push_back(attrib.vertices[3 * idx.vertex_index + 1] + 107);
				vertices.push_back(attrib.vertices[3 * idx.vertex_index + 2] + 512);
				texcoords.push_back(attrib.texcoords[2 * idx.texcoord_index + 0]);
				texcoords.push_back(attrib.texcoords[2 * idx.texcoord_index + 1]);
				normals.push_back(attrib.normals[3 * idx.normal_index + 0]);
				normals.push_back(attrib.normals[3 * idx.normal_index + 1]);
				normals.push_back(attrib.normals[3 * idx.normal_index + 2]);
			}
			index_offset += fv;
			water_model.vertexCount += fv;
		}
	}

	glGenVertexArrays(1, &water_model.vao);
	glBindVertexArray(water_model.vao);

	glGenBuffers(1, &water_model.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, water_model.vbo);

	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float) + texcoords.size() * sizeof(float) + normals.size() * sizeof(float), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());
	glBufferSubData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), texcoords.size() * sizeof(float), texcoords.data());
	glBufferSubData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float) + texcoords.size() * sizeof(float), normals.size() * sizeof(float), normals.data());

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(vertices.size() * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(vertices.size() * sizeof(float) + texcoords.size() * sizeof(float)));
	glEnableVertexAttribArray(2);
}



void initDepth() {
	depth_prog = new Shader("src\\shader\\depth.vs.glsl", "src\\shader\\depth.fs.glsl");
	depth_prog2 = new Shader("src\\shader\\depth2.vs.glsl", "src\\shader\\depth2.fs.glsl");

	glGenFramebuffers(1, &depth.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, depth.fbo);

	uniforms.depth.light_mvp_matrix = glGetUniformLocation(depth_prog->getProgramID(), "light_mvp_matrix");

	uniforms.depth2.light_vp_matrix = glGetUniformLocation(depth_prog2->getProgramID(), "light_vp_matrix");
	uniforms.depth2.isLeaves = glGetUniformLocation(depth_prog2->getProgramID(), "isLeaves");

	glGenTextures(1, &depth.tex);
	glBindTexture(GL_TEXTURE_2D, depth.tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth.tex, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

////////////////////////////////////////////////
// The following functions don't need to change
void initScene() {
	m_plantManager = new PlantManager();		
	m_terrain = new Terrain(&depth);
	m_renderer->appendTerrain(m_terrain);
}
void updateAirplane(const glm::mat4 &viewMatrix) {
	// apply yaw
	glm::mat4 rm = viewMatrix;
	rm = glm::transpose(rm);
	glm::vec3 tangent(-1.0 * rm[2].x, 0.0, -1.0 * rm[2].z);
	tangent = glm::normalize(tangent);
	glm::vec3 bitangent = glm::normalize(glm::cross(glm::vec3(0.0, 1.0, 0.0), tangent));
	glm::vec3 normal = glm::normalize(glm::cross(tangent, bitangent));

	//m_airplaneRotMat[0] = glm::vec4(bitangent, 0.0);
	//m_airplaneRotMat[1] = glm::vec4(normal, 0.0);
	//m_airplaneRotMat[2] = glm::vec4(tangent, 0.0);
	//m_airplaneRotMat[3] = glm::vec4(0.0, 0.0, 0.0, 1.0);
	m_airplaneRotMat = glm::mat4(1.0);

	m_airplanePosition = m_lookAtCenter;
}

// adjust camera position and look-at center with terrain
void adjustCameraPositionWithTerrain() {
	// adjust camera height
	glm::vec3 cp = m_lookAtCenter;
	float ty = m_terrain->getHeight(cp.x, cp.z);
	if (cp.y < ty + 3.0) {
		m_lookAtCenter.y = ty + 3.0;
		m_eye.y = m_eye.y + (ty + 3.0 - cp.y);
	}
	cp = m_eye;
	ty = m_terrain->getHeight(cp.x, cp.z);
	if (cp.y < ty + 3.0) {
		m_lookAtCenter.y = ty + 3.0;
		m_eye.y = m_eye.y + (ty + 3.0 - cp.y);
	}
}
