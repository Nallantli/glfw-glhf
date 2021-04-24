#define HEIGHT_MULTIPLIER	2.0f
#define ARIDITY_MULTIPLIER	4.0f
#define INLAND_LAKE_SIZE	64
#define ISLAND_BRANCHING_SIZE	10

#define GLFW_INCLUDE_NONE
#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1
#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <eigen3/Eigen/Dense>

#include "perlin.hpp"

#include "quickhull/QuickHull.hpp"

#include<iostream>
#include<cmath>
#include<vector>
#include<chrono>
#include<map>
#include<cfloat>
#include<fstream>
#include<map>

#include "mesh.h"

bool iterate_rivers(const std::vector<face_t *> &faces);

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

//g++ main.cpp -lglfw3 -lopengl32

camera *cam = new camera(180, 90, 1);

std::map<int, bool> KEYS;

static void error_callback(int error, const char *description)
{
	std::cerr << "Error: " << description << "\n";
}

enum Mode
{
	MADE_WIRE,
	MODE_FLAT,
	MODE_DATA,
	MODE_FOEHN
};

Mode mode = MADE_WIRE;
face_t *curr = NULL;

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
		glVertex2f(t(0) / 2.0f, t(1));
	}
	Eigen::Matrix<float, 2, 1> t = cam->rot * (rot_x(cam->pit) * (rot_y(cam->yaw) * c_point(b, 1).coords));
	glVertex2f(t(0) / 2.0f, t(1));
	glEnd();
}

face_t *find_closest(const std::vector<face_t *> &faces, const float &yaw, const float &pit)
{
	float d = 100.0f;
	face_t *curr = NULL;

	for (auto &f : faces) {
		auto center = f->get_center();

		float t = std::sqrt(std::pow(center[0] - yaw, 2) + std::pow(center[1] - pit, 2));
		if (t < d) {
			curr = f;
			d = t;
		}
	}

	return curr;
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
	glVertex2f(t1(0) / 2.0f, t1(1));
	glVertex2f(t2(0) / 2.0f, t2(1));
	glVertex2f(t3(0) / 2.0f, t3(1));
	glVertex2f(t1(0) / 2.0f, t1(1));
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
	glVertex2f(t1(0) / 2.0f, t1(1));
	glVertex2f(t2(0) / 2.0f, t2(1));
	glVertex2f(t3(0) / 2.0f, t3(1));
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

const float scale(const float &pit)
{
	return 1.0 / std::sin((M_PI * pit) / 180.0f);
}

const float true_dist(const face_t *a, const face_t *b)
{
	s_point ca{ a->get_center() };
	s_point cb{ b->get_center() };
	c_point cca(ca, 1);
	c_point ccb(cb, 1);
	auto x = cca.coords.dot(ccb.coords);
	return std::acos(x);
}

const bool does_share_side(const face_t *a, const face_t *b)
{
	std::vector<s_point> vertices = {
		a->a,
		a->b,
		a->c,
		b->a,
		b->b,
		b->c
	};

	if (std::count(vertices.begin(), vertices.end(), a->a) >= 2 && std::count(vertices.begin(), vertices.end(), a->b) >= 2)
		return true;
	if (std::count(vertices.begin(), vertices.end(), a->b) >= 2 && std::count(vertices.begin(), vertices.end(), a->c) >= 2)
		return true;
	if (std::count(vertices.begin(), vertices.end(), a->c) >= 2 && std::count(vertices.begin(), vertices.end(), a->a) >= 2)
		return true;
	return false;
}

void iterate_land(face_t *curr, int w)
{
	if (curr->type == face_t::FACE_LAND)
		return;
	curr->type = face_t::FACE_LAND;
	if (w > 0) {
		for (auto &n : curr->neighbors) {
			if (n->type == face_t::FACE_WATER)
				iterate_land(n, w - 1);
		}
	}
}

std::vector<face_t *> get_water_extent(face_t *f, const std::vector<face_t *> &faces)
{
	std::vector<face_t *> closed;
	std::vector<face_t *> open = { f };

	while (!open.empty()) {
		face_t *curr = open.back();
		open.erase(std::remove(open.begin(), open.end(), curr), open.end());
		closed.push_back(curr);
		for (auto &n : curr->neighbors) {
			if (n->type == face_t::FACE_WATER
				&& std::find(closed.begin(), closed.end(), n) == closed.end()
				&& std::find(open.begin(), open.end(), n) == open.end())
				open.push_back(n);
		}
	}

	return closed;
}

std::pair<face_t *, float> find_nearest_lake(face_t *f, const std::vector<face_t *> &faces)
{
	if (f->type == face_t::FACE_INLAND_LAKE)
		return { f, 0 };
	face_t *min = NULL;
	float d;
	for (auto &s : faces) {
		if (s->type != face_t::FACE_INLAND_LAKE)
			continue;
		float t = true_dist(f, s);
		if (min == NULL || t < d) {
			d = t;
			min = s;
		}
	}
	return { min, d };
}

std::pair<face_t *, float> find_nearest_land(face_t *f, const std::vector<face_t *> &faces)
{
	if (f->type == face_t::FACE_LAND)
		return { f, 0 };
	face_t *min = NULL;
	float d;
	for (auto &s : faces) {
		if (s->type != face_t::FACE_LAND)
			continue;
		float t = true_dist(f, s);
		if (min == NULL || t < d) {
			d = t;
			min = s;
		}
	}
	return { min, d };
}

std::pair<face_t *, float> find_nearest_ocean(face_t *f, const std::vector<face_t *> &faces)
{
	if (f->type == face_t::FACE_WATER)
		return { f, 0 };
	face_t *min = NULL;
	float d;
	for (auto &s : faces) {
		if (s->type != face_t::FACE_WATER)
			continue;
		float t = true_dist(f, s);
		if (min == NULL || t < d) {
			d = t;
			min = s;
		}
	}
	return { min, d };
}

std::vector<face_t *> set;

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	switch (action) {
		case GLFW_PRESS:
			KEYS[key] = true;

			if (key == GLFW_KEY_Z) {
				curr = curr->neighbors[0];
			}
			if (key == GLFW_KEY_X) {
				curr = curr->neighbors[1];
			}
			if (key == GLFW_KEY_C) {
				curr = curr->neighbors[2];
			}
			if (key == GLFW_KEY_SPACE) {
				iterate_rivers(set);
			}
			break;
		case GLFW_RELEASE:
			KEYS[key] = false;
			break;
	}
}

