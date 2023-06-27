#include "engine.h"
#ifdef _WIN32
#include <Windows.h>
#include <ShellScalingAPI.h>
#include <glm/mat3x3.hpp>
#include <comdef.h>
#endif
#include <fstream>
#include <glm/gtx/string_cast.hpp>
#include <bits/stdc++.h>

#include "../FONT.h"

void engine_t::draw_letter(const char &c, const double &size, const double &x, const double &y)
{
	const long long data = FONT_CHAR_MAP.at(c);

	double _x = 0;
	double _y = 0;

	for (long long i = 39LL; i >= 0LL; i--)
	{
		if ((data & (1LL << i)) >> i == 1LL)
		{
			glBegin(GL_QUADS);
			glVertex2d((x + _x * size) / ((double)this->_screenWidth / 2.0) - 1.0, -((y + _y * size) / ((double)this->_screenHeight / 2.0) - 1.0));
			glVertex2d((x + (_x + 1.0) * size) / ((double)this->_screenWidth / 2.0) - 1.0, -((y + _y * size) / ((double)this->_screenHeight / 2.0) - 1.0));
			glVertex2d((x + (_x + 1.0) * size) / ((double)this->_screenWidth / 2.0) - 1.0, -((y + (_y + 1.0) * size) / ((double)this->_screenHeight / 2.0) - 1.0));
			glVertex2d((x + _x * size) / ((double)this->_screenWidth / 2.0) - 1.0, -((y + (_y + 1.0) * size) / ((double)this->_screenHeight / 2.0) - 1.0));
			glEnd();
		}
		_x++;
		if (i % 5LL == 0LL)
		{
			_x = 0;
			_y++;
		}
	}
}

void engine_t::draw_string(const std::string &s, const double &size, const double &x, const double &y)
{
	double _x = 0;
	double _y = 0;
	for (auto &c : s)
	{
		switch (c)
		{
		case '\n':
			_y += size * 9.0;
			_x = 0;
			break;
		case ' ':
			_x += size * 6.0;
			break;
		default:
			draw_letter(c, size, x + _x, y + _y);
			_x += size * 6.0;
			break;
		}
	}
}

/*static std::string load_shader(const std::string &filename)
{
	std::ifstream file(filename);
	std::string line;
	std::string content;

	while (std::getline(file, line)) {
		content += line + "\n";
	}

	return content;
}*/

enum Mode
{
	MODE_LANDMASS,
	MODE_FLAT,
	MODE_HEIGHT,
	MODE_ARIDITY,
	MODE_FOEHN,
	MODE_DATA
};

Mode mode = MODE_FLAT;

const glm::mat3 rot_x(const double &theta)
{
	glm::mat3 m{
		1, 0, 0,
		0, DCOS(theta), -DSIN(theta),
		0, DSIN(theta), DCOS(theta)
	};
	return m;
}

const glm::mat3 rot_y(const double &theta)
{
	glm::mat3 m{
		DCOS(theta), -DSIN(theta), 0,
		DSIN(theta), DCOS(theta), 0,
		0, 0, 1
	};
	return m;
}

surface_t *get_face_from_point(const polar_t &p, const std::vector<surface_t *> &set)
{
	surface_t *curr = NULL;
	double dist = 100.0;
	for (auto &s : set) {
		point3_t cc(p, 1.0);
		auto x = glm::dot(cc.coords, s->get_center_c().coords);
		double d = std::acos(x);
		if (curr == NULL || d < dist) {
			curr = s;
			dist = d;
		}
	}
	return curr;
}

camera::camera(const double &yaw, const double &pit, const double &dist)
	: yaw{ yaw }
	, pit{ pit }
	, rot{
		{dist, 0, 0},
		{0, dist, 0}
}
{}

