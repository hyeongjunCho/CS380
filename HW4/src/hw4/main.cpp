// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>
#include <common/affine.hpp>
#include <common/geometry.hpp>
#include <common/arcball.hpp>

float g_groundSize = 100.0f;
float g_groundY = -2.5f;

GLuint lightLocGround, lightLocObject, lightLocObject2, lightLocObject3;

// View properties
glm::mat4 Projection;
float windowWidth = 1024.0f;
float windowHeight = 768.0f;
int frameBufferWidth = 0;
int frameBufferHeight = 0;
float fov = 45.0f;
float fovy = fov;

// Model properties
Model ground, object, object2, object3;
glm::mat4 skyRBT;
glm::mat4 eyeRBT;
const glm::mat4 worldRBT = glm::mat4(1.0f);
glm::mat4 objectRBT = glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 0.0f, 0.0f)) * glm::rotate(glm::mat4(1.0f), 10.0f, glm::vec3(1.0f, 0.0f, 0.0f)) * glm::scale(7.0f, 7.0f, 7.0f);
glm::mat4 object2RBT = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)) * glm::rotate(glm::mat4(1.0f), 10.0f, glm::vec3(1.0f, 0.0f, 0.0f)) * glm::scale(7.0f, 7.0f, 7.0f);
glm::mat4 object3RBT = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::rotate(glm::mat4(1.0f), 10.0f, glm::vec3(1.0f, 0.0f, 0.0f)) * glm::scale(7.0f, 7.0f, 7.0f);
glm::mat4 arcballRBT = glm::mat4(1.0f);
glm::mat4 aFrame;

// Mouse interaction
bool MOUSE_LEFT_PRESS = false; bool MOUSE_MIDDLE_PRESS = false; bool MOUSE_RIGHT_PRESS = false;

// Transformation
glm::mat4 m = glm::mat4(1.0f);

// Manipulation index
int object_index = 0; int view_index = 0; int sky_type = 0;

// Arcball manipulation
Model arcBall;
float arcBallScreenRadius = 0.25f * min(windowWidth, windowHeight);
float arcBallScale = 0.01f; float ScreenToEyeScale = 0.01f;
float prev_x = 0.0f; float prev_y = 0.0f;

// Light
glm::vec3 lightColor = glm::vec3(0.0f, 1.0f, 1.0f);	//color(directional)
glm::vec3 lightColorp = glm::vec3(0.0f, 1.0f, 1.0f);	//color(point)
glm::vec3 lightColors = glm::vec3(1.0f, 1.0f, 1.0f);	//color(spotlight)
GLfloat lightIntensity;	//intensity(directioal)
GLfloat lightIntensityp;	//intensity(point)
GLfloat lightIntensitys;	//intensity(spotlight)
glm::vec3 lightVec;	//direction(directional)
glm::vec3 lightAxis;	//axis(spotlight)
glm::vec3 lightLocationp;	//location(point)
glm::vec3 lightLocations;	//location(spotlight)
GLfloat lightRadius;	//radius(spotlight)
glm::vec3 lightFallout;
GLuint dLight = 0;	//directional
GLuint pLight = 0;	//point
GLuint sLight = 0;	//spotlight
glm::vec3 dRotateAxis = glm::vec3(-1.0f, 0.0f, 0.0f);
glm::vec3 pRotateAxis = glm::vec3(-1.0f, 0.0f, 0.0f);
glm::vec3 sRotateAxis = glm::vec3(0.0f, -1.0f, 0.0f);
glm::vec3 pRotateCenter = glm::vec3(0.5f, 0.4f, -3.8f);
glm::mat4 pRotateRBT = glm::translate(glm::mat4(1.0f), pRotateCenter);

glm::vec3 rabbitColor = glm::vec3(0.1, 0.3, 1.0);
glm::vec3 *rabbitColorp = &rabbitColor;

int size;

static bool non_ego_cube_manipulation()
{
	return object_index != 0 && view_index != object_index;
}

static bool use_arcball()
{
	return (object_index == 0 && sky_type == 0) || non_ego_cube_manipulation();
}

