#include "engine.h"
#ifdef _WIN32
#include <Windows.h>
#include <ShellScalingAPI.h>
#include <glm/mat3x3.hpp>
#include <comdef.h>
#endif
#include <fstream>
#include <bits/stdc++.h>

enum Mode
{
	MODE_LANDMASS,
	MODE_FLAT,
	MODE_DATA,
	MODE_FOEHN
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

surface_t *get_face_from_point(const polar_t &p, std::vector<surface_t *> &set)
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

camera::camera(const double &yaw, const double &pit, const double &dist) : yaw{ yaw }, pit{ pit }
{
	rot[0][0] = dist;
	rot[1][1] = dist;
}

engine::engine(const int &seed)
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
	_cam = new camera(270, 90, 1);
	_windowState = windowState::RUN;
	_fpsMax = 120;
}

engine::~engine()
{}

void engine::run()
{
	init_engine();
}

void engine::init_engine()
{
	srand(_seed);
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_DisplayMode DM;
	std::cout << SDL_GetCurrentDisplayMode(0, &DM);
	_screenWidth = DM.w / 1.2;
	_screenHeight = DM.h / 1.2;
	_window = SDL_CreateWindow("My Title", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, _screenWidth, _screenHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI);
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

	generate_world(_set, landmasses, _seed);
	std::cout << "World generated with seed: " << _seed << std::endl;
	engine_loop();
}

void engine::draw_secant_line(const polar_t &a, const polar_t &b)
{
	/*double z_a = flatten(a, _cam, r_a);
		double z_b = flatten(b, _cam, r_b);

		double dp = r_a.x * r_b.x + r_a.y * r_b.y + z_a * z_b;

		double theta = std::acos(dp / (_cam->dist * _cam->dist));*/

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

void engine::draw_shape(const surface_t *s)
{
	auto r1 = rot_x(_cam->pit) * (rot_y(_cam->yaw) * point3_t(s->a, 1).coords);
	auto t1 = _cam->rot * r1;
	auto r2 = rot_x(_cam->pit) * (rot_y(_cam->yaw) * point3_t(s->b, 1).coords);
	auto t2 = _cam->rot * r2;
	auto r3 = rot_x(_cam->pit) * (rot_y(_cam->yaw) * point3_t(s->c, 1).coords);
	auto t3 = _cam->rot * r3;

	if (r1[2] < 0 || r2[2] < 0 || r3[2] < 0)
		return;

	glBegin(GL_LINE_STRIP);
	glVertex2d(t1[0] / _resRatio, t1[1]);
	glVertex2d(t2[0] / _resRatio, t2[1]);
	glVertex2d(t3[0] / _resRatio, t3[1]);
	glVertex2d(t1[0] / _resRatio, t1[1]);
	glEnd();
}

void engine::fill_shape(const surface_t *s)
{
	auto r1 = rot_x(_cam->pit) * (rot_y(_cam->yaw) * point3_t(s->a, 1).coords);
	auto t1 = _cam->rot * r1;
	auto r2 = rot_x(_cam->pit) * (rot_y(_cam->yaw) * point3_t(s->b, 1).coords);
	auto t2 = _cam->rot * r2;
	auto r3 = rot_x(_cam->pit) * (rot_y(_cam->yaw) * point3_t(s->c, 1).coords);
	auto t3 = _cam->rot * r3;

	if (r1[2] < 0 || r2[2] < 0 || r3[2] < 0)
		return;

	glBegin(GL_TRIANGLES);
	glVertex2d(t1[0] / _resRatio, t1[1]);
	glVertex2d(t2[0] / _resRatio, t2[1]);
	glVertex2d(t3[0] / _resRatio, t3[1]);
	glEnd();
}

void engine::render_world()
{
	glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glColor3d(0.0, 0.0, 0.5);

	switch (projection) {
		case PROJ_MERC:
		{
			glBegin(GL_QUADS);
			glVertex2d(-1, -1);
			glVertex2d(1, -1);
			glVertex2d(1, 1);
			glVertex2d(-1, 1);
			glEnd();
			for (auto &s : _set) {
				if (s->type != surface_t::FACE_LAND)
					continue;
				switch (mode) {
					case MODE_LANDMASS:
						glColor3d(s->landmass->r, s->landmass->g, s->landmass->b);
						break;
					case MODE_FLAT: {
						auto biome = s->get_biome();
						glColor3d(biome.r, biome.g, biome.b);
						break;
					}
					case MODE_FOEHN:
						glColor3d(s->foehn, s->foehn, s->foehn);
						break;
					case MODE_DATA:
						glColor3d(s->aridity, s->height, 0.0);
						break;
				}
				glBegin(GL_TRIANGLES);
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

			_selected = get_face_from_point(mp, _set);
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
			for (auto &s : _set) {
				if (s->type != surface_t::FACE_LAND)
					continue;
				switch (mode) {
					case MODE_LANDMASS:
						glColor3d(s->landmass->r, s->landmass->g, s->landmass->b);
						break;
					case MODE_FLAT: {
						auto biome = s->get_biome();
						glColor3d(biome.r, biome.g, biome.b);
						break;
					}
					case MODE_FOEHN:
						glColor3d(s->foehn, s->foehn, s->foehn);
						break;
					case MODE_DATA:
						glColor3d(s->aridity, s->height, 0.0);
						break;
				}
				fill_shape(s);
			}

			double m_x = (((double)mouse_x / (double)_screenWidth) * 2.0 - 1.0) * _resRatio / _cam->rot[0][0];
			double m_y = (((double)mouse_y / (double)_screenHeight) * -2.0 + 1.0) / _cam->rot[0][0];

			point3_t test(-m_x, -m_y, -std::sqrt(1.0 - std::sqrt(m_x * m_x + m_y * m_y)));
			test.coords = glm::inverse(rot_y(_cam->yaw)) * (glm::inverse(rot_x(_cam->pit)) * test.coords);

			double r = std::sqrt(test[0] * test[0] + test[1] * test[1] + test[2] * test[2]);
			polar_t mp(
				180.0 * std::atan2(test[1], test[0]) / M_PI + 180.0,
				180.0 * std::asin(test[2] / r) / M_PI + 90.0
			);

			_selected = get_face_from_point(mp, _set);
			if (_selected != NULL) {
				glColor3d(1.0, 0.0, 0.0);
				draw_shape(_selected);
			}
			break;
		}
	}

	SDL_GL_SwapWindow(_window);
}

void engine::user_input()
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
		_cam->pit = CLAMP(_cam->pit - 0.002, 0, 180);
	}
	if (keystate[SDL_SCANCODE_S] || keystate[SDL_SCANCODE_DOWN]) {
		_cam->pit = CLAMP(_cam->pit + 0.002, 0, 180);
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
						std::cout << "SAVED.";
						break;
					}
					case SDL_SCANCODE_I: {
						std::cout << "FILENAME: ";
						std::string fname;
						std::getline(std::cin, fname);
						load_file(fname);
						std::cout << "LOADED.";
						break;
					}
					case SDL_SCANCODE_1:
						mode = MODE_LANDMASS;
						break;
					case SDL_SCANCODE_2:
						mode = MODE_FLAT;
						break;
					case SDL_SCANCODE_3:
						mode = MODE_DATA;
						break;
					case SDL_SCANCODE_4:
						mode = MODE_FOEHN;
						break;
					case SDL_SCANCODE_TAB:
						projection = (projection_t)((projection + 1) % 2);
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

void engine::engine_loop()
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

		//regulate fps
		/*
		if (frameCount==30)
		{
			std::cout << _fps << std::endl;
			frameCount = 0;
		}
		frameCount++;
		*/
		if (1000.0 / _fpsMax > frameTime) {
			SDL_Delay((1000.0 / _fpsMax) - frameTime);
		}
		accumulatorTime = SDL_GetTicks() - frameStart;
		fps_counter();
	}

	SDL_GL_DeleteContext(glContext);
	SDL_DestroyWindow(_window);
	SDL_Quit();
	delete _cam;
	for (auto &e : _set)
		delete e;
	for (auto &e : landmasses)
		delete e;
	_set.clear();
	landmasses.clear();
}