engine_t::engine_t(const int &seed)
	: _seed(seed)
{
#ifdef _WIN32
	HRESULT hr = SetProcessDPIAware();
	if (FAILED(hr)) {
		_com_error err(hr);
		fwprintf(stderr, L"SetProcessDPIAware: %s\n", err.ErrorMessage());
	}
#endif

	_window = nullptr;
	_screenWidth = 1600;
	_screenHeight = 900;
	_resRatio = (double)_screenWidth / (double)_screenHeight;
	_cam = new camera(270, 90, .8);
	_windowState = windowState::RUN;
	_fpsMax = 120;
}

engine_t::~engine_t()
{
	delete world;
	delete _cam;
}

void engine_t::run()
{
	init_engine();
}

void engine_t::init_engine()
{
	srand(_seed);
	world = new world_t(_seed);
	std::cout << "World generated with seed: " << _seed << std::endl;

	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_DisplayMode DM;
	if (0 == SDL_GetCurrentDisplayMode(0, &DM)) {
		_screenWidth = DM.w * 0.8;
		_screenHeight = DM.w * 0.4;
		_resRatio = (double)_screenWidth / (double)_screenHeight;
	}
	_window = SDL_CreateWindow("Planet Display", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, _screenWidth, _screenHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI);
	if (_window == nullptr) {
		std::cerr << "SDL_window could not be created.";
	}

	glContext = SDL_GL_CreateContext(_window);
	if (_window == nullptr) {
		std::cerr << "SDL_GL context could not be created.";
	}

	GLenum error = glewInit();
	if (error != GLEW_OK) {
		std::cerr << "Glew init failed.";
	}

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	engine_loop();
}

/* -- UNUSED -- */
void engine_t::draw_secant_line(const polar_t &a, const polar_t &b)
{
	double length = std::sqrt((a[0] - b[0]) * (a[0] - b[0]) + (a[1] - b[1]) * (a[1] - b[1]));

	double dist_yaw = b[0] - a[0];

	if (std::abs(dist_yaw) > std::abs(b[0] - (a[0] - 360.0)))
		dist_yaw = b[0] - (a[0] - 360.0);
	if (std::abs(dist_yaw) > std::abs((b[0] - 360.0) - a[0]))
		dist_yaw = (b[0] - 360.0) - a[0];

	double delta_yaw = dist_yaw / length;
	double delta_pit = (b[1] - a[1]) / length;

	glBegin(GL_LINE_STRIP);
	for (double i = 0.0; i < length; i++) {
		polar_t p(a[0] + delta_yaw * i, a[1] + delta_pit * i);
		glm::vec2 t = _cam->rot * (rot_x(_cam->pit) * (rot_y(_cam->yaw) * point3_t(p, 1).coords));
		glVertex2d(t[0] / _resRatio, t[1]);
	}
	glm::vec2 t = _cam->rot * (rot_x(_cam->pit) * (rot_y(_cam->yaw) * point3_t(b, 1).coords));
	glVertex2d(t[0] / _resRatio, t[1]);
	glEnd();
}

void engine_t::draw_shape(const surface_t *s)
{
	// generate transformed lon+lat coords into 3d cartesian points, then project
	auto r1 = rot_x(_cam->pit) * (rot_y(_cam->yaw) * point3_t(s->a, 1).coords);
	auto t1 = _cam->rot * r1;
	auto r2 = rot_x(_cam->pit) * (rot_y(_cam->yaw) * point3_t(s->b, 1).coords);
	auto t2 = _cam->rot * r2;
	auto r3 = rot_x(_cam->pit) * (rot_y(_cam->yaw) * point3_t(s->c, 1).coords);
	auto t3 = _cam->rot * r3;

	// don't draw if opposite side of sphere
	if (r1[2] < 0 || r2[2] < 0 || r3[2] < 0)
		return;

	glVertex2d(t1[0] / _resRatio, t1[1]);
	glVertex2d(t2[0] / _resRatio, t2[1]);
	glVertex2d(t3[0] / _resRatio, t3[1]);
}