face_t *get_lowest_neighbor(face_t *f)
{
	face_t *lowest = NULL;
	for (auto &n : f->neighbors) {
		if (n->type != face_t::FACE_LAND)
			continue;
		if (lowest == NULL) {
			if (n->height < f->height)
				lowest = n;
		} else if (n->height < lowest->height)
			lowest = n;
	}
	return lowest;
}

face_t *get_highest_neighbor(face_t *f)
{
	face_t *highest = NULL;
	for (auto &n : f->neighbors) {
		if (n->type != face_t::FACE_LAND)
			continue;
		if (highest == NULL) {
			if (n->height > f->height)
				highest = n;
		} else if (n->height > highest->height)
			highest = n;
	}
	return highest;
}

bool borders_ocean(const face_t *f)
{
	for (auto &n : f->neighbors) {
		if (n->type == face_t::FACE_OCEAN)
			return true;
	}
	return false;
}

bool sees_ocean(const float basin_height, face_t *curr, std::vector<face_t *> &explored)
{
	if (curr->type == face_t::FACE_OCEAN)
		return true;

	explored.push_back(curr);
	for (auto &n : curr->neighbors) {
		if (std::find(explored.begin(), explored.end(), n) == explored.end() && n->height < basin_height && sees_ocean(basin_height, n, explored))
			return true;
	}
	return false;
}

void stagnate_lake(const float basin_height, face_t *curr)
{
	curr->type = face_t::FACE_STAGNANT;
	for (auto &n : curr->neighbors) {
		if (n->height < basin_height && n->type == face_t::FACE_LAND)
			stagnate_lake(basin_height, n);
	}
}

