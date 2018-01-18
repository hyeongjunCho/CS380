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

GLint lightLocGround, lightLocRed, lightLocGreen, lightLocArc;

// View properties
glm::mat4 Projection;
float windowWidth = 1024.0f;
float windowHeight = 768.0f;
int frameBufferWidth = 0;
int frameBufferHeight = 0;
float fov = 45.0f;
float fovy = fov;

// Model properties
Model ground, redCube, greenCube;
glm::mat4 skyRBT;
glm::mat4 g_objectRbt[2] = { glm::translate(glm::mat4(1.0f), glm::vec3(-1.5f, 0.5f, 0.0f)) * glm::rotate(glm::mat4(1.0f), -90.0f, glm::vec3(0.0f, 1.0f, 0.0f)), // RBT for redCube
							glm::translate(glm::mat4(1.0f), glm::vec3(1.5f, 0.5f, 0.0f)) * glm::rotate(glm::mat4(1.0f), 90.0f, glm::vec3(0.0f, 1.0f, 0.0f))}; // RBT for greenCube
glm::mat4 eyeRBT;
glm::mat4 worldRBT = glm::mat4(1.0f);
glm::mat4 aFrame;

int select_frame = 0;
int number_of_frames = 3;

int select_object = 0;
int number_of_objects = 3;

int select_ball_frame = 0;
int number_of_ball_frame = 2;

int select_mode = 0;
int number_of_mode = 2;

// Arcball manipulation
Model arcBall;
glm::mat4 arcballRBT = glm::mat4(1.0f);
glm::mat4 arcballRBT_i = arcballRBT;
float arcBallScreenRadius = 0.25f * min(windowWidth, windowHeight); // for the initial assignment
float arcBallScale = 0.01f;

// mouse
int mouse_mode = 0;
double mouse_xpos;
double mouse_ypos;
double click_xpos;
double click_ypos;

// c
glm::mat4 g_objectRbt_i[2] = { glm::translate(glm::mat4(1.0f), glm::vec3(-1.5f, 0.5f, 0.0f)) * glm::rotate(glm::mat4(1.0f), -90.0f, glm::vec3(0.0f, 1.0f, 0.0f)), // RBT for redCube
							glm::translate(glm::mat4(1.0f), glm::vec3(1.5f, 0.5f, 0.0f)) * glm::rotate(glm::mat4(1.0f), 90.0f, glm::vec3(0.0f, 1.0f, 0.0f)) }; // RBT for greenCube


// Function definition
static void window_size_callback(GLFWwindow*, int, int);
static void mouse_button_callback(GLFWwindow*, int, int, int);
static void cursor_pos_callback(GLFWwindow*, double, double);
static void keyboard_callback(GLFWwindow*, int, int, int, int);
void update_fovy(void);

static void get_aFrame(void);
double compute_z(float, float);

// Helper function: Update the vertical field-of-view(float fovy in global)
void update_fovy()
{
	if (frameBufferWidth >= frameBufferHeight)
	{
		fovy = fov;
	}
	else {
		const float RAD_PER_DEG = 0.5f * glm::pi<float>() / 180.0f;
		fovy = (float) atan2(sin(fov * RAD_PER_DEG) * ((float) frameBufferHeight / (float) frameBufferWidth), cos(fov * RAD_PER_DEG)) / RAD_PER_DEG;
	}
}

// TODO: Modify GLFW window resized callback function
static void window_size_callback(GLFWwindow* window, int width, int height)
{
	// Get resized size and set to current window
	windowWidth = (float)width;
	windowHeight = (float)height;

	// glViewport accept pixel size, please use glfwGetFramebufferSize rather than window size.
	// window size != framebuffer size
	glfwGetFramebufferSize(window, &frameBufferWidth, &frameBufferHeight);
	glViewport(0, 0, (GLsizei) frameBufferWidth, (GLsizei) frameBufferHeight);

	// Update projection matrix
	Projection = glm::perspective(fov, ((float) frameBufferWidth / (float) frameBufferHeight), 0.1f, 100.0f);
}