void engine_t::render_world()
{
	// clear screen
	glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glColor3ub(255, 255, 255);
	draw_string("Pitch: " + std::to_string(_cam->pit) + "\nYaw: " + std::to_string(_cam->yaw), 5, 10, 10);

	// set ocean color
	glColor3ub(26, 26, 102);

	switch (projection) {
		case PROJ_MERC:
		{
			// draw ocean rectangle
			glBegin(GL_QUADS);
			glVertex2d(-1, -1);
			glVertex2d(1, -1);
			glVertex2d(1, 1);
			glVertex2d(-1, 1);
			glEnd();
			for (auto &s : world->get_faces()) {
				if (s->type != surface_t::FACE_LAND)
					continue;
				switch (mode) {
					case MODE_LANDMASS:
						glColor3d(s->landmass->r, s->landmass->g, s->landmass->b);
						break;
					case MODE_FLAT: {
						auto biome = s->get_biome();
						glColor3ub(biome.r, biome.g, biome.b);
						break;
					}
					case MODE_ARIDITY:
						glColor3d(s->aridity - 2.0, 1.0 - std::abs(s->aridity - 2.0), 1.0 - std::abs(s->aridity - 1.0));
						break;
					case MODE_HEIGHT:
						glColor3d(s->height - 2.0, 1.0 - std::abs(s->height - 2.0), 1.0 - std::abs(s->height - 1.0));
						break;
					case MODE_FOEHN:
						glColor3d(s->foehn - 2.0, 1.0 - std::abs(s->foehn - 2.0), 1.0 - std::abs(s->foehn - 1.0));
						break;
					case MODE_DATA:
						glColor3d(s->aridity, s->height, s->foehn);
						break;
				}
				glBegin(GL_TRIANGLES);
				// if the points are clockwise (determinant > 0), the triangle does not need to be translated
				if (s->b[0] * s->a[1] + s->c[0] * s->b[1] + s->a[0] * s->c[1] > s->a[0] * s->b[1] + s->b[0] * s->c[1] + s->c[0] * s->a[1]) {
					glVertex2d(s->a[0] / 180.0 - 1.0, -s->a[1] / 90.0 + 1.0);
					glVertex2d(s->b[0] / 180.0 - 1.0, -s->b[1] / 90.0 + 1.0);
					glVertex2d(s->c[0] / 180.0 - 1.0, -s->c[1] / 90.0 + 1.0);
				} else {
					if (s->b[0] < s->get_center()[0]) {
						glVertex2d((s->a[0] + 360.0) / 180.0 - 1.0, -s->a[1] / 90.0 + 1.0);
						glVertex2d((s->b[0] + 360.0) / 180.0 - 1.0, -s->b[1] / 90.0 + 1.0);
						glVertex2d(s->c[0] / 180.0 - 1.0, -s->c[1] / 90.0 + 1.0);

						glVertex2d(s->a[0] / 180.0 - 1.0, -s->a[1] / 90.0 + 1.0);
						glVertex2d(s->b[0] / 180.0 - 1.0, -s->b[1] / 90.0 + 1.0);
						glVertex2d((s->c[0] - 360.0) / 180.0 - 1.0, -s->c[1] / 90.0 + 1.0);
					} else if (s->c[0] < s->get_center()[0]) {
						glVertex2d((s->a[0] + 360.0) / 180.0 - 1.0, -s->a[1] / 90.0 + 1.0);
						glVertex2d(s->b[0] / 180.0 - 1.0, -s->b[1] / 90.0 + 1.0);
						glVertex2d((s->b[0] + 360.0) / 180.0 - 1.0, -s->c[1] / 90.0 + 1.0);

						glVertex2d(s->a[0] / 180.0 - 1.0, -s->a[1] / 90.0 + 1.0);
						glVertex2d((s->b[0] - 360.0) / 180.0 - 1.0, -s->b[1] / 90.0 + 1.0);
						glVertex2d(s->c[0] / 180.0 - 1.0, -s->c[1] / 90.0 + 1.0);
					} else {
						glVertex2d((s->a[0] + 360.0) / 180.0 - 1.0, -s->a[1] / 90.0 + 1.0);
						glVertex2d(s->b[0] / 180.0 - 1.0, -s->b[1] / 90.0 + 1.0);
						glVertex2d(s->c[0] / 180.0 - 1.0, -s->c[1] / 90.0 + 1.0);

						glVertex2d(s->a[0] / 180.0 - 1.0, -s->a[1] / 90.0 + 1.0);
						glVertex2d((s->b[0] - 360.0) / 180.0 - 1.0, -s->b[1] / 90.0 + 1.0);
						glVertex2d((s->c[0] - 360.0) / 180.0 - 1.0, -s->c[1] / 90.0 + 1.0);
					}
				}
				glEnd();
			}

			double m_x = ((double)mouse_x / (double)_screenWidth) * 360.0;
			double m_y = ((double)mouse_y / (double)_screenHeight) * 180.0;

			polar_t mp(
				m_x,
				m_y
			);

			_selected = get_face_from_point(mp, world->get_faces());
			if (_selected != NULL) {
				glColor3d(1.0, 0.0, 0.0);
				if (_selected->b[0] * _selected->a[1] + _selected->c[0] * _selected->b[1] + _selected->a[0] * _selected->c[1] > _selected->a[0] * _selected->b[1] + _selected->b[0] * _selected->c[1] + _selected->c[0] * _selected->a[1]) {
					glBegin(GL_LINE_STRIP);
					glVertex2d(_selected->a[0] / 180.0 - 1.0, -_selected->a[1] / 90.0 + 1.0);
					glVertex2d(_selected->b[0] / 180.0 - 1.0, -_selected->b[1] / 90.0 + 1.0);
					glVertex2d(_selected->c[0] / 180.0 - 1.0, -_selected->c[1] / 90.0 + 1.0);
					glVertex2d(_selected->a[0] / 180.0 - 1.0, -_selected->a[1] / 90.0 + 1.0);
					glEnd();
				} else {
					if (_selected->b[0] < _selected->get_center()[0]) {
						glBegin(GL_LINE_STRIP);
						glVertex2d((_selected->a[0] + 360.0) / 180.0 - 1.0, -_selected->a[1] / 90.0 + 1.0);
						glVertex2d((_selected->b[0] + 360.0) / 180.0 - 1.0, -_selected->b[1] / 90.0 + 1.0);
						glVertex2d(_selected->c[0] / 180.0 - 1.0, -_selected->c[1] / 90.0 + 1.0);
						glVertex2d((_selected->a[0] + 360.0) / 180.0 - 1.0, -_selected->a[1] / 90.0 + 1.0);
						glEnd();

						glBegin(GL_LINE_STRIP);
						glVertex2d(_selected->a[0] / 180.0 - 1.0, -_selected->a[1] / 90.0 + 1.0);
						glVertex2d(_selected->b[0] / 180.0 - 1.0, -_selected->b[1] / 90.0 + 1.0);
						glVertex2d((_selected->c[0] - 360.0) / 180.0 - 1.0, -_selected->c[1] / 90.0 + 1.0);
						glVertex2d(_selected->a[0] / 180.0 - 1.0, -_selected->a[1] / 90.0 + 1.0);
						glEnd();
					} else if (_selected->c[0] < _selected->get_center()[0]) {
						glBegin(GL_LINE_STRIP);
						glVertex2d((_selected->a[0] + 360.0) / 180.0 - 1.0, -_selected->a[1] / 90.0 + 1.0);
						glVertex2d(_selected->b[0] / 180.0 - 1.0, -_selected->b[1] / 90.0 + 1.0);
						glVertex2d((_selected->b[0] + 360.0) / 180.0 - 1.0, -_selected->c[1] / 90.0 + 1.0);
						glVertex2d((_selected->a[0] + 360.0) / 180.0 - 1.0, -_selected->a[1] / 90.0 + 1.0);
						glEnd();

						glBegin(GL_LINE_STRIP);
						glVertex2d(_selected->a[0] / 180.0 - 1.0, -_selected->a[1] / 90.0 + 1.0);
						glVertex2d((_selected->b[0] - 360.0) / 180.0 - 1.0, -_selected->b[1] / 90.0 + 1.0);
						glVertex2d(_selected->c[0] / 180.0 - 1.0, -_selected->c[1] / 90.0 + 1.0);
						glVertex2d(_selected->a[0] / 180.0 - 1.0, -_selected->a[1] / 90.0 + 1.0);
						glEnd();
					} else {
						glBegin(GL_LINE_STRIP);
						glVertex2d((_selected->a[0] + 360.0) / 180.0 - 1.0, -_selected->a[1] / 90.0 + 1.0);
						glVertex2d(_selected->b[0] / 180.0 - 1.0, -_selected->b[1] / 90.0 + 1.0);
						glVertex2d(_selected->c[0] / 180.0 - 1.0, -_selected->c[1] / 90.0 + 1.0);
						glVertex2d((_selected->a[0] + 360.0) / 180.0 - 1.0, -_selected->a[1] / 90.0 + 1.0);
						glEnd();

						glBegin(GL_LINE_STRIP);
						glVertex2d(_selected->a[0] / 180.0 - 1.0, -_selected->a[1] / 90.0 + 1.0);
						glVertex2d((_selected->b[0] - 360.0) / 180.0 - 1.0, -_selected->b[1] / 90.0 + 1.0);
						glVertex2d((_selected->c[0] - 360.0) / 180.0 - 1.0, -_selected->c[1] / 90.0 + 1.0);
						glVertex2d(_selected->a[0] / 180.0 - 1.0, -_selected->a[1] / 90.0 + 1.0);
						glEnd();
					}
				}
			}
			break;
		}
		case PROJ_SPHERE:
		{
			glBegin(GL_TRIANGLE_FAN); //BEGIN CIRCLE
			glVertex2d(0, 0); // center of circle
			for (int i = 0; i <= 90; i++) {
				glVertex2d(
					_cam->rot[0][0] * cos(i * 2.0 * M_PI / 90.0) / _resRatio, _cam->rot[0][0] * sin(i * 2.0 * M_PI / 90.0)
				);
			}
			glEnd(); //END
			glBegin(GL_TRIANGLES);
			for (auto &s : world->get_faces()) {
				if (s->type != surface_t::FACE_LAND)
					continue;
				switch (mode) {
					case MODE_LANDMASS:
						glColor3d(s->landmass->r, s->landmass->g, s->landmass->b);
						break;
					case MODE_FLAT: {
						auto biome = s->get_biome();
						glColor3ub(biome.r, biome.g, biome.b);
						break;
					}
					case MODE_ARIDITY:
						glColor3d(s->aridity - 2.0, 1.0 - std::abs(s->aridity - 2.0), 1.0 - std::abs(s->aridity - 1.0));
						break;
					case MODE_HEIGHT:
						glColor3d(s->height - 2.0, 1.0 - std::abs(s->height - 2.0), 1.0 - std::abs(s->height - 1.0));
						break;
					case MODE_FOEHN:
						glColor3d(s->foehn - 2.0, 1.0 - std::abs(s->foehn - 2.0), 1.0 - std::abs(s->foehn - 1.0));
						break;
					case MODE_DATA:
						glColor3d(s->aridity, s->height, s->foehn);
						break;
				}
				draw_shape(s);
			}
			glEnd();

			double m_x = (((double)mouse_x / (double)_screenWidth) * 2.0 - 1.0) * _resRatio / _cam->rot[0][0];
			double m_y = (((double)mouse_y / (double)_screenHeight) * -2.0 + 1.0) / _cam->rot[0][0];

			point3_t test(-m_x, -m_y, -std::sqrt(1.0 - std::sqrt(m_x * m_x + m_y * m_y)));
			test.coords = glm::inverse(rot_y(_cam->yaw)) * (glm::inverse(rot_x(_cam->pit)) * test.coords);

			double r = std::sqrt(test[0] * test[0] + test[1] * test[1] + test[2] * test[2]);
			polar_t mp(
				180.0 * std::atan2(test[1], test[0]) / M_PI + 180.0,
				180.0 * std::asin(test[2] / r) / M_PI + 90.0
			);

			_selected = get_face_from_point(mp, world->get_faces());
			if (_selected != NULL) {
				glColor3d(1.0, 0.0, 0.0);
				glBegin(GL_LINE_LOOP);
				draw_shape(_selected);
				glEnd();
			}
			break;
		}
	}

	SDL_GL_SwapWindow(_window);
}