void engine::fps_counter()
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

void engine::serialize(const std::string &filename)
{
	std::ofstream file(filename);
	for (auto &s : _set) {
		file << s->ID << "\t" << s->type << "\t" << s->height << "\t" << s->aridity << "\t" << s->foehn << "\t" << s->a[0] << "\t" << s->a[1] << "\t" << s->b[0] << "\t" << s->b[1] << "\t" << s->c[0] << "\t" << s->c[1];
		for (auto &n : s->neighbors){
			file << "\t" << n->ID;
		}
		file << "\n";
	}
	file.close();
}

void engine::load_file(const std::string &filename)
{
	for (auto &e : _set)
		delete e;
	for (auto &e : landmasses)
		delete e;
	_set.clear();
	landmasses.clear();

	std::vector<std::pair<surface_t*, std::vector<std::string>>> parsed;

	std::ifstream file(filename);
	std::string line;
	while (std::getline(file, line)) {
		std::vector<std::string> data_set;
		std::stringstream data(line);
		std::string token;
		while (getline(data, token, '\t')){
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

		surface_t * s = new surface_t(
			std::stoull(data_set[0]),
			a, b, c
		);

		s->type = (surface_t::surface_type)std::stoi(data_set[1]);
		s->height = std::stod(data_set[2]);
		s->aridity = std::stod(data_set[3]);
		s->foehn = std::stod(data_set[4]);

		data_set.erase(data_set.begin(), data_set.begin() + 11);

		parsed.push_back({s, data_set});
	}

	for (auto &e : parsed) {
		for (auto &n : e.second) {
			e.first->neighbors.push_back(parsed[std::stoull(n)].first);
		}
		_set.push_back(e.first);
	}

	for (auto &s : _set) {
		if (s->landmass == NULL) {
			landmass_t *l = new landmass_t{
				(double)rand() / (double)RAND_MAX,
				(double)rand() / (double)RAND_MAX,
				(double)rand() / (double)RAND_MAX
			};
			s->landmass = l;
			landmasses.push_back(l);
			l->members.push_back(s);
			make_landmasses(s);
		}
	}
}