std::vector<face_t *> get_lake_edges(face_t *curr, std::vector<face_t *> &explored)
{
	explored.push_back(curr);
	std::vector<face_t *> edges;
	bool flag = false;
	for (auto &n : curr->neighbors) {
		if (n->type == face_t::FACE_LAND) {
			flag = true;
		} else if (std::find(explored.begin(), explored.end(), n) == explored.end() && n->type == face_t::FACE_STAGNANT) {
			auto ex2 = get_lake_edges(n, explored);
			for (auto &e : ex2)
				edges.push_back(e);
		}
	}
	if (flag)
		edges.push_back(curr);
	return edges;
}

bool iterate_rivers(const std::vector<face_t *> &faces)
{
	std::vector<face_t *> rivers;
	for (auto &f : faces) {
		if (f->type == face_t::FACE_FLOWING)
			rivers.push_back(f);
	}

	if (rivers.empty())
		return false;

	for (auto &s : rivers) {
		if (borders_ocean(s)) {
			s->type = face_t::FACE_INLAND_LAKE;
		} else {
			auto ln = get_lowest_neighbor(s);
			if (ln != NULL) {
				std::vector<face_t *> ex;
				s->type = face_t::FACE_INLAND_LAKE;
				if (!sees_ocean(ln->height, ln, ex)) {
					ex.clear();
					stagnate_lake(ln->height, ln);
					std::vector<face_t *> possible = get_lake_edges(ln, ex);
					for (auto &e : possible) {
						auto out = get_highest_neighbor(e);
						if (out != NULL)
							out->type = face_t::FACE_FLOWING;
					}
				} else if (ln->height < s->height) {
					ln->type = face_t::FACE_FLOWING;
				}
			} else {
				s->type = face_t::FACE_INLAND_LAKE;
			}
		}
	}
	return true;
}

void propagate_wind_east(face_t *f, float p_factor, const float start_y, std::vector<face_t *> &explored)
{
	explored.push_back(f);
	f->foehn += p_factor;
	p_factor -= 0.5;
	if (p_factor < 0)
		return;
	for (auto &n : f->neighbors) {
		if (
			(n->get_center()[0] > f->get_center()[0] && std::abs(n->get_center()[0] - f->get_center()[0]) <= 10) || (n->get_center()[0] < f->get_center()[0] && std::abs(n->get_center()[0] - f->get_center()[0]) > 10)
			&& n->height <= f->height && n->type == face_t::FACE_LAND && std::find(explored.begin(), explored.end(), n) == explored.end()
			) {
			propagate_wind_east(
				n,
				p_factor * MAX(0, -std::sqrt(std::abs(n->get_center()[1] - start_y) / 10.0f) + 1.0f),
				start_y,
				explored
			);
		}
	}
}

void propagate_wind_west(face_t *f, float p_factor, const float start_y, std::vector<face_t *> &explored)
{
	explored.push_back(f);
	f->foehn += p_factor;
	p_factor -= 0.5;
	if (p_factor < 0)
		return;
	for (auto &n : f->neighbors) {
		if (
			(n->get_center()[0] < f->get_center()[0] && std::abs(n->get_center()[0] - f->get_center()[0]) <= 10) || (n->get_center()[0] > f->get_center()[0] && std::abs(n->get_center()[0] - f->get_center()[0]) > 10)
			&& n->height <= f->height && n->type == face_t::FACE_LAND && std::find(explored.begin(), explored.end(), n) == explored.end()
			) {
			propagate_wind_west(
				n,
				p_factor * MAX(0, -std::sqrt(std::abs(n->get_center()[1] - start_y) / 10.0f) + 1.0f),
				start_y,
				explored
			);
		}
	}
}

void set_foehn(const std::vector<face_t *> &set)
{
	for (auto &f : set) {
		if (f->type == face_t::FACE_LAND) {
			float w_factor = clamp(-1, 1, std::pow(DSIN(3.0f * (f->get_center()[1] - 90.0f)), 2) / DCOS(3.0f * (f->get_center()[1] - 90.0f))) / 2.0f;
			float h_factor = f->height;
			float p_factor = w_factor * h_factor;
			std::vector<face_t *> explored;
			if (p_factor > 0) {
				propagate_wind_east(f, p_factor, f->get_center()[1], explored);
			} else {
				propagate_wind_east(f, -p_factor, f->get_center()[1], explored);
			}
		}
	}
}

