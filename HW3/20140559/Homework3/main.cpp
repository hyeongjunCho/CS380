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
#include <common/picking.hpp>

#define PI 3.14159265

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
//Model ground, redCube, greenCube;
Model ground;
glm::mat4 skyRBT;
//glm::mat4 g_objectRbt[2] = { glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, 0.5f, -2.0f)) * glm::rotate(glm::mat4(1.0f), -90.0f, glm::vec3(0.0f, 1.0f, 0.0f)), // RBT for redCube
//							glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, -2.0f)) * glm::rotate(glm::mat4(1.0f), 90.0f, glm::vec3(0.0f, 1.0f, 0.0f))}; // RBT for greenCube
glm::mat4 eyeRBT;
glm::mat4 worldRBT = glm::mat4(1.0f);
glm::mat4 aFrame;

// Arcball manipulation
Model arcBall;
glm::mat4 arcballRBT = glm::mat4(1.0f);
float arcBallScreenRadius = 0.25f * min(windowWidth, windowHeight); // for the initial assignment
float screenToEyeScale = 0.01f;
float arcBallScale = 0.01f;
glm::mat4 arcballRBT_i = arcballRBT;

// Function definition
static void window_size_callback(GLFWwindow*, int, int);
static void mouse_button_callback(GLFWwindow*, int, int, int);
static void cursor_pos_callback(GLFWwindow*, double, double);
static void keyboard_callback(GLFWwindow*, int, int, int, int);
void update_fovy(void);

// mouse
int current_button = 0; // 1: left, 2: right
int target = 0;
double prev_xpos;
double prev_ypos;

int mouse_mode = 0;	// 0: pick or no click, 1: nopick and left, 2: nopick and right, 3: nopick and middle

// keyboard
int picking = 0;	// 0: no, 1: yes


// Floppy cube
void* f;
//


double compute_z(float, float);

class Floppy {
	// TODO LIST
	// Rotation information
	// Layout of Floppy Cube (parent-children)
	// Current selected nodes
	// Conditional check whether your selection is valid or not
	// etc

	// Rotation: 4가지
	/*
	 * 1 2 3
	 * 4 5 6
	 * 7 8 9
	 *
	 * cubebase * objectrbt
	 *
	 * Left button
	 * 2
	 *   1
	 *   3
	 * 8
	 *   7
	 *   9
	 * Right button
	 * 4
	 *   1
	 *   7
	 * 6
	 *   3
	 *   9
	 */

	double rotations[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
	int selected_rotation = 0;	// 1, 2, 3,   4, 5, 6
	int prev_rotation = 0;	// 1: left button, 2: right button

	


	Model cube[9] = {};
	GLint lightLoc[9] = {};
	glm::vec3 colors[6] = {glm::vec3(1.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0), glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0), glm::vec3(1.0, 0.5, 0.0), glm::vec3(1.0, 1.0, 1.0)};
	glm::vec3 black = glm::vec3(0.0, 0.0, 0.0);
	glm::vec3 cube_color[9][6] = {
		{ black, colors[3], colors[4], colors[0], black, colors[5] },
		{ black, colors[3], black, colors[0], black, colors[5] },
		{ black, colors[3], black, colors[0], colors[2], colors[5] },
		{ black, black, colors[4], colors[0], black, colors[5] },
		{ black, black, black, colors[0], black, colors[5] },
		{ black, black, black, colors[0], colors[2], colors[5] },
		{ colors[1], black, colors[4], colors[0], black, colors[5] },
		{ colors[1], black, black, colors[0], black, colors[5] },
		{ colors[1], black, black, colors[0], colors[2], colors[5] }
	};
/*	glm::vec3 cube_color[9][6] = {
		{ colors[4], black, colors[3], colors[0], black, colors[5] },
		{ black, black, colors[3], colors[0], black, colors[5] },
		{ black, colors[2], colors[3], colors[0], black, colors[5] },
		{ colors[4], black, black, colors[0], black, colors[5] },
		{ black, black, black, colors[0], black, colors[5] },
		{ black, colors[2], black, colors[0], black, colors[5] },
		{ colors[4], black, black, colors[0], colors[1], colors[5] },
		{ black, black, black, colors[0], colors[1], colors[5] },
		{ black, colors[2], black, colors[0], colors[1], colors[5] }
	};*/
	/*
	ffff00
	0000ff ff0000 00ff00 ff8000
	ffffff
	// 왼 오 뒤 위 앞 아래
	*/
	// 앞 뒤 왼 위 오 아래
	/*
	cube 배치
	1 2 3	(-0.22, 0.0, 0.22), 
	4 5 6
	7 8 9
	*/
public:

	glm::mat4 cubebaseRbt = glm::translate(glm::mat4(1.0f), glm::vec3(-1.02f, 0.0f, 1.02f));
	glm::vec3 cubeposition[9] = { glm::vec3(-1.0f, 0.0f, -1.0f),
		glm::vec3(0.0f, 0.0f, -1.0f),
		glm::vec3(1.0f, 0.0f, -1.0f),
		glm::vec3(-1.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(-1.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(1.0f, 0.0f, 1.0f)
	};

	glm::mat4 cubeobjectRbt[9] = { glm::translate(glm::mat4(1.0f), cubeposition[0]),
		glm::translate(glm::mat4(1.0f), cubeposition[1]),
		glm::translate(glm::mat4(1.0f), cubeposition[2]),
		glm::translate(glm::mat4(1.0f), cubeposition[3]),
		glm::translate(glm::mat4(1.0f), cubeposition[4]),
		glm::translate(glm::mat4(1.0f), cubeposition[5]),
		glm::translate(glm::mat4(1.0f), cubeposition[6]),
		glm::translate(glm::mat4(1.0f), cubeposition[7]),
		glm::translate(glm::mat4(1.0f), cubeposition[8])
	};

	glm::mat4 cubeRbt[9] = { cubebaseRbt * cubeobjectRbt[0],	// a = wOA, A: translate O by cubeposition[0]
		cubebaseRbt * cubeobjectRbt[1],
		cubebaseRbt * cubeobjectRbt[2],
		cubebaseRbt * cubeobjectRbt[3],
		cubebaseRbt * cubeobjectRbt[4],
		cubebaseRbt * cubeobjectRbt[5],
		cubebaseRbt * cubeobjectRbt[6],
		cubebaseRbt * cubeobjectRbt[7],
		cubebaseRbt * cubeobjectRbt[8]
	};
	

	int selected_node = 0;	// 1, 2, 3, 4, 5, 6, 7, 8, 9
	int cube_curr[9] = { 0,1,2,3,4,5,6,7,8 };
	glm::mat4* top[3] = { &cubeobjectRbt[0], &cubeobjectRbt[1], &cubeobjectRbt[2] };
	glm::mat4* left[3] = { &cubeobjectRbt[0], &cubeobjectRbt[3], &cubeobjectRbt[6] };
	glm::mat4* bottom[3] = { &cubeobjectRbt[6], &cubeobjectRbt[7], &cubeobjectRbt[8] };
	glm::mat4* right[3] = { &cubeobjectRbt[2], &cubeobjectRbt[5], &cubeobjectRbt[8] };

	glm::vec3 axis[4];
	Floppy();
	void Floppy_drawPicking();
	void Floppy_draw();
	bool Floppy_rotation_0_(int i, int j) //가생이, 중앙
	{
		if (fabs(rotations[j] - rotations[i]) < 10.0 || fabs(rotations[j] - rotations[i]) > 350.0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	bool Floppy_rotation_180_(int i, int j)
	{
		if (fabs(rotations[j] - rotations[i]) < 190.0 &&
			fabs(rotations[j] - rotations[i]) > 170.0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	
	bool Floppy_valid()
	{
		if (current_button == 1)	// left
		{
			if ((Floppy_rotation_0_(0,4) || Floppy_rotation_180_(0,4)) &&
				(Floppy_rotation_0_(2,4) || Floppy_rotation_180_(2,4)))
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		else if (current_button == 2)	// right
		{
			if ((Floppy_rotation_0_(1,5) || Floppy_rotation_180_(1,5)) &&
				(Floppy_rotation_0_(3,5) || Floppy_rotation_180_(3,5)))
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		return false;
	}

	
	void Floppy_set_flat()
	{
		double subs;
		int temp;
		glm::mat4 objectrbt;
		int i = 0;
		int j = 0;
		// rotations의 원소들을 0.0 or 180.0으로 맞춰줌
		if (current_button == 1)
		{
			if (Floppy_rotation_0_(0, 4))
			{
				for (auto rbt : top)
				{
					i = 0;
					j = 0;
					objectrbt = (*rbt);
					for (i = 0; i < 4; i++)
					{
						for (j = 0; j < 4; j++)
						{
							printf("%f ", objectrbt[i][j]);
							if (objectrbt[i][j] > 0.8) objectrbt[i][j] = 1.0;
							else if (objectrbt[i][j] < -0.8) objectrbt[i][j] = -1.0;
							else if (objectrbt[i][j] > -0.2 && objectrbt[i][j] < 0.2) objectrbt[i][j] = 0.0;
						}
						printf("\n");
					}
					*rbt = objectrbt;
				}
			}
			else if (Floppy_rotation_180_(0, 4))
			{
				i = 0;
				for (auto rbt : top)
				{
					i = 0;
					j = 0;
					objectrbt = (*rbt);
					for (i = 0; i < 4; i++)
					{
						for (j = 0; j < 4; j++)
						{
							if (objectrbt[i][j] > 0.8) objectrbt[i][j] = 1.0;
							else if (objectrbt[i][j] < -0.8) objectrbt[i][j] = -1.0;
							else if (objectrbt[i][j] > -0.2 && objectrbt[i][j] < 0.2) objectrbt[i][j] = 0.0;
						}
					}
					*rbt = objectrbt;
				}
				temp = cube_curr[0];
				cube_curr[0] = cube_curr[2];
				cube_curr[2] = temp;
			}
		}

		else if (current_button == 2)
		{
			if (Floppy_rotation_0_(1, 5))
			{
				for (auto rbt : left)
				{
					i = 0;
					j = 0;
					objectrbt = (*rbt);
					for (i = 0; i < 4; i++)
					{
						for (j = 0; j < 4; j++)
						{
							if (objectrbt[i][j] > 0.8) objectrbt[i][j] = 1.0;
							else if (objectrbt[i][j] < -0.8) objectrbt[i][j] = -1.0;
							else if (objectrbt[i][j] > -0.2 && objectrbt[i][j] < 0.2) objectrbt[i][j] = 0.0;
						}
					}
					*rbt = objectrbt;
				}
			}
			else if (Floppy_rotation_180_(1, 5))
			{
				for (auto rbt : left)
				{
					i = 0;
					j = 0;
					objectrbt = (*rbt);
					for (i = 0; i < 4; i++)
					{
						for (j = 0; j < 4; j++)
						{
							if (objectrbt[i][j] > 0.8) objectrbt[i][j] = 1.0;
							else if (objectrbt[i][j] < -0.8) objectrbt[i][j] = -1.0;
							else if (objectrbt[i][j] > -0.2 && objectrbt[i][j] < 0.2) objectrbt[i][j] = 0.0;
						}
					}
					*rbt = objectrbt;
				}

				temp = cube_curr[0];
				cube_curr[0] = cube_curr[6];
				cube_curr[6] = temp;
			}
		}

		if (current_button == 1)
		{
			if (Floppy_rotation_0_(2, 4))
			{
				for (auto rbt : bottom)
				{
					i = 0;
					j = 0;
					objectrbt = (*rbt);
					for (i = 0; i < 4; i++)
					{
						for (j = 0; j < 4; j++)
						{
							if (objectrbt[i][j] > 0.8) objectrbt[i][j] = 1.0;
							else if (objectrbt[i][j] < -0.8) objectrbt[i][j] = -1.0;
							else if (objectrbt[i][j] > -0.2 && objectrbt[i][j] < 0.2) objectrbt[i][j] = 0.0;
						}
					}
					*rbt = objectrbt;
				}
			}
			else if (Floppy_rotation_180_(2, 4))
			{
				for (auto rbt : bottom)
				{
					i = 0;
					j = 0;
					objectrbt = (*rbt);
					for (i = 0; i < 4; i++)
					{
						for (j = 0; j < 4; j++)
						{
							if (objectrbt[i][j] > 0.8) objectrbt[i][j] = 1.0;
							else if (objectrbt[i][j] < -0.8) objectrbt[i][j] = -1.0;
							else if (objectrbt[i][j] > -0.2 && objectrbt[i][j] < 0.2) objectrbt[i][j] = 0.0;
						}
					}
					*rbt = objectrbt;
				}

				temp = cube_curr[6];
				cube_curr[6] = cube_curr[8];
				cube_curr[8] = temp;
			}
		}

		else if (current_button == 2)
		{
			if (Floppy_rotation_0_(3, 5))
			{
				for (auto rbt : right)
				{
					i = 0;
					j = 0;
					objectrbt = (*rbt);
					for (i = 0; i < 4; i++)
					{
						for (j = 0; j < 4; j++)
						{
							if (objectrbt[i][j] > 0.8) objectrbt[i][j] = 1.0;
							else if (objectrbt[i][j] < -0.8) objectrbt[i][j] = -1.0;
							else if (objectrbt[i][j] > -0.2 && objectrbt[i][j] < 0.2) objectrbt[i][j] = 0.0;
						}
					}
					*rbt = objectrbt;
				}
			}
			else if (Floppy_rotation_180_(3, 5))
			{
				for (auto rbt : right)
				{
					i = 0;
					j = 0;
					objectrbt = (*rbt);
					for (i = 0; i < 4; i++)
					{
						for (j = 0; j < 4; j++)
						{
							if (objectrbt[i][j] > 0.8) objectrbt[i][j] = 1.0;
							else if (objectrbt[i][j] < -0.8) objectrbt[i][j] = -1.0;
							else if (objectrbt[i][j] > -0.2 && objectrbt[i][j] < 0.2) objectrbt[i][j] = 0.0;
						}
					}
					*rbt = objectrbt;
				}
				temp = cube_curr[2];
				cube_curr[2] = cube_curr[8];
				cube_curr[8] = temp;
			}
			
		}
		Floppy_set_current_layout();
		for (i = 0; i < 6; i++)
		{
			rotations[i] = 0.0;
		}
	}
	void Floppy_set_current_layout();
	void Floppy_rotation(double xpos, double ypos);
	void Floppy_rotation_all(double xpos, double ypos);
	void Floppy_translation_xy(double xpos, double ypos);
	void Floppy_translation_z(double ypos);
};

Floppy::Floppy()
{
	int i = 0;
	glm::vec3 lightVec = glm::vec3(0.0f, 1.0f, 0.0f);
	cubebaseRbt = cubebaseRbt * glm::scale(glm::vec3(0.2));
	aFrame = transFact(cubebaseRbt) * linearFact(eyeRBT);
	cubebaseRbt = aFrame * glm::rotate(glm::mat4(1.0f), 45.0f, glm::vec3(1.0f, 0.0f, 0.0f)) * glm::inverse(aFrame) * cubebaseRbt;

	for (i = 0; i < 9; i++)
	{
		init_rubic(cube[i], cube_color[i]);
		cube[i].initialize(DRAW_TYPE::ARRAY, "VertexShader.glsl", "FragmentShader.glsl");
		cube[i].initialize_picking("PickingVertexShader.glsl", "PickingFragmentShader.glsl");

		cube[i].set_projection(&Projection);
		cube[i].set_eye(&eyeRBT);
		cube[i].set_model(&cubeRbt[i]);

		cube[i].objectID = i+1;

		lightLoc[i] = glGetUniformLocation(cube[i].GLSLProgramID, "uLight");
		glUniform3f(lightLoc[i], lightVec.x, lightVec.y, lightVec.z);

		cubeRbt[i] = cubebaseRbt * cubeobjectRbt[i];
	}

	axis[0] = glm::vec3(1.0f, 0.0f, 0.0f);	// x
	axis[1] = glm::vec3(-1.0f, 0.0f, 0.0f);	// -x
	axis[2] = glm::vec3(0.0f, 0.0f, 1.0f);	// z
	axis[3] = glm::vec3(0.0f, 0.0f, -1.0f);	// -z
}

void Floppy::Floppy_drawPicking()
{
	int i = 0;
	for (i = 0; i < 9; i++)
	{
		cube[i].drawPicking();
	}
}

void Floppy::Floppy_draw()
{
	int i = 0;
	for (i = 0; i < 9; i++)
	{
		cube[i].draw();
	}
}

void Floppy::Floppy_set_current_layout()
{
	top[0] = &cubeobjectRbt[cube_curr[0]];
	top[1] = &cubeobjectRbt[cube_curr[1]];
	top[2] = &cubeobjectRbt[cube_curr[2]];
	left[0] = &cubeobjectRbt[cube_curr[0]];
	left[1] = &cubeobjectRbt[cube_curr[3]];
	left[2] = &cubeobjectRbt[cube_curr[6]];
	right[0] = &cubeobjectRbt[cube_curr[2]];
	right[1] = &cubeobjectRbt[cube_curr[5]];
	right[2] = &cubeobjectRbt[cube_curr[8]];
	bottom[0] = &cubeobjectRbt[cube_curr[6]];
	bottom[1] = &cubeobjectRbt[cube_curr[7]];
	bottom[2] = &cubeobjectRbt[cube_curr[8]];
}

void Floppy::Floppy_rotation(double xpos, double ypos)
{
	if (!Floppy_valid()) return;
	if (target < 1 || target > 9) return;
	selected_node = target;
	if (current_button != prev_rotation)
	{
		Floppy_set_flat();
		prev_rotation = current_button;
	}
	double theta = 0.0;
	double cos2Theta;
	glm::mat4 rotation, objectrbt;
	glm::quat q;
	glm::vec3 k;
	double c, s;
	printf("%f,%f,%f,%f,%f,%f\n", rotations[0], rotations[1], rotations[2], rotations[3], rotations[4], rotations[5]);
	printf("%d,%d,%d,%d,%d,%d,%d,%d,%d\n", cube_curr[0], cube_curr[1], cube_curr[2], cube_curr[3], cube_curr[4], cube_curr[5], cube_curr[6], cube_curr[7], cube_curr[8]);
	int i = 0;

	if (current_button == 1)	// left
	{
		if (selected_node == cube_curr[0] + 1 || selected_node == cube_curr[3] + 1 || selected_node == cube_curr[6] + 1)	//left
		{
			glm::vec2 center = eye_to_screen(glm::vec3(glm::inverse(eyeRBT) * (cubeRbt[3][3])), Projection, windowWidth, windowHeight);
			center.y = windowHeight - center.y;

			glm::vec3 start = glm::normalize(glm::vec3(prev_xpos - center.x, prev_ypos - center.y, 0.0f));
			glm::vec3 dest = glm::normalize(glm::vec3(xpos - center.x, ypos - center.y, 0.0f));

			cos2Theta = glm::dot(start, dest);

			if (cos2Theta > 1.0f - 0.00000001)
			{
				k = glm::vec3(0.0, 0.0, 1.0);
				cos2Theta = 1.0f - 0.00000001;
			}
			else if (cos2Theta < -1.0f + 0.00000001)
			{
				k = glm::vec3(0.0, 0.0, -1.0);
				cos2Theta = -1.0f + 0.00000001;
			}
			else k = glm::normalize(glm::cross(start, dest));

			float c = sqrt((1 + cos2Theta) * 2) / 2.0;
			if (c > 1.0f - 0.000001) c = 1.0f - 0.000001;

			float s = sqrt(1 - c*c);

			if (k.z < -0.9)
			{
				theta = acos(cos2Theta) * 180.0 / PI;
				k = axis[0];
			}
			else if (k.z > 0.9)
			{
				theta = -acos(cos2Theta) * 180.0 / PI;
				k = axis[1];
			}
			else
			{
				return;
			}
			
			q = glm::quat(c, k.x * s, k.y * s, k.z * s);
			rotation = mat4_cast(q);

			rotations[1] = rotations[1] + theta;
			if (rotations[1] > 360.0f)
			{
				rotations[1] = rotations[1] - 360.0f;
			}
			else if (rotations[1] < 0.0f)
			{
				rotations[1] = rotations[1] + 360.0f;
			}

			for (auto rbt : left)
			{
				*rbt = rotation * *rbt;
			}
		}
		else if (selected_node == cube_curr[2] + 1 || selected_node == cube_curr[5] + 1 || selected_node == cube_curr[8] + 1)	//right
		{
			glm::vec2 center = eye_to_screen(glm::vec3(glm::inverse(eyeRBT) * (cubeRbt[5][3])), Projection, windowWidth, windowHeight);
			center.y = windowHeight - center.y;
			
			glm::vec3 start = glm::normalize(glm::vec3(prev_xpos - center.x, prev_ypos - center.y, 0.0f));
			glm::vec3 dest = glm::normalize(glm::vec3(xpos - center.x, ypos - center.y, 0.0f));

			cos2Theta = glm::dot(start, dest);

			if (cos2Theta > 1.0f - 0.00000001)
			{
				k = glm::vec3(0.0, 0.0, 1.0);
				cos2Theta = 1.0f - 0.00000001;
			}
			else if (cos2Theta < -1.0f + 0.00000001)
			{
				k = glm::vec3(0.0, 0.0, -1.0);
				cos2Theta = -1.0f + 0.00000001;
			}
			else k = glm::normalize(glm::cross(start, dest));

			float c = sqrt((1 + cos2Theta) * 2) / 2.0;
			if (c > 1.0f - 0.000001) c = 1.0f - 0.000001;

			float s = sqrt(1 - c*c);

			if (k.z < -0.9)
			{
				theta = acos(cos2Theta) * 180.0 / PI;
				k = axis[0];
			}
			else if (k.z > 0.9)
			{
				theta = -acos(cos2Theta) * 180.0 / PI;
				k = axis[1];
			}
			else
			{
				return;
			}

			q = glm::quat(c, k.x * s, k.y * s, k.z * s);
			rotation = mat4_cast(q);

			rotations[3] = rotations[3] + theta;
			if (rotations[3] > 360.0f)
			{
				rotations[3] = rotations[3] - 360.0f;
			}
			else if (rotations[3] < 0.0f)
			{
				rotations[3] = rotations[3] + 360.0f;
			}

			for (auto rbt : right)
			{
				*rbt = rotation * *rbt;
			}
		}

		else if (selected_node == cube_curr[1] + 1 || selected_node == cube_curr[4] + 1 || selected_node == cube_curr[7] + 1)
		{
			glm::vec2 center = eye_to_screen(glm::vec3(glm::inverse(eyeRBT) * (cubeRbt[4][3])), Projection, windowWidth, windowHeight);
			center.y = windowHeight - center.y;

			glm::vec3 start = glm::normalize(glm::vec3(prev_xpos - center.x, prev_ypos - center.y, 0.0f));
			glm::vec3 dest = glm::normalize(glm::vec3(xpos - center.x, ypos - center.y, 0.0f));

			cos2Theta = glm::dot(start, dest);

			if (cos2Theta > 1.0f - 0.00000001)
			{
				k = glm::vec3(0.0, 0.0, 1.0);
				cos2Theta = 1.0f - 0.00000001;
			}
			else if (cos2Theta < -1.0f + 0.00000001)
			{
				k = glm::vec3(0.0, 0.0, -1.0);
				cos2Theta = -1.0f + 0.00000001;
			}
			else k = glm::normalize(glm::cross(start, dest));

			float c = sqrt((1 + cos2Theta) * 2) / 2.0;
			if (c > 1.0f - 0.000001) c = 1.0f - 0.000001;

			float s = sqrt(1 - c*c);

			if (k.z < -0.9)
			{
				theta = acos(cos2Theta) * 180.0 / PI;
				k = axis[0];
			}
			else if (k.z > 0.9)
			{
				theta = -acos(cos2Theta) * 180.0 / PI;
				k = axis[1];
			}
			else
			{
				return;
			}

			q = glm::quat(c, k.x * s, k.y * s, k.z * s);
			rotation = mat4_cast(q);
		
			rotations[5] = rotations[5] + theta;
			if (rotations[5] > 360.0)
			{
				rotations[5] = rotations[5] - 360.0;
			}
			else if (rotations[5] < 0.0)
			{
				rotations[5] = rotations[5] + 360.0;
			}
			for (auto rbt : left)
			{
				*rbt = glm::inverse(rotation) * *rbt;
			}
			for (auto rbt : right)
			{
				*rbt = glm::inverse(rotation) * *rbt;
			}

			cubebaseRbt = cubebaseRbt * rotation * cubeobjectRbt[4];

			int i = 0;

		}
		for (i = 0; i < 9; i++)
		{
			cubeRbt[i] = cubebaseRbt * cubeobjectRbt[i];
		}
	}
	else if (current_button == 2)	// right
	{
		if (selected_node == cube_curr[0] + 1 || selected_node == cube_curr[1] + 1 || selected_node == cube_curr[2] + 1)	//top
		{
			glm::vec2 center = eye_to_screen(glm::vec3(glm::inverse(eyeRBT) * (cubeRbt[1][3])), Projection, windowWidth, windowHeight);
			center.y = windowHeight - center.y;

			glm::vec3 start = glm::normalize(glm::vec3(prev_xpos - center.x, prev_ypos - center.y, 0.0f));
			glm::vec3 dest = glm::normalize(glm::vec3(xpos - center.x, ypos - center.y, 0.0f));

			cos2Theta = glm::dot(start, dest);

			if (cos2Theta > 1.0f - 0.00000001)
			{
				k = glm::vec3(0.0, 0.0, 1.0);
				cos2Theta = 1.0f - 0.00000001;
			}
			else if (cos2Theta < -1.0f + 0.00000001)
			{
				k = glm::vec3(0.0, 0.0, -1.0);
				cos2Theta = -1.0f + 0.00000001;
			}
			else k = glm::normalize(glm::cross(start, dest));

			float c = sqrt((1 + cos2Theta) * 2) / 2.0;
			if (c > 1.0f - 0.000001) c = 1.0f - 0.000001;

			float s = sqrt(1 - c*c);

			if (k.z < -0.9)
			{
				theta = acos(cos2Theta) * 180.0 / PI;
				k = axis[2];
			}
			else if (k.z > 0.9)
			{
				theta = -acos(cos2Theta) * 180.0 / PI;
				k = axis[3];
			}
			else
			{
				return;
			}

			int t, l;
			q = glm::quat(c, k.x * s, k.y * s, k.z * s);

			rotation = mat4_cast(q);

			rotations[0] = rotations[0] + theta;
			if (rotations[0] > 360.0f)
			{
				rotations[0] = rotations[0] - 360.0f;
			}
			else if (rotations[0] < 0.0f)
			{
				rotations[0] = rotations[0] + 360.0f;
			}


			for (auto rbt : top)
			{
				*rbt = rotation * *rbt;
			}
		}
		else if (selected_node == cube_curr[6] + 1 || selected_node == cube_curr[7] + 1 || selected_node == cube_curr[8] + 1)	//bottom
		{
			glm::vec2 center = eye_to_screen(glm::vec3(glm::inverse(eyeRBT) * (cubeRbt[7][3])), Projection, windowWidth, windowHeight);
			center.y = windowHeight - center.y;

			glm::vec3 start = glm::normalize(glm::vec3(prev_xpos - center.x, prev_ypos - center.y, 0.0f));
			glm::vec3 dest = glm::normalize(glm::vec3(xpos - center.x, ypos - center.y, 0.0f));

			cos2Theta = glm::dot(start, dest);

			if (cos2Theta > 1.0f - 0.00000001)
			{
				k = glm::vec3(0.0, 0.0, 1.0);
				cos2Theta = 1.0f - 0.00000001;
			}
			else if (cos2Theta < -1.0f + 0.00000001)
			{
				k = glm::vec3(0.0, 0.0, -1.0);
				cos2Theta = -1.0f + 0.00000001;
			}
			else k = glm::normalize(glm::cross(start, dest));

			float c = sqrt((1 + cos2Theta) * 2) / 2.0;
			if (c > 1.0f - 0.000001) c = 1.0f - 0.000001;

			float s = sqrt(1 - c*c);

			if (k.z < -0.9)
			{
				theta = acos(cos2Theta) * 180.0 / PI;
				k = axis[2];
			}
			else if (k.z > 0.9)
			{
				theta = -acos(cos2Theta) * 180.0 / PI;
				k = axis[3];
			}
			else
			{
				return;
			}

			q = glm::quat(c, k.x * s, k.y * s, k.z * s);
			rotation = mat4_cast(q);

			rotations[2] = rotations[2] + theta;
			if (rotations[2] > 360.0f)
			{
				rotations[2] = rotations[2] - 360.0f;
			}
			else if (rotations[2] < 0.0f)
			{
				rotations[2] = rotations[2] + 360.0f;
			}

			for (auto rbt : bottom)
			{
				*rbt = rotation * *rbt;
			}
		}
		else if (selected_node == cube_curr[3] + 1 || selected_node == cube_curr[4] + 1 || selected_node == cube_curr[5] + 1)
		{
			glm::vec2 center = eye_to_screen(glm::vec3(glm::inverse(eyeRBT) * (cubeRbt[4][3])), Projection, windowWidth, windowHeight);
			center.y = windowHeight - center.y;

			glm::vec3 start = glm::normalize(glm::vec3(prev_xpos - center.x, prev_ypos - center.y, 0.0f));
			glm::vec3 dest = glm::normalize(glm::vec3(xpos - center.x, ypos - center.y, 0.0f));

			cos2Theta = glm::dot(start, dest);

			if (cos2Theta > 1.0f - 0.00000001)
			{
				k = glm::vec3(0.0, 0.0, 1.0);
				cos2Theta = 1.0f - 0.00000001;
			}
			else if (cos2Theta < -1.0f + 0.00000001)
			{
				k = glm::vec3(0.0, 0.0, -1.0);
				cos2Theta = -1.0f + 0.00000001;
			}
			else k = glm::normalize(glm::cross(start, dest));

			float c = sqrt((1 + cos2Theta) * 2) / 2.0;
			if (c > 1.0f - 0.000001) c = 1.0f - 0.000001;

			float s = sqrt(1 - c*c);

			if (k.z < -0.9)
			{
				theta = acos(cos2Theta) * 180.0 / PI;
				k = axis[2];
			}
			else if (k.z > 0.9)
			{
				theta = -acos(cos2Theta) * 180.0 / PI;
				k = axis[3];
			}
			else
			{
				return;
			}

			q = glm::quat(c, k.x * s, k.y * s, k.z * s);
			rotation = mat4_cast(q);

			
			rotations[4] = rotations[4] + theta;
			if (rotations[4] > 360.0)
			{
				rotations[4] = rotations[4] - 360.0;
			}
			else if (rotations[4] < 0.0)
			{
				rotations[4] = rotations[4] + 360.0;
			}


			for (auto rbt : top)
			{
				*rbt = glm::inverse(rotation) * *rbt;
			}
			for (auto rbt : bottom)
			{
				*rbt = glm::inverse(rotation) * *rbt;
			}

			cubebaseRbt = cubebaseRbt * rotation * cubeobjectRbt[4];

			int i = 0;


		}
		for (i = 0; i < 9; i++)
		{
			cubeRbt[i] = cubebaseRbt * cubeobjectRbt[i];
		}
	}

}

// Rotation whole by arcball
void Floppy::Floppy_rotation_all(double xpos, double ypos)
{
	int i = 0;
	double theta = 0.0;
	double cos2Theta;
	glm::mat4 rotation;
	glm::quat q;
	glm::vec3 k;
	double c, s;
	double z = 0.0;
	double zpos = 0.0;

	z = compute_z(prev_xpos, prev_ypos);
	zpos = compute_z(xpos, ypos);
	

	glm::vec2 center = eye_to_screen(glm::vec3(glm::inverse(eyeRBT) * (arcballRBT_i[3])), Projection, windowWidth, windowHeight);
	center.y = windowHeight - center.y;

	glm::vec3 start = glm::normalize(glm::vec3(prev_xpos - center.x, prev_ypos - center.y, z));
	glm::vec3 dest = glm::normalize(glm::vec3(xpos - center.x, ypos - center.y, zpos));

	cos2Theta = glm::dot(start, dest);

	if (cos2Theta > 1.0f - 0.00000001)
	{
		k = glm::vec3(0.0, 0.0, 1.0);
		cos2Theta = 1.0f - 0.00000001;
	}
	else if (cos2Theta < -1.0f + 0.00000001)
	{
		k = glm::vec3(0.0, 0.0, -1.0);
		cos2Theta = -1.0f + 0.00000001;
	}
	else k = glm::normalize(glm::cross(start, dest));

	c = sqrt((1 + cos2Theta) * 2) / 2.0;
	if (c > 1.0f - 0.000001) c = 1.0f - 0.000001;

	s = sqrt(1 - c*c);

	q = glm::quat(c, -k.x * s, k.y * s, -k.z * s);
	rotation = mat4_cast(q);
	
	aFrame = transFact(cubebaseRbt) * linearFact(eyeRBT);
	cubebaseRbt = aFrame * rotation * glm::inverse(aFrame) * cubebaseRbt;

	for (i = 0; i < 9; i++)
	{
		cubeRbt[i] = cubebaseRbt * cubeobjectRbt[i];
	}
}

// Transition whole cube by arcball
void Floppy::Floppy_translation_xy(double xpos, double ypos)
{
	int i = 0;
	aFrame = transFact(cubebaseRbt) * linearFact(eyeRBT);
	cubebaseRbt = aFrame * glm::translate(glm::mat4(1.0f), arcBallScale * glm::vec3(xpos - prev_xpos, -(ypos - prev_ypos), 0.0)) * glm::inverse(aFrame) * cubebaseRbt;

	for (i = 0; i < 9; i++)
	{
		cubeRbt[i] = cubebaseRbt * cubeobjectRbt[i];
	}
}

// Transition whole cube by arcball
void Floppy::Floppy_translation_z(double ypos)
{
	int i = 0;
	aFrame = transFact(cubebaseRbt) * linearFact(eyeRBT);
	cubebaseRbt = aFrame * glm::translate(glm::mat4(1.0f), arcBallScale * glm::vec3(0.0, 0.0, -(ypos - prev_ypos))) * glm::inverse(aFrame) * cubebaseRbt;

	for (i = 0; i < 9; i++)
	{
		cubeRbt[i] = cubebaseRbt * cubeobjectRbt[i];
	}
}


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

	// re-allocate textures with respect to new framebuffer width and height
	reallocate_picking_texture(frameBufferWidth, frameBufferHeight);

	// Update projection matrix
	Projection = glm::perspective(fov, ((float) frameBufferWidth / (float) frameBufferHeight), 0.1f, 100.0f);
}

// TODO: Fill up GLFW mouse button callback function
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (picking)
	{
		mouse_mode = 0;
		//example code for picking
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		{
			double xpos, ypos;
			glfwGetCursorPos(window, &xpos, &ypos);
			target = pick((int)xpos, (int)ypos, frameBufferWidth, frameBufferHeight);
			std::cout << "Picked node: " << target << std::endl;
			current_button = 1;
		}
		else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
		{
			double xpos, ypos;
			glfwGetCursorPos(window, &xpos, &ypos);
			target = pick((int)xpos, (int)ypos, frameBufferWidth, frameBufferHeight);
			std::cout << "Picked node: " << target << std::endl;
			current_button = 2;
		}
		else
		{
			target = 0;
			current_button = 0;
		}
	}
	else
	{
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		{
			mouse_mode = 1;
		}
		else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
		{
			mouse_mode = 2;
		}
		else if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS)
		{
			mouse_mode = 3;
		}
		else
			mouse_mode = 0;
	}
}

// TODO: Fill up GLFW cursor position callback function
static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (picking)
	{
		(*(Floppy*)f).Floppy_rotation(xpos, ypos);
	}
	else
	{
		//
		switch (mouse_mode)
		{
		case 0: break;
		case 1:
			int picked;
			picked = pick((int)xpos, (int)ypos, frameBufferWidth, frameBufferHeight);
			(*(Floppy*)f).Floppy_rotation_all(xpos, ypos);
			break;
		case 2:
			(*(Floppy*)f).Floppy_translation_xy(xpos, ypos);
			break;
		case 3:
			(*(Floppy*)f).Floppy_translation_z(ypos);
			break;
		default:
			break;
		}
	}


	prev_xpos = xpos;
	prev_ypos = ypos;
}

static void keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		switch (key)
		{
		case GLFW_KEY_H:
			std::cout << "CS380 Homework Assignment 3" << std::endl;
			std::cout << "keymaps:" << std::endl;
			std::cout << "h\t\t Help command" << std::endl;
			std::cout << "p\t\t Enable/Disable picking" << std::endl;
			break;
		case GLFW_KEY_P:
			// TODO: Enable/Disable picking
			picking = (picking + 1) % 2;
			break;
		default:
			break;
		}
	}
}

double compute_z(float x, float y) {
	double square_z;
	double z;
	glm::mat4 temp = arcballRBT_i;

	glm::vec2 center = eye_to_screen(glm::vec3(glm::inverse(eyeRBT) * arcballRBT_i[3]), Projection, windowWidth, windowHeight);
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
	window = glfwCreateWindow((int)windowWidth, (int)windowHeight, "Homework 3: 20140559 - Hyeongjun Cho ", NULL, NULL);
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

	// Initialize framebuffer object and picking textures
	picking_initialize(frameBufferWidth, frameBufferHeight);

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

	/*
	redCube = Model();
	init_cube(redCube, glm::vec3(1.0, 0.0, 0.0));
	redCube.initialize(DRAW_TYPE::ARRAY, "VertexShader.glsl", "FragmentShader.glsl");
	redCube.initialize_picking("PickingVertexShader.glsl", "PickingFragmentShader.glsl");

	redCube.set_projection(&Projection);
	redCube.set_eye(&eyeRBT);
	redCube.set_model(&g_objectRbt[0]);

	redCube.objectID = 1;

	greenCube = Model();
	init_cube(greenCube, glm::vec3(0.0, 1.0, 0.0));
	greenCube.initialize(DRAW_TYPE::ARRAY, "VertexShader.glsl", "FragmentShader.glsl");
	greenCube.initialize_picking("PickingVertexShader.glsl", "PickingFragmentShader.glsl");
	greenCube.set_projection(&Projection);
	greenCube.set_eye(&eyeRBT);
	greenCube.set_model(&g_objectRbt[1]);

	greenCube.objectID = 2;
	*/
	Floppy floppy = Floppy();
	f = &floppy;
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

	/*
	lightLocRed = glGetUniformLocation(redCube.GLSLProgramID, "uLight");
	glUniform3f(lightLocRed, lightVec.x, lightVec.y, lightVec.z);

	lightLocGreen = glGetUniformLocation(greenCube.GLSLProgramID, "uLight");
	glUniform3f(lightLocGreen, lightVec.x, lightVec.y, lightVec.z);
	*/

	lightLocArc = glGetUniformLocation(arcBall.GLSLProgramID, "uLight");
	glUniform3f(lightLocArc, lightVec.x, lightVec.y, lightVec.z);

	do {
		// first pass: picking shader
		// binding framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, picking_fbo);
		// Background: RGB = 0x000000 => objectID: 0
		glClearColor((GLclampf) 0.0f, (GLclampf) 0.0f, (GLclampf) 0.0f, (GLclampf) 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		if (picking == 0) arcballRBT_i = (*(Floppy*)f).cubebaseRbt;
		else if (target > 0 && target < 10)
		{
			int position, i;
			for (i = 0; i < 9; i++)
			{
				if ((*(Floppy*)f).cube_curr[i] == target-1)
				{
					position = i;
					break;
				}
			}
			if (current_button == 1)
			{
				arcballRBT_i = (*(Floppy*)f).cubeRbt[(*(Floppy*)f).cube_curr[(position % 3) + 3]];
			}
			else if (current_button == 2)
			{
				arcballRBT_i = (*(Floppy*)f).cubeRbt[(*(Floppy*)f).cube_curr[(position / 3) * 3 + 1]];
			}
		}

		// drawing objects in framebuffer (picking process)
		//redCube.drawPicking();
		//greenCube.drawPicking();

		floppy.Floppy_drawPicking();

		// second pass: your drawing
		// unbinding framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor((GLclampf)(128. / 255.), (GLclampf)(200. / 255.), (GLclampf)(255. / 255.), (GLclampf)0.);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//redCube.draw();
		//greenCube.draw();
		ground.draw();

		floppy.Floppy_draw();

		// TODO: Draw wireframe of arcball with dynamic radius
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // draw wireframe

		update_fovy();
		//if (picking == 0 && mouse_mode == 0)
		//{
			arcBallScale = compute_screen_eye_scale((glm::inverse(eyeRBT) * (arcballRBT_i[3]))[2], fovy, int(frameBufferHeight));

		//}

		arcballRBT = arcballRBT_i * glm::scale(glm::vec3(arcBallScreenRadius * arcBallScale / 0.2));

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
	//redCube.cleanup();
	//greenCube.cleanup();
	arcBall.cleanup();

	// Cleanup textures
	delete_picking_resources();

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}