static void window_size_callback(GLFWwindow* window, int width, int height)
{
	// Get resized size and set to current window
	windowWidth = (float)width;
	windowHeight = (float)height;

	// glViewport accept pixel size, please use glfwGetFramebufferSize rather than window size.
	// window size != framebuffer size
	glfwGetFramebufferSize(window, &frameBufferWidth, &frameBufferHeight);
	glViewport(0, 0, frameBufferWidth, frameBufferHeight);

	arcBallScreenRadius = 0.25f * min(frameBufferWidth, frameBufferHeight);

	if (frameBufferWidth >= frameBufferHeight)
	{
		fovy = fov;
	}
	else {
		const float RAD_PER_DEG = 0.5f * glm::pi<float>() / 180.0f;
		fovy = atan2(sin(fov * RAD_PER_DEG) * (float)frameBufferHeight / (float)frameBufferWidth, cos(fov * RAD_PER_DEG)) / RAD_PER_DEG;
	}

	// Update projection matrix
	Projection = glm::perspective(fov, windowWidth / windowHeight, 0.1f, 100.0f);
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	MOUSE_LEFT_PRESS |= (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS);
	MOUSE_RIGHT_PRESS |= (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS);
	MOUSE_MIDDLE_PRESS |= (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS);

	MOUSE_LEFT_PRESS &= !(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE);
	MOUSE_RIGHT_PRESS &= !(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE);
	MOUSE_MIDDLE_PRESS &= !(button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE);

	if (action == GLFW_RELEASE) {
		prev_x = 0.0f; prev_y = 0.0f;
	}
}

void setWrtFrame()
{
	switch (object_index)
	{
	case 0:
		// world-sky: transFact(worldRBT) * linearFact(skyRBT), sky-sky: transFact(skyRBT) * linearFact(skyRBT)
		aFrame = (sky_type == 0) ? linearFact(skyRBT) : skyRBT;
		break;
	case 1:
		aFrame = transFact(objectRBT) * linearFact(eyeRBT);
		break;
	case 2:
		aFrame = transFact(object2RBT) * linearFact(eyeRBT);
		break;
	case 3:
		aFrame = transFact(object3RBT) * linearFact(eyeRBT);
		break;
	}
}

static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (view_index != 0 && object_index == 0) return;
	// Convert mouse pointer into screen space. (http://gamedev.stackexchange.com/questions/83570/why-is-the-origin-in-computer-graphics-coordinates-at-the-top-left)
	xpos = xpos * ((float)frameBufferWidth / windowWidth);
	ypos = (float)frameBufferHeight - ypos * ((float)frameBufferHeight / windowHeight) - 1.0f;

	double dx_t = xpos - prev_x;
	double dy_t = ypos - prev_y;
	double dx_r = xpos - prev_x;
	double dy_r = ypos - prev_y;

	if (view_index == 0 && object_index == 0)
	{
		if (sky_type == 0) { dx_t = -dx_t; dy_t = -dy_t; dx_r = -dx_r; dy_r = -dy_r; }
		else { dx_r = -dx_r; dy_r = -dy_r; }
	}

	if (MOUSE_LEFT_PRESS)
	{
		if (prev_x - 1e-16< 1e-8 && prev_y - 1e-16 < 1e-8) {
			prev_x = (float)xpos; prev_y = (float)ypos;
			return;
		}

		if (use_arcball())
		{
			// 1. Get eye coordinate of arcball and compute its screen coordinate
			glm::vec4 arcball_eyecoord = glm::inverse(eyeRBT) * arcballRBT * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			glm::vec2 arcballCenter = eye_to_screen(
				glm::vec3(arcball_eyecoord),
				Projection,
				frameBufferWidth,
				frameBufferHeight
				);

			// compute z index
			glm::vec2 p1 = glm::vec2(prev_x, prev_y) - arcballCenter;
			glm::vec2 p2 = glm::vec2(xpos, ypos) - arcballCenter;

			glm::vec3 v1 = glm::normalize(glm::vec3(p1.x, p1.y, sqrt(max(0.0f, pow(arcBallScreenRadius, 2) - pow(p1.x, 2) - pow(p1.y, 2)))));
			glm::vec3 v2 = glm::normalize(glm::vec3(p2.x, p2.y, sqrt(max(0.0f, pow(arcBallScreenRadius, 2) - pow(p2.x, 2) - pow(p2.y, 2)))));

			glm::quat w1, w2;
			// 2. Compute arcball rotation (Chatper 8)
			if (object_index == 0 && view_index == 0 && sky_type == 0) { w1 = glm::quat(0.0f, -v1); w2 = glm::quat(0.0f, v2); }
			else { w1 = glm::quat(0.0f, v2); w2 = glm::quat(0.0f, -v1); }

			// Arcball: axis k and 2*theta (Chatper 8)
			glm::quat w = w1 * w2;
			m = glm::toMat4(w);

		}
		else // ego motion
		{
			glm::quat xRotation = glm::angleAxis((float)-dy_r * 0.1f, glm::vec3(1.0f, 0.0f, 0.0f));
			glm::quat yRotation = glm::angleAxis((float)dx_r * 0.1f, glm::vec3(0.0f, 1.0f, 0.0f));

			glm::quat w = yRotation * xRotation;
			m = glm::toMat4(w);
		}

		// Apply transformation with auxiliary frame
		setWrtFrame();
		if (object_index == 0)
		{
			glm::mat4 old = skyRBT;
			glm::mat4 n = aFrame * m * glm::inverse(aFrame);
			skyRBT = n * skyRBT;
			glm::mat4 o = glm::inverse(old) * glm::inverse(n) * old;
			lightVec = glm::vec3(o * glm::vec4(lightVec, 0.0f));
			lightAxis = glm::vec3(o * glm::vec4(lightAxis, 0.0f));
			//lightLocationp = glm::vec3(o * glm::vec4(lightLocationp, 1.0f));
			lightLocations = glm::vec3(o * glm::vec4(lightLocations, 1.0f));
			dRotateAxis = glm::vec3(o * glm::vec4(dRotateAxis, 0.0f));
			pRotateAxis = glm::vec3(o * glm::vec4(pRotateAxis, 0.0f));
			sRotateAxis = glm::vec3(o * glm::vec4(sRotateAxis, 0.0f));
			pRotateCenter = glm::vec3(o * glm::vec4(pRotateCenter, 1.0f));
			pRotateRBT = o * pRotateRBT;
			//lightVec = glm::vec3(glm::inverse(m) * glm::vec4(lightVec, 0.0f));
			
			//skyRBT = aFrame * m * glm::inverse(aFrame) * skyRBT;

		}
		else if (object_index == 1) { objectRBT = aFrame * m * glm::inverse(aFrame) * objectRBT; }
		else if (object_index == 2) { object2RBT = aFrame * m * glm::inverse(aFrame) * object2RBT; }
		else if (object_index == 3) { object3RBT = aFrame * m * glm::inverse(aFrame) * object3RBT; }


		glm::vec3 temp = glm::vec3(linearFact(skyRBT) * glm::vec4(lightLocations, 1.0f));
		//printf("%f, %f, %f, %f, %f, %f, %f, %f, %f\n", lightAxis.x, lightAxis.y, lightAxis.z, lightLocations.x, lightLocations.y, lightLocations.z, temp.x, temp.y, temp.z);

		prev_x = (float)xpos; prev_y = (float)ypos;
	}
}