// TODO: Fill up GLFW mouse button callback function
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		switch (button)
		{
		case GLFW_MOUSE_BUTTON_LEFT:
			mouse_mode = 1;
			click_xpos = mouse_xpos;
			click_ypos = mouse_ypos;
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			mouse_mode = 2;
			click_xpos = mouse_xpos;
			click_ypos = mouse_ypos;
			break;
		case GLFW_MOUSE_BUTTON_MIDDLE:
			mouse_mode = 3;
			click_xpos = mouse_xpos;
			click_ypos = mouse_ypos;
			g_objectRbt_i[0] = g_objectRbt[0];
			g_objectRbt_i[1] = g_objectRbt[1];
			break;
		default:
			mouse_mode = 0;
			break;
		}
	}
	else
	{
		mouse_mode = 0;
	}
}

// TODO: Fill up GLFW cursor position callback function
static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
	switch (mouse_mode)
	{
	case 0:
		break;
	case 1:
		if (select_frame == 0
			|| select_object == 1 && select_frame == 2
			|| select_object == 2 && select_frame == 1)
		{
			glm::vec3 k;
			double x = mouse_xpos;
			double y = mouse_ypos;
			double z = 0.0;
			double zpos = 0.0;
			if (!(select_object == 0 && select_ball_frame == 0))
			{
				z = compute_z(x, y);
				zpos = compute_z(xpos, ypos);
			}
			glm::vec2 center = eye_to_screen(glm::vec3(glm::inverse(eyeRBT) * (arcballRBT_i[3])), Projection, windowWidth, windowHeight);
			center.y = windowHeight - center.y;
			glm::vec3 start = glm::normalize(glm::vec3(x-center.x, y-center.y, z));
			glm::vec3 dest = glm::normalize(glm::vec3(xpos-center.x, ypos-center.y, zpos));

			float cos2Theta = glm::dot(start, dest);

			if (cos2Theta > 1.0f - 0.000001) k = glm::vec3(0.0, 0.0, 1.0);
			else if (cos2Theta < -1.0f + 0.000001) k = glm::vec3(0.0, 0.0, -1.0);
			else k = glm::normalize(glm::cross(start,dest));

			float c = sqrt((1 + cos2Theta) * 2) / 2.0;
			if (c > 1.0f - 0.0001) c = 1.0f - 0.0001;

			float s = sqrt(1-c*c);


			glm::quat q = glm::quat(c, -k.x * s, k.y * s, -k.z * s);
			if (select_object == 0)
				q = glm::quat(c, k.x * s, -k.y * s, k.z*s);
			

			mat4 rotation = mat4_cast(q);
			


			if (select_object == 0)
			{
				if (select_ball_frame == 1)
				{
					//skyRBT = aFrame * rotation * glm::inverse(aFrame) * skyRBT;
					skyRBT = rotation * skyRBT;
				}
				else
				{
					//skyRBT = aFrame * rotation * glm::inverse(aFrame) * skyRBT;
					skyRBT = rotation * skyRBT;
				}
			}
			else if (select_object == 1)
			{
				aFrame = glm::translate(glm::mat4(1), glm::vec3(arcballRBT_i[3][0], arcballRBT_i[3][1], arcballRBT_i[3][2]));
				//g_objectRbt[0] = aFrame * rotation * glm::inverse(aFrame) * g_objectRbt[0];
				g_objectRbt[0] = aFrame * rotation * glm::inverse(aFrame) * g_objectRbt[0];
				//g_objectRbt[0] = rotation * g_objectRbt[0];
			}
			else if (select_object == 2)
			{
				aFrame = glm::translate(glm::mat4(1), glm::vec3(arcballRBT_i[3][0], arcballRBT_i[3][1], arcballRBT_i[3][2]));
				g_objectRbt[1] = aFrame * rotation * glm::inverse(aFrame) * g_objectRbt[1];
			}
		}
		break;
	case 2:
		if (select_frame == 0 				
			|| select_object == 1 && select_frame == 2
			|| select_object == 2 && select_frame == 1)
		{
			if (select_object == 0)
			{
				if (select_ball_frame == 1)
				{
					//skyRBT = glm::translate(glm::mat4(1.0f), glm::vec3(xpos - click_xpos, ypos - click_ypos, 0.0)) * skyRBT;
					skyRBT = aFrame * glm::translate(glm::mat4(1.0f), arcBallScale * glm::vec3(xpos - mouse_xpos, -(ypos - mouse_ypos), 0.0)) * glm::inverse(aFrame) * skyRBT;
				}
				else
				{
					skyRBT = aFrame * glm::translate(glm::mat4(1.0f), arcBallScale * glm::vec3(-(xpos - mouse_xpos), (ypos - mouse_ypos), 0.0)) * glm::inverse(aFrame) * skyRBT;
				}
			}
			else if (select_object == 1)
			{
				//g_objectRbt[0] = glm::translate(glm::mat4(1.0f), glm::vec3(xpos - click_xpos, ypos - click_ypos, 0.0)) * g_objectRbt[0];
				g_objectRbt[0] = aFrame * glm::translate(glm::mat4(1.0f), arcBallScale * glm::vec3(xpos - mouse_xpos, -(ypos - mouse_ypos), 0.0)) * glm::inverse(aFrame) * g_objectRbt[0];
			}
			else if (select_object == 2)
			{
				//g_objectRbt[1] = glm::translate(glm::mat4(1.0f), glm::vec3(xpos - click_xpos, ypos - click_ypos, 0.0)) * g_objectRbt[1];
				g_objectRbt[1] = aFrame * glm::translate(glm::mat4(1.0f), arcBallScale * glm::vec3(xpos - mouse_xpos, -(ypos - mouse_ypos), 0.0)) * glm::inverse(aFrame) * g_objectRbt[1];
			}
		}
		break;
	case 3:
		if ((select_frame == 0
			|| select_object == 1 && select_frame == 2
			|| select_object == 2 && select_frame == 1))
		{
			if (select_object == 0)
			{
				if (select_ball_frame == 1)
				{
					//skyRBT = glm::translate(glm::mat4(1.0f), glm::vec3(xpos - click_xpos, ypos - click_ypos, 0.0)) * skyRBT;
					skyRBT = aFrame * glm::translate(glm::mat4(1.0f), arcBallScale * glm::vec3(0.0, 0.0, (ypos - mouse_ypos))) * glm::inverse(aFrame) * skyRBT;
				}
				else
				{
					skyRBT = aFrame * glm::translate(glm::mat4(1.0f), arcBallScale * glm::vec3(0.0, 0.0, -(ypos - mouse_ypos))) * glm::inverse(aFrame) * skyRBT;

				}
			}
			else if (select_object == 1)
			{
				if (select_mode == 0)
				{
					//g_objectRbt[0] = glm::translate(glm::mat4(1.0f), glm::vec3(xpos - click_xpos, ypos - click_ypos, 0.0)) * g_objectRbt[0];
					g_objectRbt[0] = aFrame * glm::translate(glm::mat4(1.0f), arcBallScale * glm::vec3(0.0, 0.0, (ypos - mouse_ypos))) * glm::inverse(aFrame) * g_objectRbt[0];
				}

				else
				{
					g_objectRbt_i[0] = aFrame * glm::translate(glm::mat4(1.0f), arcBallScale * glm::vec3(0.0, 0.0, (ypos - mouse_ypos))) * glm::inverse(aFrame) * g_objectRbt_i[0];
				}
			}
			else if (select_object == 2)
			{
				if (select_mode == 1)
				{
					//g_objectRbt[1] = glm::translate(glm::mat4(1.0f), glm::vec3(xpos - click_xpos, ypos - click_ypos, 0.0)) * g_objectRbt[1];
					g_objectRbt[1] = aFrame * glm::translate(glm::mat4(1.0f), arcBallScale * glm::vec3(0.0, 0.0, (ypos - mouse_ypos))) * glm::inverse(aFrame) * g_objectRbt[1];
				}
				else
				{
					g_objectRbt_i[1] = aFrame * glm::translate(glm::mat4(1.0f), arcBallScale * glm::vec3(0.0, 0.0, (ypos - mouse_ypos))) * glm::inverse(aFrame) * g_objectRbt_i[1];
				}
			}
			
		}
		if (!(select_object == 0) && (select_mode == 1) &&
			(!(select_object == 1 && select_frame == 1
				|| select_object == 2 && select_frame == 2)))
		{
			if (select_object == 1)
			{
				g_objectRbt[0] = g_objectRbt_i[0] * glm::scale(glm::vec3(1.0 + (xpos - click_xpos) / (2*windowWidth)));
			}
			else if (select_object == 2)
			{
				g_objectRbt[1] = g_objectRbt_i[1] * glm::scale(glm::vec3(1.0 + (xpos - click_xpos) / (2*windowWidth)));
			}
		}
		break;
	default:
		break;
	}
	mouse_xpos = xpos;
	mouse_ypos = ypos;
}