void engine_t::user_input()
{
	SDL_Event evnt;
	const Uint8 *keystate = SDL_GetKeyboardState(NULL);

	if (keystate[SDL_SCANCODE_A] || keystate[SDL_SCANCODE_LEFT]) {
		_cam->yaw = std::fmod(_cam->yaw - 0.002 + 360.0, 360);
	}
	if (keystate[SDL_SCANCODE_D] || keystate[SDL_SCANCODE_RIGHT]) {
		_cam->yaw = std::fmod(_cam->yaw + 0.002 + 360.0, 360);
	}
	if (keystate[SDL_SCANCODE_W] || keystate[SDL_SCANCODE_UP]) {
		_cam->pit = CLAMP<double>(_cam->pit - 0.002, 0, 180);
	}
	if (keystate[SDL_SCANCODE_S] || keystate[SDL_SCANCODE_DOWN]) {
		_cam->pit = CLAMP<double>(_cam->pit + 0.002, 0, 180);
	}
	if (keystate[SDL_SCANCODE_Q]) {
		_cam->rot[0][0] += 0.00005;
		_cam->rot[1][1] += 0.00005;
	}
	if (keystate[SDL_SCANCODE_E]) {
		_cam->rot[0][0] -= 0.00005;
		_cam->rot[1][1] -= 0.00005;
	}

	while (SDL_PollEvent(&evnt)) {
		switch (evnt.type) {
			case SDL_QUIT:
				_windowState = windowState::EXIT;
				break;
			case SDL_MOUSEMOTION:
				mouse_x = evnt.motion.x;
				mouse_y = evnt.motion.y;
				//std::cout << "mouse x: " << evnt.motion.x << " mouse y: " << evnt.motion.y << std::endl;
				break;
			case SDL_MOUSEBUTTONDOWN:
				if (evnt.button.button == SDL_BUTTON_LEFT && _selected != NULL) {
					std::cout << _selected->get_biome().name << "\n";
					std::cout << "HEIGHT: " << _selected->height << "\nARIDITY: " << _selected->aridity << "\nFOEHN: " << _selected->foehn << "\n";
					std::cout << "A: (" << _selected->a[0] << ", " << _selected->a[1] << ")\n";
					std::cout << "B: (" << _selected->b[0] << ", " << _selected->b[1] << ")\n";
					std::cout << "C: (" << _selected->c[0] << ", " << _selected->c[1] << ")\n";
				}
			case SDL_KEYDOWN:
				switch (evnt.key.keysym.scancode) {
					case SDL_SCANCODE_O: {
						std::cout << "FILENAME: ";
						std::string fname;
						std::getline(std::cin, fname);
						serialize(fname);
						std::cout << "SAVED.\n";
						break;
					}
					case SDL_SCANCODE_I: {
						std::cout << "FILENAME: ";
						std::string fname;
						std::getline(std::cin, fname);
						load_file(fname);
						std::cout << "LOADED.\n";
						break;
					}
					case SDL_SCANCODE_1:
						mode = MODE_LANDMASS;
						break;
					case SDL_SCANCODE_2:
						mode = MODE_FLAT;
						break;
					case SDL_SCANCODE_3:
						mode = MODE_HEIGHT;
						break;
					case SDL_SCANCODE_4:
						mode = MODE_ARIDITY;
						break;
					case SDL_SCANCODE_5:
						mode = MODE_FOEHN;
						break;
					case SDL_SCANCODE_6:
						mode = MODE_DATA;
						break;
					case SDL_SCANCODE_TAB:
						projection = (projection_t)((projection + 1) % 2);
						break;
					case SDL_SCANCODE_R:
						std::cout << glm::to_string(_cam->rot) << "\n";
						break;
					default:
						break;
				}
				break;
			default:
				break;
		}
	}
}