void toggleEyeMode()
{
	view_index = (view_index + 1) % 2;
	if (view_index == 0) {
		std::cout << "Using sky view" << std::endl;
	}
	else {
		std::cout << "Using object " << view_index << " view" << std::endl;
	}
}

void cycleManipulation()
{
	object_index = (object_index + 1) % 2;
	if (object_index == 0) {
		std::cout << "Manipulating sky frame" << std::endl;
	}
	else {
		std::cout << "Manipulating object " << object_index << std::endl;
	}
}

void cycleSkyAMatrix()
{
	if (object_index == 0 && view_index == 0) {
		sky_type = (sky_type + 1) % 2;
		if (sky_type == 0) {
			std::cout << "world-sky" << std::endl;
		}
		else {
			std::cout << "sky-sky" << std::endl;
		}
	}
	else {
		std::cout << "Unable to change sky mode" << std::endl;
	}
}

static void keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	glm::mat4 m;
	if (action == GLFW_PRESS)
	{
		switch (key)
		{
		case GLFW_KEY_H:
			std::cout << "CS380 Homework Assignment 2" << std::endl;
			std::cout << "keymaps:" << std::endl;
			std::cout << "h\t\t Help command" << std::endl;
			std::cout << "v\t\t Change eye matrix" << std::endl;
			std::cout << "o\t\t Change current manipulating object" << std::endl;
			std::cout << "m\t\t Change auxiliary frame between world-sky and sky-sky" << std::endl;
			break;
		case GLFW_KEY_V:
			
			break;
		case GLFW_KEY_O:
			object_index = (object_index + 1) % 4;
			break;
		case GLFW_KEY_M:
			
			break;
		case GLFW_KEY_1:
			dLight = (dLight + 1) % 2;
			break;
		case GLFW_KEY_2:
			pLight = (pLight + 1) % 2;
			break;
		case GLFW_KEY_3:
			sLight = (sLight + 1) % 2;
			break;
		default:
			break;
		}
	}
}