static void keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		glm::mat4 m = glm::mat4(1.0f);
		switch (key)
		{
		case GLFW_KEY_H:
			std::cout << "CS380 Homework Assignment 2" << std::endl;
			std::cout << "keymaps:" << std::endl;
			std::cout << "h\t\t Help command" << std::endl;
			std::cout << "v\t\t Change eye frame (your viewpoint)" << std::endl;
			std::cout << "o\t\t Change current manipulating object" << std::endl;
			std::cout << "m\t\t Change auxiliary frame between world-sky and sky-sky" << std::endl;
			std::cout << "c\t\t Change manipulation method" << std::endl;
			break;
		case GLFW_KEY_V:
			// TODO: Change viewpoint
			select_frame = (select_frame + 1) % number_of_frames;
			break;
		case GLFW_KEY_O:
			// TODO: Change manipulating object
			select_object = (select_object + 1) % number_of_objects;
			break;
		case GLFW_KEY_M:
			// TODO: Change auxiliary frame between world-sky and sky-sky
			select_ball_frame = (select_ball_frame + 1) % number_of_ball_frame;
			break;
		case GLFW_KEY_C:
			// TODO: Add an additional manipulation method
			select_mode = (select_mode + 1) % number_of_mode;
			break;
		default:
			break;
		}
		switch (select_frame)
		{
		case 0:
			skyRBT = skyRBT * m;
			break;
		case 1:
			g_objectRbt[0] = g_objectRbt[0] * m;
			break;
		case 2:
			g_objectRbt[1] = g_objectRbt[1] * m;
			break;
		default:
			break;
		}
	}
}

