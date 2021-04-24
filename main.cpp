#define GLFW_INCLUDE_NONE
#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1
#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <eigen3/Eigen/Dense>
#include<iostream>
#include<cmath>
#include<vector>
#include<chrono>
#include<map>
#include<cfloat>
#include<fstream>
#include<map>

#include "mesh.h"
#include "gen.h"

std::map<int, bool> KEYS;

enum Mode
{
	MODE_WIRE,
	MODE_FLAT,
	MODE_DATA,
	MODE_FOEHN
};

Mode mode = MODE_WIRE;

const Eigen::Matrix<float, 3, 3> rot_x(const float &theta)
{
	Eigen::Matrix<float, 3, 3> m;
	m <<
		1, 0, 0,
		0, DCOS(theta), -DSIN(theta),
		0, DSIN(theta), DCOS(theta);
	return m;
}

const Eigen::Matrix<float, 3, 3> rot_y(const float &theta)
{
	Eigen::Matrix<float, 3, 3> m;
	m <<
		DCOS(theta), -DSIN(theta), 0,
		DSIN(theta), DCOS(theta), 0,
		0, 0, 1;
	return m;
}

struct camera
{
	float yaw;
	float pit;
	Eigen::Matrix<float, 2, 3> rot;

	camera(const float &yaw, const float &pit, const float &dist);
};

camera::camera(const float &yaw, const float &pit, const float &dist) : yaw{ yaw }, pit{ pit }
{
	rot(0, 0) = dist;
	rot(1, 1) = dist;
}

static void error_callback(int error, const char *description)
{
	std::cerr << "Error: " << description << "\n";
}

void draw_secant_line(const s_point &a, const s_point &b, camera *cam)
{
	/*float z_a = flatten(a, cam, r_a);
		float z_b = flatten(b, cam, r_b);

		float dp = r_a.x * r_b.x + r_a.y * r_b.y + z_a * z_b;

		float theta = std::acos(dp / (cam->dist * cam->dist));*/

	float length = std::sqrt((a[0] - b[0]) * (a[0] - b[0]) + (a[1] - b[1]) * (a[1] - b[1]));

	float dist_yaw = b[0] - a[0];

	if (std::abs(dist_yaw) > std::abs(b[0] - (a[0] - 360.0f)))
		dist_yaw = b[0] - (a[0] - 360.0f);
	if (std::abs(dist_yaw) > std::abs((b[0] - 360.0f) - a[0]))
		dist_yaw = (b[0] - 360.0f) - a[0];

	float delta_yaw = dist_yaw / length;
	float delta_pit = (b[1] - a[1]) / length;

	glBegin(GL_LINE_STRIP);
	for (float i = 0.0f; i < length; i++) {
		s_point p(a[0] + delta_yaw * i, a[1] + delta_pit * i);
		Eigen::Matrix<float, 2, 1> t = cam->rot * (rot_x(cam->pit) * (rot_y(cam->yaw) * c_point(p, 1).coords));
		glVertex2f(t(0) / RES_RATIO, t(1));
	}
	Eigen::Matrix<float, 2, 1> t = cam->rot * (rot_x(cam->pit) * (rot_y(cam->yaw) * c_point(b, 1).coords));
	glVertex2f(t(0) / RES_RATIO, t(1));
	glEnd();
}

void draw_shape(const face_t *s, camera *cam)
{
	Eigen::Matrix<float, 3, 1> r1 = rot_x(cam->pit) * (rot_y(cam->yaw) * c_point(s->a, 1).coords);
	Eigen::Matrix<float, 2, 1> t1 = cam->rot * r1;
	Eigen::Matrix<float, 3, 1> r2 = rot_x(cam->pit) * (rot_y(cam->yaw) * c_point(s->b, 1).coords);
	Eigen::Matrix<float, 2, 1> t2 = cam->rot * r2;
	Eigen::Matrix<float, 3, 1> r3 = rot_x(cam->pit) * (rot_y(cam->yaw) * c_point(s->c, 1).coords);
	Eigen::Matrix<float, 2, 1> t3 = cam->rot * r3;

	if (r1(2) < 0 || r2(2) < 0 || r3(2) < 0)
		return;

	glBegin(GL_LINE_STRIP);
	glVertex2f(t1(0) / RES_RATIO, t1(1));
	glVertex2f(t2(0) / RES_RATIO, t2(1));
	glVertex2f(t3(0) / RES_RATIO, t3(1));
	glVertex2f(t1(0) / RES_RATIO, t1(1));
	glEnd();
}