void engine_t::engine_loop()
{
	//static int frameCount = 0;
	static double accumulator = 0.0;

	static double accumulatorTime = 1.0;
	while (_windowState != windowState::EXIT) {
		double frameStart = SDL_GetTicks();

		//render
		render_world();

		static double frameTime = SDL_GetTicks() - frameStart;
		//adjust simulation speed for framerate
		accumulator += accumulatorTime;
		while (accumulator >= 1.0 / 10.0) {
			user_input();
			accumulator -= 1.0 / 10.0;
		}
		if (1000.0 / _fpsMax > frameTime) {
			SDL_Delay((1000.0 / _fpsMax) - frameTime);
		}
		accumulatorTime = SDL_GetTicks() - frameStart;
		fps_counter();
	}

	SDL_GL_DeleteContext(glContext);
	SDL_DestroyWindow(_window);
	SDL_Quit();
}

void engine_t::fps_counter()
{
	static const int SAMPLES_PER_FPS = 10;
	static double frameTimes[SAMPLES_PER_FPS];
	static double prevTicks = SDL_GetTicks();
	static int currentFrame = 0;

	double currentTicks;
	currentTicks = SDL_GetTicks();

	_frameTime = currentTicks - prevTicks;
	prevTicks = currentTicks;
	frameTimes[currentFrame % SAMPLES_PER_FPS] = _frameTime;

	int count;

	if (currentFrame < SAMPLES_PER_FPS) {
		count = currentFrame;
	} else {
		count = SAMPLES_PER_FPS;
	}

	double frameTimeAverage = 0;

	for (int i = 0; i < count; i++) {
		frameTimeAverage += frameTimes[i];
	}
	frameTimeAverage /= count;

	if (frameTimeAverage > 0) {
		_fps = 1000.0 / frameTimeAverage;
	} else {
		_fps = -1;
	}

	currentFrame++;
}