static void get_aFrame(void)
{
	glm::mat4 O_t, E_r;
	O_t = transFact(arcballRBT_i);
	E_r = linearFact(eyeRBT);
	aFrame = O_t * E_r;
}

double compute_z(float x, float y) {
	double square_z;
	double z;
	glm::mat4 temp = arcballRBT_i;

	glm::vec2 center = eye_to_screen(glm::vec3(glm::inverse(eyeRBT) * arcballRBT_i[3]), Projection, windowWidth, windowHeight);
	//glm::vec2 center = eye_to_screen(glm::vec3(arcballRBT_i[3]), Projection, windowWidth, windowHeight);
	center.y = windowHeight - center.y;

	square_z = arcBallScreenRadius * arcBallScreenRadius - (x - center.x) * (x - center.x) - (y - center.y) * (y - center.y);
	if (square_z < 0) {
		z = 0.0;
	}
	else {
		z = sqrt(square_z);

	}
	return z;
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
	window = glfwCreateWindow((int)windowWidth, (int)windowHeight, "Homework 2", NULL, NULL);
	if (window == NULL) {
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = (GLboolean) true; // Needed for core profile
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
	// Update arcBallScreenRadius with framebuffer size
	arcBallScreenRadius = 0.25f * min((float) frameBufferWidth, (float) frameBufferHeight); // for the initial assignment

	// Clear with sky color
	glClearColor((GLclampf)(128. / 255.), (GLclampf)(200. / 255.), (GLclampf)(255. / 255.), (GLclampf) 0.);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);
	// Enable culling
	glEnable(GL_CULL_FACE);
	// Backface culling
	glCullFace(GL_BACK);

	Projection = glm::perspective(fov, ((float) frameBufferWidth / (float) frameBufferHeight), 0.1f, 100.0f);
	skyRBT = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.25, 4.0));

	// initial eye frame = sky frame;
	eyeRBT = skyRBT;

	// Initialize Ground Model
	ground = Model();
	init_ground(ground);
	ground.initialize(DRAW_TYPE::ARRAY, "VertexShader.glsl", "FragmentShader.glsl"); //
	ground.set_projection(&Projection);
	ground.set_eye(&eyeRBT);
	glm::mat4 groundRBT = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, g_groundY, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(g_groundSize, 1.0f, g_groundSize));
	ground.set_model(&groundRBT);

	redCube = Model();
	init_cube(redCube, glm::vec3(1.0, 0.0, 0.0));
	redCube.initialize(DRAW_TYPE::ARRAY, "VertexShader.glsl", "FragmentShader.glsl");

	redCube.set_projection(&Projection);
	redCube.set_eye(&eyeRBT);
	redCube.set_model(&g_objectRbt[0]);

	greenCube = Model();
	init_cube(greenCube, glm::vec3(0.0, 1.0, 0.0));
	greenCube.initialize(DRAW_TYPE::ARRAY, "VertexShader.glsl", "FragmentShader.glsl");

	greenCube.set_projection(&Projection);
	greenCube.set_eye(&eyeRBT);
	greenCube.set_model(&g_objectRbt[1]);

	// TODO: Initialize arcBall
	// Initialize your arcBall with DRAW_TYPE::INDEX (it uses GL_ELEMENT_ARRAY_BUFFER to draw sphere)
	init_sphere(arcBall);
	arcBall.initialize(DRAW_TYPE::INDEX, "VertexShader.glsl", "FragmentShader.glsl");
	arcBall.set_projection(&Projection);
	arcBall.set_eye(&eyeRBT);
	arcBall.set_model(&arcballRBT);


	// Setting Light Vectors
	glm::vec3 lightVec = glm::vec3(0.0f, 1.0f, 0.0f);
	lightLocGround = glGetUniformLocation(ground.GLSLProgramID, "uLight");
	glUniform3f(lightLocGround, lightVec.x, lightVec.y, lightVec.z);

	lightLocRed = glGetUniformLocation(redCube.GLSLProgramID, "uLight");
	glUniform3f(lightLocRed, lightVec.x, lightVec.y, lightVec.z);

	lightLocGreen = glGetUniformLocation(greenCube.GLSLProgramID, "uLight");
	glUniform3f(lightLocGreen, lightVec.x, lightVec.y, lightVec.z);

	lightLocArc = glGetUniformLocation(arcBall.GLSLProgramID, "uLight");
	glUniform3f(lightLocArc, lightVec.x, lightVec.y, lightVec.z);

	do {
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		// TODO: Change Viewpoint with respect to your current view index
		eyeRBT = (select_frame == 0) ? skyRBT : (select_frame == 1) ? g_objectRbt[0] : g_objectRbt[1];
		//arcballRBT_i[3] = (select_object == 0) ? skyRBT[3] : (select_object == 1) ? g_objectRbt[0][3] : g_objectRbt[1][3];
		arcballRBT_i = (select_object == 0) ? skyRBT : (select_object == 1) ? g_objectRbt[0] : g_objectRbt[1];
		if (select_object == 0)
		{
			if (select_ball_frame == 1)
			{
				//arcballRBT_i[3] = worldRBT[3];
				//arcballRBT_i = worldRBT;
				arcballRBT_i = worldRBT;
			}
		}
		get_aFrame();

		redCube.draw();
		greenCube.draw();
		ground.draw();
		// TODO: Draw wireframe of arcball with dynamic radius
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // draw wireframe
		
		update_fovy();
		if (mouse_mode == 0)
		{
			arcBallScale = compute_screen_eye_scale((glm::inverse(eyeRBT) * (arcballRBT_i[3]))[2], fovy, int(windowHeight));
			if (select_object == 0)
				arcBallScale = compute_screen_eye_scale((glm::inverse(eyeRBT) * worldRBT[3])[2], fovy, int(windowHeight));
		}
		arcballRBT = arcballRBT_i * glm::scale(glm::vec3(arcBallScreenRadius * arcBallScale));
		arcBall.draw();
		
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // draw filled models again

		// Swap buffers (Double buffering)
		glfwSwapBuffers(window);
		glfwPollEvents();
	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	// Clean up data structures and glsl objects
	ground.cleanup();
	redCube.cleanup();
	greenCube.cleanup();
	arcBall.cleanup();

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}