void fill_shape(const face_t *s, camera *cam)
{
	Eigen::Matrix<float, 3, 1> r1 = rot_x(cam->pit) * (rot_y(cam->yaw) * c_point(s->a, 1).coords);
	Eigen::Matrix<float, 2, 1> t1 = cam->rot * r1;
	Eigen::Matrix<float, 3, 1> r2 = rot_x(cam->pit) * (rot_y(cam->yaw) * c_point(s->b, 1).coords);
	Eigen::Matrix<float, 2, 1> t2 = cam->rot * r2;
	Eigen::Matrix<float, 3, 1> r3 = rot_x(cam->pit) * (rot_y(cam->yaw) * c_point(s->c, 1).coords);
	Eigen::Matrix<float, 2, 1> t3 = cam->rot * r3;

	if (r1(2) < 0 || r2(2) < 0 || r3(2) < 0)
		return;

	glBegin(GL_TRIANGLES);
	glVertex2f(t1(0) / RES_RATIO, t1(1));
	glVertex2f(t2(0) / RES_RATIO, t2(1));
	glVertex2f(t3(0) / RES_RATIO, t3(1));
	glEnd();
}

std::vector<std::string> split(const std::string &s, char delimiter)
{
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(s);
	while (std::getline(tokenStream, token, delimiter)) {
		tokens.push_back(token);
	}
	return tokens;
}

std::vector<face_t *> set;

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	switch (action) {
		case GLFW_PRESS:
			KEYS[key] = true;
			break;
		case GLFW_RELEASE:
			KEYS[key] = false;
			break;
	}
}

void render_world(GLFWwindow *window, camera *cam)
{
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(0.0f, 0.0f, 0.5f);
	glBegin(GL_TRIANGLE_FAN); //BEGIN CIRCLE
	glVertex2f(0, 0); // center of circle
	for (int i = 0; i <= 90; i++) {
		glVertex2f(
			cam->rot(0, 0) * cos(i * 2.0f * M_PI / 90.0f) / RES_RATIO, cam->rot(0, 0) * sin(i * 2.0f * M_PI / 90.0f)
		);
	}
	glEnd(); //END

	for (auto &s : set) {
		switch (s->type) {
			case face_t::FACE_OCEAN:
				//glColor3f(0.0f, 0.0f, 0.5f);
				//break;
			case face_t::FACE_INLAND_LAKE:
				//glColor3f(0.0, 0.0, 0.5f);
				break;
			case face_t::FACE_FLOWING:
				//glColor3f(1.0f, 0.0f, 1.0f);
				//break;
			case face_t::FACE_STAGNANT:
				//glColor3f(0.5f, 0.5f, 1.0f);
				break;
			case face_t::FACE_LAND: {
				switch (mode) {
					case MODE_FOEHN:
						glColor3f(s->foehn, s->foehn, s->foehn);
						break;
					case MODE_DATA:
						glColor3f(s->aridity, s->height, 0.0f);
						break;
					case MODE_FLAT: {
						auto biome = s->get_biome();
						glColor3f(biome.r, biome.g, biome.b);
						break;
					}
				}
				fill_shape(s, cam);
				break;
			}
		}
		switch (mode) {
			case MODE_WIRE: {
				glColor3f(1.0f, 1.0f, 1.0f); // Red
				draw_shape(s, cam);
				break;
			}
			case MODE_FLAT: {
				break;
			}
		}
	}
	glfwPollEvents();
	glfwSwapBuffers(window);
}

void user_input(double &time, camera *cam)
{
	double nt = glfwGetTime();
	double lapse = nt - time;
	time = nt;
	if (KEYS[GLFW_KEY_A])
		cam->yaw = std::fmod(cam->yaw + lapse * 10.0f + 360.0f, 360);
	if (KEYS[GLFW_KEY_D])
		cam->yaw = std::fmod(cam->yaw - lapse * 10.0f + 360.0f, 360);

	if (KEYS[GLFW_KEY_S])
		cam->pit = cam->pit + lapse * 10.0f;
	if (KEYS[GLFW_KEY_W])
		cam->pit = cam->pit - lapse * 10.0f;

	if (cam->pit < 0)
		cam->pit = 0;
	if (cam->pit > 180)
		cam->pit = 180;

	if (KEYS[GLFW_KEY_Q]) {
		cam->rot(0, 0) += lapse;
		cam->rot(1, 1) += lapse;
	}
	if (KEYS[GLFW_KEY_E]) {
		cam->rot(0, 0) -= lapse;
		cam->rot(1, 1) -= lapse;
	}

	if (KEYS[GLFW_KEY_1]) {
		mode = MODE_WIRE;
	}
	if (KEYS[GLFW_KEY_2]) {
		mode = MODE_FLAT;
	}
	if (KEYS[GLFW_KEY_3]) {
		mode = MODE_DATA;
	}
	if (KEYS[GLFW_KEY_4]) {
		mode = MODE_FOEHN;
	}
}

int main()
{
	camera *cam = new camera(180, 90, 1);
	srand(time(0));
	glfwSetErrorCallback(error_callback);
	if (!glfwInit())
		return 1;

	GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "My Title", NULL, NULL);
	if (!window) {
		return 1;
	}

	glfwMakeContextCurrent(window);

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	glfwSetKeyCallback(window, key_callback);

	generate_world(set);

	double time = glfwGetTime();
	while (!glfwWindowShouldClose(window)) {
		render_world(window, cam);
		user_input(time, cam);
	}

	glfwDestroyWindow(window);

	glfwTerminate();

	delete cam;
	for (auto &e : set)
		delete e;
	set.clear();

	return 0;
}