void engine_t::serialize(const std::string &filename)
{
	std::ofstream file(filename);
	for (auto &s : world->get_faces()) {
		file << s->ID << "\t" << s->type << "\t" << s->height << "\t" << s->aridity << "\t" << s->foehn << "\t" << s->a[0] << "\t" << s->a[1] << "\t" << s->b[0] << "\t" << s->b[1] << "\t" << s->c[0] << "\t" << s->c[1];
		for (auto &n : s->neighbors) {
			file << "\t" << n->ID;
		}
		file << "\n";
	}
	file.close();
}

void engine_t::load_file(const std::string &filename)
{
	delete world;

	std::vector<std::pair<surface_t *, std::vector<std::string>>> parsed;

	std::ifstream file(filename);
	std::string line;
	while (std::getline(file, line)) {
		std::vector<std::string> data_set;
		std::stringstream data(line);
		std::string token;
		while (getline(data, token, '\t')) {
			data_set.push_back(token);
		}

		polar_t a(
			std::stod(data_set[5]),
			std::stod(data_set[6])
		);
		polar_t b(
			std::stod(data_set[7]),
			std::stod(data_set[8])
		);
		polar_t c(
			std::stod(data_set[9]),
			std::stod(data_set[10])
		);

		surface_t *s = new surface_t(
			std::stoull(data_set[0]),
			a, b, c
		);

		s->type = (surface_t::surface_type)std::stoi(data_set[1]);
		s->height = std::stod(data_set[2]);
		s->aridity = std::stod(data_set[3]);
		s->foehn = std::stod(data_set[4]);

		data_set.erase(data_set.begin(), data_set.begin() + 11);

		parsed.push_back({ s, data_set });
	}

	std::vector<surface_t *> _set;

	for (auto &e : parsed) {
		for (auto &n : e.second) {
			e.first->neighbors.push_back(parsed[std::stoull(n)].first);
		}
		_set.push_back(e.first);
	}

	world = new world_t(_set);
}