int main()
{
	std::vector<s_point> ps;
	srand(time(0));
	glfwSetErrorCallback(error_callback);
	if (!glfwInit())
		return 1;

	GLFWwindow *window = glfwCreateWindow(2560, 1440, "My Title", NULL, NULL);
	if (!window) {
		return 1;
	}

	glfwMakeContextCurrent(window);

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	glfwSetKeyCallback(window, key_callback);

	float size = 2.0f;

	for (int i = size; i <= 180 - size; i += size) {
		for (float j = size; j < 360;) {
			float x = j + ((float)rand() / (float)RAND_MAX) * (size / 2.0f);
			float y = (float)i + ((float)rand() / (float)RAND_MAX) * (size / 2.0f);
			ps.push_back(s_point(x, y));
			j += scale(i) * size;
		}
	}

	ps.push_back(s_point(0, 0));
	ps.push_back(s_point(0, 180));

	quickhull::QuickHull<float> qh;
	std::vector<quickhull::Vector3<float>> qhpoints;

	for (auto &p : ps) {
		c_point c(p, 1);
		qhpoints.push_back({
			c[0],
			c[1],
			c[2]
			});
	}

	std::cout << "Setting triangles...\n";
	auto hull = qh.getConvexHull(qhpoints, true, false);
	const auto &indexBuffer = hull.getIndexBuffer();
	const auto &vertexBuffer = hull.getVertexBuffer();

	qhpoints.clear();
	ps.clear();

	std::vector<s_point> translated_vertices;

	for (auto &v : vertexBuffer) {
		float r = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
		translated_vertices.push_back(
			s_point(180.0f * std::atan2(v.y, v.x) / M_PI + 180.0f, 180.0f * std::asin(v.z / r) / M_PI + 90.0f)
		);
	}

	for (int i = 0; i < indexBuffer.size(); i += 3) {
		set.push_back(
			new face_t{
			translated_vertices[indexBuffer[i]],
			translated_vertices[indexBuffer[i + 1]],
			translated_vertices[indexBuffer[i + 2]]
			}
		);
	}

	std::cout << "Setting neighbors...\n";
	translated_vertices.clear();
	for (auto &f : set) {
		for (auto &f2 : set) {
			if (f == f2)
				continue;
			if (std::find(f->neighbors.begin(), f->neighbors.end(), f2) != f->neighbors.end())
				continue;
			if (does_share_side(f, f2)) {
				f->neighbors.push_back(f2);
				f2->neighbors.push_back(f);
			}
		}
	}

	/*std::ifstream file("cities500.txt");
	if (!file.is_open())
		return 1;

	std::string line;
	size_t c = 0;
	while (std::getline(file, line)) {
		auto data = split(line, '\t');
		ps.push_back(s_point(std::stof(data[5]) + 180.0f, std::stof(data[4]) + 90.0f));
		c++;
	}

	file.close();

	std::cout << "Cites: " << c << "\n";*/

	std::cout << "Setting Islands...\n";
	for (auto i = 0; i < 16;i++) {
		auto origin = set[rand() % set.size()];
		float size = ((float)rand() / (float)RAND_MAX) * 0.5f;
		for (auto &e : set) {
			if (true_dist(origin, e) < size)
				iterate_land(e, ISLAND_BRANCHING_SIZE);
		}
	}

	std::cout << "Setting Height Map...\n";
	for (auto &f : set) {
		if (f->type != face_t::FACE_LAND)
			continue;
		std::pair<face_t *, float> n = find_nearest_ocean(f, set);
		c_point cc = f->get_center_c();
		float pm =
			perlin2d(perlin2d((cc[0] + 1.0f) / 32.0f, (cc[1] + 1.0f) / 32.0f, 2.0f, 4), (cc[2] + 1.0f) / 32.0f, 2.0f, 4) +
			perlin2d(perlin2d((cc[0] + 1.0f) / 16.0f, (cc[1] + 1.0f) / 16.0f, 4.0f, 4), (cc[2] + 1.0f) / 16.0f, 4.0f, 4) +
			perlin2d(perlin2d((cc[0] + 1.0f) / 8.0f, (cc[1] + 1.0f) / 8.0f, 8.0f, 4), (cc[2] + 1.0f) / 8.0f, 8.0f, 4) +
			perlin2d(perlin2d((cc[0] + 1.0f) / 4.0f, (cc[1] + 1.0f) / 4.0f, 16.0f, 4), (cc[2] + 1.0f) / 4.0f, 16.0f, 4);
		f->height = (n.second + MAX(0, pm / 4.0f - 0.25f)) * HEIGHT_MULTIPLIER;
	}

	std::cout << "Setting Water Types...\n";
	for (auto &f : set) {
		if (f->type != face_t::FACE_WATER)
			continue;
		auto water = get_water_extent(f, set);
		if (water.size() < INLAND_LAKE_SIZE) {
			for (auto &e : water) {
				e->height = find_nearest_land(e, set).first->height;
				e->type = face_t::FACE_LAND;
			}
		} else {
			for (auto &e : water) {
				e->type = face_t::FACE_OCEAN;
			}
		}
	}

	std::cout << "Setting Springs...\n";

	for (auto &f : set) {
		if (f->type != face_t::FACE_LAND || f->height > 0.4f && f->height < 0.5f) {
			continue;
		}
		if (rand() % 64 == 0)
			f->type = face_t::FACE_FLOWING;
	}

	std::cout << "Setting Rivers...\n";
	//iterate_rivers(set);
	while (iterate_rivers(set));

	for (auto &f : set) {
		if (f->type == face_t::FACE_STAGNANT)
			f->type = face_t::FACE_INLAND_LAKE;
	}

	std::cout << "Setting Aridity Map...\n";
	for (auto &f : set) {
		if (f->type != face_t::FACE_LAND)
			continue;
		std::pair<face_t *, float> n = find_nearest_lake(f, set);
		c_point cc = f->get_center_c();
		f->aridity = n.second * ARIDITY_MULTIPLIER;
	}

	std::cout << "Setting Foehn Map...\n";

	set_foehn(set);

	std::cout << "Done.\n";

	curr = set.back();

	double time = glfwGetTime();
	while (!glfwWindowShouldClose(window)) {
		double nt = glfwGetTime();
		double lapse = nt - time;
		time = nt;
		glClear(GL_COLOR_BUFFER_BIT);
		for (auto &s : set) {
			switch (s->type) {
				case face_t::FACE_OCEAN:
					glColor3f(0.0f, 0.0f, 0.5f);
					break;
				case face_t::FACE_FLOWING:
					glColor3f(1.0f, 0.0f, 1.0f);
					break;
				case face_t::FACE_STAGNANT:
					glColor3f(0.5f, 0.5f, 1.0f);
					break;
				case face_t::FACE_INLAND_LAKE:
					glColor3f(0.0, 0.0, 0.5f);
					break;
				case face_t::FACE_LAND: {
					switch (mode) 					{
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
					break;
				}
			}
			fill_shape(s, cam);
			switch (mode) {
				case MADE_WIRE: {
					glColor3f(1.0f, 1.0f, 1.0f); // Red
					draw_shape(s, cam);
					break;
				}
				case MODE_FLAT: {
					break;
				}
			}
			//auto f = find_closest(set, -cam->yaw + 90.0f, cam->pit);
			if (curr != NULL) {
				glColor3f(0.0f, 0.0f, 1.0f); // Red
				draw_shape(curr->neighbors[0], cam);
				glColor3f(0.0f, 1.0f, 1.0f); // Red
				draw_shape(curr->neighbors[1], cam);
				glColor3f(0.0f, 1.0f, 0.0f); // Red
				draw_shape(curr->neighbors[2], cam);

				glColor3f(1.0f, 0.0f, 0.0f); // Red
				draw_shape(curr, cam);
			}
		}

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
			mode = MADE_WIRE;
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

		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	glfwDestroyWindow(window);

	glfwTerminate();

	delete cam;
	for (auto &e : set)
		delete e;
	set.clear();

	return 0;
}