int main(void)
{
	// Initialise GLFW
	if (!glfwInit())
	{
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow((int)windowWidth, (int)windowHeight, "Lab 3", NULL, NULL);
	if (window == NULL) {
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetWindowSizeCallback(window, window_size_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, cursor_pos_callback);
	glfwSetKeyCallback(window, keyboard_callback);

	glfwGetFramebufferSize(window, &frameBufferWidth, &frameBufferHeight);

	// Clear with sky color
	glClearColor((GLclampf)(128. / 255.), (GLclampf)(200. / 255.), (GLclampf)(255. / 255.), (GLclampf) 0.);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);	


	Projection = glm::perspective(fov, windowWidth / windowHeight, 0.1f, 100.0f);
	skyRBT = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.25, 4.0));

	aFrame = linearFact(skyRBT);

	// initial eye frame = sky frame;
	eyeRBT = skyRBT;

	// Initialize Ground Model
	ground = Model();
	init_ground(ground);
	ground.initialize(DRAW_TYPE::ARRAY, "VertexShader.glsl", "FragmentShader.glsl");
	ground.set_projection(&Projection);
	ground.set_eye(&eyeRBT);
	glm::mat4 groundRBT = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, g_groundY, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(g_groundSize, 1.0f, g_groundSize));
	ground.set_model(&groundRBT);

	//TODO: Initialize model by loading .obj file
	object = Model();
	//init_obj(object, "bunny.obj", glm::vec3(0.1, 0.3, 1.0));
	init_obj(object, "bunny.obj", rabbitColor);
	object.initialize(DRAW_TYPE::ARRAY, "VertexShader.glsl", "FragmentShader.glsl");

	object.set_projection(&Projection);
	object.set_eye(&eyeRBT);
	object.set_model(&objectRBT);

	object2 = Model();
	//init_obj(object2, "bunny.obj", glm::vec3(0.1, 0.3, 1.0));
	init_obj(object2, "bunny.obj", rabbitColor);
	object2.initialize(DRAW_TYPE::ARRAY, "VertexShader2.glsl", "FragmentShader2.glsl");

	object2.set_projection(&Projection);
	object2.set_eye(&eyeRBT);
	object2.set_model(&object2RBT);

	object3 = Model();
	//init_obj(object3, "bunny.obj", glm::vec3(0.1, 0.3, 1.0));
	init_obj(object3, "bunny.obj", rabbitColor);
	object3.initialize(DRAW_TYPE::ARRAY, "VertexShader3.glsl", "FragmentShader3.glsl");

	object3.set_projection(&Projection);
	object3.set_eye(&eyeRBT);
	object3.set_model(&object3RBT);

	arcBall = Model();
	init_sphere(arcBall);
	arcBall.initialize(DRAW_TYPE::INDEX, "VertexShader.glsl", "FragmentShader.glsl");

	arcBall.set_projection(&Projection);
	arcBall.set_eye(&eyeRBT);
	arcBall.set_model(&arcballRBT);

	//TODO Setting Light Vectors	
	//lightLocGround = glGetUniformLocation(ground.GLSLProgramID, "uLight");
	//lightLocObject = glGetUniformLocation(object.GLSLProgramID, "uLight");
	//lightLocObject2 = glGetUniformLocation(object2.GLSLProgramID, "uLight");
	//lightLocObject3 = glGetUniformLocation(object3.GLSLProgramID, "uLight");

	lightColor = glm::vec3(0.0f, 1.0f, 1.0f);
	lightIntensity = 0.9f;
	lightVec = glm::vec3(1.0f, 0.7f, 0.0f);

	lightColorp = glm::vec3(0.0f, 1.0f, 1.0f);
	lightIntensityp = 1.0f;
	//lightLocationp = glm::vec3(1.0f, -0.5f, 0.0f);
	lightLocationp = glm::vec3(pRotateRBT * glm::vec4(0.0f, 0.0f, -1.0f,1.0f));
	lightFallout = glm::vec3(1.0f, 0.1f, 0.1f);

	lightColors = glm::vec3(1.0f, 1.0f, 1.0f);
	lightIntensitys = 8.0f;
	lightAxis = glm::vec3(0.0f, -1.0f, -0.25f);
	lightLocations = glm::vec3(-0.2f, 7.0f, -3.5f);
	lightRadius = 0.4f;

	//L = glm::translate(worldRBT, lightLocations);
	//lightAxis = glm::vec3(skyRBT * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));
	//lightLocations = glm::vec3(skyRBT * glm::vec4(0.0f, 2.0f, 0.0f, 1.0f));
	size = (object.colors).size();
	do {
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		eyeRBT = (view_index == 0) ? skyRBT : objectRBT;

		
		rabbitColor.x = rabbitColor.x + 0.001f;
		if (rabbitColor.x > 1.0f) rabbitColor.x -= 1.0f;
		//if (rabbitColor.y > 1.0f) rabbitColor.y -= 1.0f;
		//if (rabbitColor.z > 1.0f) rabbitColor.z -= 1.0f;
		//printf("%f,%f,%f\n", (*rabbitColorp).x, (*rabbitColorp).y, (*rabbitColorp).z);
		std::vector<glm::vec3> colors(size, rabbitColor);

		object.colors = colors;
		object2.colors = colors;
		
		glGenBuffers(1, &object.ColorBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, object.ColorBufferID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*object.colors.size(), &object.colors[0], GL_STATIC_DRAW);
		
		glGenBuffers(1, &object2.ColorBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, object2.ColorBufferID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*object2.colors.size(), &object2.colors[0], GL_STATIC_DRAW);

		//printf("%f,%f,%f\n", (object.colors[0]).x, (object.colors[0]).y, (object.colors[0]).z);
		
		if (dLight || pLight || sLight)
		{
			setWrtFrame();
			if (dLight)
			{
				glm::quat w1 = glm::quat(cos(0.02f), sin(0.02f) * dRotateAxis);
				glm::mat4 m1 = glm::toMat4(w1);
				//lightVec = glm::vec3(glm::inverse(skyRBT) * aFrame * m1 * inverse(aFrame) * skyRBT * glm::vec4(lightVec, 0.0f));
				lightVec = glm::vec3(m1 * glm::vec4(lightVec, 0.0f));
			}
			if (pLight)
			{
				glm::quat w2 = glm::quat(cos(0.02f), sin(0.02f) * pRotateAxis);
				glm::mat4 m2 = glm::toMat4(w2);
				aFrame = transFact(pRotateRBT) * linearFact(skyRBT);
				//lightLocationp = glm::vec3(glm::inverse(skyRBT) * aFrame * m2 * inverse(aFrame) * skyRBT * glm::vec4(lightLocationp, 1.0f));
				//lightLocationp = glm::vec3(aFrame * m2 * inverse(aFrame) * glm::vec4(lightLocationp, 1.0f));
				//lightLocationp = glm::vec3(1.0f, -0.3f, 0.0f);
				
				pRotateRBT = aFrame * m2 * inverse(aFrame) * pRotateRBT;
				lightLocationp = glm::vec3(pRotateRBT * glm::vec4(0.0f, 0.0f, 2.0f, 1.0f));

				//printf("%f,%f,%f\n", lightLocationp.x, lightLocationp.y, lightLocationp.z);

			}
			if (sLight)
			{
				glm::quat w3 = glm::quat(cos(0.02f), sin(0.02f) * sRotateAxis);
				glm::mat4 m3 = glm::toMat4(w3);
				//lightAxis = glm::vec3(glm::inverse(skyRBT) * glm::inverse(aFrame * m3 * inverse(aFrame)) * skyRBT * glm::vec4(lightAxis, 0.0f));
				//printf("%f,%f,%f, %f,%f,%f\n", lightAxis.x, lightAxis.y, lightAxis.z,sRotateAxis.x, sRotateAxis.y, sRotateAxis.z);
				lightAxis = glm::vec3(m3 * glm::vec4(lightAxis, 0.0f));
			}
		}
		//TODO: pass the light value to the shader
		glUseProgram(ground.GLSLProgramID);
		glUseProgram(object.GLSLProgramID);
		glUseProgram(object2.GLSLProgramID);
		glUseProgram(object3.GLSLProgramID);
		//color
		lightLocGround = glGetUniformLocation(ground.GLSLProgramID, "Color");
		glProgramUniform3f(ground.GLSLProgramID, lightLocGround, lightColor.x, lightColor.y, lightColor.z);
		lightLocObject = glGetUniformLocation(object.GLSLProgramID, "Color");
		glProgramUniform3f(object.GLSLProgramID, lightLocObject, lightColor.x, lightColor.y, lightColor.z);
		lightLocObject2 = glGetUniformLocation(object2.GLSLProgramID, "Color");
		glProgramUniform3f(object2.GLSLProgramID, lightLocObject2, lightColor.x, lightColor.y, lightColor.z);
		lightLocObject3 = glGetUniformLocation(object3.GLSLProgramID, "Color");
		glProgramUniform3f(object3.GLSLProgramID, lightLocObject3, lightColor.x, lightColor.y, lightColor.z);

		//intensity
		lightLocGround = glGetUniformLocation(ground.GLSLProgramID, "Intensity");
		glProgramUniform1f(ground.GLSLProgramID, lightLocGround, lightIntensity);	
		lightLocObject = glGetUniformLocation(object.GLSLProgramID, "Intensity");
		glProgramUniform1f(object.GLSLProgramID, lightLocObject, lightIntensity);
		lightLocObject2 = glGetUniformLocation(object2.GLSLProgramID, "Intensity");
		glProgramUniform1f(object2.GLSLProgramID, lightLocObject2, lightIntensity);
		lightLocObject3 = glGetUniformLocation(object3.GLSLProgramID, "Intensity");
		glProgramUniform1f(object3.GLSLProgramID, lightLocObject3, lightIntensity);
		
		//direction
		lightLocGround = glGetUniformLocation(ground.GLSLProgramID, "Direction");
		glProgramUniform3f(ground.GLSLProgramID, lightLocGround, lightVec.x, lightVec.y, lightVec.z);
		lightLocObject = glGetUniformLocation(object.GLSLProgramID, "Direction");
		glProgramUniform3f(object.GLSLProgramID, lightLocObject, lightVec.x, lightVec.y, lightVec.z);
		lightLocObject2 = glGetUniformLocation(object2.GLSLProgramID, "Direction");
		glProgramUniform3f(object2.GLSLProgramID, lightLocObject2, lightVec.x, lightVec.y, lightVec.z);
		lightLocObject3 = glGetUniformLocation(object3.GLSLProgramID, "Direction");
		glProgramUniform3f(object3.GLSLProgramID, lightLocObject3, lightVec.x, lightVec.y, lightVec.z);


		//point
		//lightColorp = glm::vec3(0.0f, 1.0f, 1.0f);
		//lightIntensityp = 0.9f;
		//lightLocationp = glm::vec3(1.0f, 1.0f, 3.0f);
		//lightFallout = glm::vec3(1.0f, 0.045f, 0.075f);

		//color
		lightLocGround = glGetUniformLocation(ground.GLSLProgramID, "pColor");
		glProgramUniform3f(ground.GLSLProgramID, lightLocGround, lightColorp.x, lightColorp.y, lightColorp.z);
		lightLocObject = glGetUniformLocation(object.GLSLProgramID, "pColor");
		glProgramUniform3f(object.GLSLProgramID, lightLocObject, lightColorp.x, lightColorp.y, lightColorp.z);
		lightLocObject2 = glGetUniformLocation(object2.GLSLProgramID, "pColor");
		glProgramUniform3f(object2.GLSLProgramID, lightLocObject2, lightColorp.x, lightColorp.y, lightColorp.z);
		lightLocObject3 = glGetUniformLocation(object3.GLSLProgramID, "pColor");
		glProgramUniform3f(object3.GLSLProgramID, lightLocObject3, lightColorp.x, lightColorp.y, lightColorp.z);

		//intensity
		lightLocGround = glGetUniformLocation(ground.GLSLProgramID, "pIntensity");
		glProgramUniform1f(ground.GLSLProgramID, lightLocGround, lightIntensityp);
		lightLocObject = glGetUniformLocation(object.GLSLProgramID, "pIntensity");
		glProgramUniform1f(object.GLSLProgramID, lightLocObject, lightIntensityp);
		lightLocObject2 = glGetUniformLocation(object2.GLSLProgramID, "pIntensity");
		glProgramUniform1f(object2.GLSLProgramID, lightLocObject2, lightIntensityp);
		lightLocObject3 = glGetUniformLocation(object3.GLSLProgramID, "pIntensity");
		glProgramUniform1f(object3.GLSLProgramID, lightLocObject3, lightIntensityp);

		//location
		lightLocGround = glGetUniformLocation(ground.GLSLProgramID, "pLocation");
		glProgramUniform3f(ground.GLSLProgramID, lightLocGround, lightLocationp.x, lightLocationp.y, lightLocationp.z);
		lightLocObject = glGetUniformLocation(object.GLSLProgramID, "pLocation");
		glProgramUniform3f(object.GLSLProgramID, lightLocObject, lightLocationp.x, lightLocationp.y, lightLocationp.z);
		lightLocObject2 = glGetUniformLocation(object2.GLSLProgramID, "pLocation");
		glProgramUniform3f(object2.GLSLProgramID, lightLocObject2, lightLocationp.x, lightLocationp.y, lightLocationp.z);
		lightLocObject3 = glGetUniformLocation(object3.GLSLProgramID, "pLocation");
		glProgramUniform3f(object3.GLSLProgramID, lightLocObject3, lightLocationp.x, lightLocationp.y, lightLocationp.z);

		//fallout
		lightLocGround = glGetUniformLocation(ground.GLSLProgramID, "Fallout");
		glProgramUniform3f(ground.GLSLProgramID, lightLocGround, lightFallout.x, lightFallout.y, lightFallout.z);
		lightLocObject = glGetUniformLocation(object.GLSLProgramID, "Fallout");
		glProgramUniform3f(object.GLSLProgramID, lightLocObject, lightFallout.x, lightFallout.y, lightFallout.z);
		lightLocObject2 = glGetUniformLocation(object2.GLSLProgramID, "Fallout");
		glProgramUniform3f(object2.GLSLProgramID, lightLocObject2, lightFallout.x, lightFallout.y, lightFallout.z);
		lightLocObject3 = glGetUniformLocation(object3.GLSLProgramID, "Fallout");
		glProgramUniform3f(object3.GLSLProgramID, lightLocObject3, lightFallout.x, lightFallout.y, lightFallout.z);

		//spotlight
		//lightColors = glm::vec3(1.0f, 1.0f, 1.0f);
		//lightIntensitys = 0.9f;
		//lightAxis = glm::vec3(0.0f, -7.0f, 0.0f);
		//lightLocations = glm::vec3(0.0f, 3.0f, -2.0f);
		//lightRadius = 0.5f;

		//color
		lightLocGround = glGetUniformLocation(ground.GLSLProgramID, "sColor");
		glProgramUniform3f(ground.GLSLProgramID, lightLocGround, lightColors.x, lightColors.y, lightColors.z);
		lightLocObject = glGetUniformLocation(object.GLSLProgramID, "sColor");
		glProgramUniform3f(object.GLSLProgramID, lightLocObject, lightColors.x, lightColors.y, lightColors.z);
		lightLocObject2 = glGetUniformLocation(object2.GLSLProgramID, "sColor");
		glProgramUniform3f(object2.GLSLProgramID, lightLocObject2, lightColors.x, lightColors.y, lightColors.z);
		lightLocObject3 = glGetUniformLocation(object3.GLSLProgramID, "sColor");
		glProgramUniform3f(object3.GLSLProgramID, lightLocObject3, lightColors.x, lightColors.y, lightColors.z);

		//intensity
		lightLocGround = glGetUniformLocation(ground.GLSLProgramID, "sIntensity");
		glProgramUniform1f(ground.GLSLProgramID, lightLocGround, lightIntensitys);
		lightLocObject = glGetUniformLocation(object.GLSLProgramID, "sIntensity");
		glProgramUniform1f(object.GLSLProgramID, lightLocObject, lightIntensitys);
		lightLocObject2 = glGetUniformLocation(object2.GLSLProgramID, "sIntensity");
		glProgramUniform1f(object2.GLSLProgramID, lightLocObject2, lightIntensitys);
		lightLocObject3 = glGetUniformLocation(object3.GLSLProgramID, "sIntensity");
		glProgramUniform1f(object3.GLSLProgramID, lightLocObject3, lightIntensitys);

		//location
		lightLocGround = glGetUniformLocation(ground.GLSLProgramID, "sLocation");
		glProgramUniform3f(ground.GLSLProgramID, lightLocGround, lightLocations.x, lightLocations.y, lightLocations.z);
		lightLocObject = glGetUniformLocation(object.GLSLProgramID, "sLocation");
		glProgramUniform3f(object.GLSLProgramID, lightLocObject, lightLocations.x, lightLocations.y, lightLocations.z);
		lightLocObject2 = glGetUniformLocation(object2.GLSLProgramID, "sLocation");
		glProgramUniform3f(object2.GLSLProgramID, lightLocObject2, lightLocations.x, lightLocations.y, lightLocations.z);
		lightLocObject3 = glGetUniformLocation(object3.GLSLProgramID, "sLocation");
		glProgramUniform3f(object3.GLSLProgramID, lightLocObject3, lightLocations.x, lightLocations.y, lightLocations.z);

		//Axis
		lightLocGround = glGetUniformLocation(ground.GLSLProgramID, "Axis");
		glProgramUniform3f(ground.GLSLProgramID, lightLocGround, lightAxis.x, lightAxis.y, lightAxis.z);
		lightLocObject = glGetUniformLocation(object.GLSLProgramID, "Axis");
		glProgramUniform3f(object.GLSLProgramID, lightLocObject, lightAxis.x, lightAxis.y, lightAxis.z);
		lightLocObject2 = glGetUniformLocation(object2.GLSLProgramID, "Axis");
		glProgramUniform3f(object2.GLSLProgramID, lightLocObject2, lightAxis.x, lightAxis.y, lightAxis.z);
		lightLocObject3 = glGetUniformLocation(object3.GLSLProgramID, "Axis");
		glProgramUniform3f(object3.GLSLProgramID, lightLocObject3, lightAxis.x, lightAxis.y, lightAxis.z);

		//radius
		lightLocGround = glGetUniformLocation(ground.GLSLProgramID, "Radius");
		glProgramUniform1f(ground.GLSLProgramID, lightLocGround, lightRadius);
		lightLocObject = glGetUniformLocation(object.GLSLProgramID, "Radius");
		glProgramUniform1f(object.GLSLProgramID, lightLocObject, lightRadius);
		lightLocObject2 = glGetUniformLocation(object2.GLSLProgramID, "Radius");
		glProgramUniform1f(object2.GLSLProgramID, lightLocObject2, lightRadius);
		lightLocObject3 = glGetUniformLocation(object3.GLSLProgramID, "Radius");
		glProgramUniform1f(object3.GLSLProgramID, lightLocObject3, lightRadius);



		//dLight
		lightLocGround = glGetUniformLocation(ground.GLSLProgramID, "dL");
		glProgramUniform1i(ground.GLSLProgramID, lightLocGround, dLight);
		lightLocObject = glGetUniformLocation(object.GLSLProgramID, "dL");
		glProgramUniform1i(object.GLSLProgramID, lightLocObject, dLight);
		lightLocObject2 = glGetUniformLocation(object2.GLSLProgramID, "dL");
		glProgramUniform1i(object2.GLSLProgramID, lightLocObject2, dLight);
		lightLocObject3 = glGetUniformLocation(object3.GLSLProgramID, "dL");
		glProgramUniform1i(object3.GLSLProgramID, lightLocObject3, dLight);

		//pLight
		lightLocGround = glGetUniformLocation(ground.GLSLProgramID, "pL");
		glProgramUniform1i(ground.GLSLProgramID, lightLocGround, pLight);
		lightLocObject = glGetUniformLocation(object.GLSLProgramID, "pL");
		glProgramUniform1i(object.GLSLProgramID, lightLocObject, pLight);
		lightLocObject2 = glGetUniformLocation(object2.GLSLProgramID, "pL");
		glProgramUniform1i(object2.GLSLProgramID, lightLocObject2, pLight);
		lightLocObject3 = glGetUniformLocation(object3.GLSLProgramID, "pL");
		glProgramUniform1i(object3.GLSLProgramID, lightLocObject3, pLight);

		//sLight
		lightLocGround = glGetUniformLocation(ground.GLSLProgramID, "sL");
		glProgramUniform1i(ground.GLSLProgramID, lightLocGround, sLight);
		lightLocObject = glGetUniformLocation(object.GLSLProgramID, "sL");
		glProgramUniform1i(object.GLSLProgramID, lightLocObject, sLight);
		lightLocObject2 = glGetUniformLocation(object2.GLSLProgramID, "sL");
		glProgramUniform1i(object2.GLSLProgramID, lightLocObject2, sLight);
		lightLocObject3 = glGetUniformLocation(object3.GLSLProgramID, "sL");
		glProgramUniform1i(object3.GLSLProgramID, lightLocObject3, sLight);

		// TODO: draw OBJ model
		object.draw();
		object2.draw();
		object3.draw();

		// Draw wireframe of arcBall with dynamic radius
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		switch (object_index)
		{
		case 0:
			arcballRBT = (sky_type == 0) ? worldRBT : skyRBT;
			break;
		case 1:
			arcballRBT = objectRBT;
			break;
		case 2:
			arcballRBT = object2RBT;
			break;
		case 3:
			arcballRBT = object3RBT;
			break;
		default:
			break;
		}

		ScreenToEyeScale = compute_screen_eye_scale(
			(glm::inverse(eyeRBT) * arcballRBT * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)).z,
			fovy,
			frameBufferHeight
			);
		arcBallScale = ScreenToEyeScale * arcBallScreenRadius;
		arcballRBT = arcballRBT * glm::scale(worldRBT, glm::vec3(arcBallScale, arcBallScale, arcBallScale));
		//arcBall.draw();
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		ground.draw();

		// Swap buffers (Double buffering)
		glfwSwapBuffers(window);
		glfwPollEvents();
	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	// Clean up data structures and glsl objects
	ground.cleanup();
	object.cleanup();
	object2.cleanup();
	object3.cleanup();

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}



