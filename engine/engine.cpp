#include "engine.h"
#ifdef _WIN32
#include <Windows.h>
#include <ShellScalingAPI.h>
#include <glm/mat3x3.hpp>
#include <comdef.h>
#endif

enum Mode
{
	MODE_WIRE,
	MODE_FLAT,
	MODE_DATA,
	MODE_FOEHN
};

Mode mode = MODE_FLAT;

const glm::mat3 rot_x(const float &theta)
{
	glm::mat3 m{
		1, 0, 0,
		0, DCOS(theta), -DSIN(theta),
		0, DSIN(theta), DCOS(theta)
	};
	return m;
}

const glm::mat3 rot_y(const float &theta)
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
	float dist = 100.0f;
	for (auto &s : set) {
		point3_t cc(p, 1.0f);
		auto x = glm::dot(cc.coords, s->get_center_c().coords);
		float d = std::acos(x);
		if (curr == NULL || d < dist) {
			curr = s;
			dist = d;
		}
	}
	return curr;
}

camera::camera(const float &yaw, const float &pit, const float &dist) : yaw{ yaw }, pit{ pit }
{
	rot[0][0] = dist;
	rot[1][1] = dist;
}

engine::engine(const long long &seed)
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
	_screenWidth = 2560;
	_screenHeight = 1440;
	_resRatio = _screenWidth / _screenHeight;
	_cam = new camera(180, 90, 1);
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

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	generate_world(_set, _seed);
	std::cout << "World generated with seed: " << _seed << std::endl;
	engine_loop();
}

void engine::draw_secant_line(const polar_t &a, const polar_t &b)
{
	/*float z_a = flatten(a, _cam, r_a);
		float z_b = flatten(b, _cam, r_b);

		float dp = r_a.x * r_b.x + r_a.y * r_b.y + z_a * z_b;

		float theta = std::acos(dp / (_cam->dist * _cam->dist));*/

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
		polar_t p(a[0] + delta_yaw * i, a[1] + delta_pit * i);
		glm::vec2 t = _cam->rot * (rot_x(_cam->pit) * (rot_y(_cam->yaw) * point3_t(p, 1).coords));
		glVertex2f(t[0] / _resRatio, t[1]);
	}
	glm::vec2 t = _cam->rot * (rot_x(_cam->pit) * (rot_y(_cam->yaw) * point3_t(b, 1).coords));
	glVertex2f(t[0] / _resRatio, t[1]);
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
	glVertex2f(t1[0] / _resRatio, t1[1]);
	glVertex2f(t2[0] / _resRatio, t2[1]);
	glVertex2f(t3[0] / _resRatio, t3[1]);
	glVertex2f(t1[0] / _resRatio, t1[1]);
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
	glVertex2f(t1[0] / _resRatio, t1[1]);
	glVertex2f(t2[0] / _resRatio, t2[1]);
	glVertex2f(t3[0] / _resRatio, t3[1]);
	glEnd();
}

void engine::render_world()
{
	glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glColor3f(0.0f, 0.0f, 0.5f);
	glBegin(GL_TRIANGLE_FAN); //BEGIN CIRCLE
	glVertex2f(0, 0); // center of circle
	for (int i = 0; i <= 90; i++) {
		glVertex2f(
			_cam->rot[0][0] * cos(i * 2.0f * M_PI / 90.0f) / _resRatio, _cam->rot[0][0] * sin(i * 2.0f * M_PI / 90.0f)
		);
	}
	glEnd(); //END

	for (auto &s : _set) {
		switch (mode) {
			case MODE_WIRE:
				if (s->type == surface_t::FACE_LAND) {
					auto biome = s->get_biome();
					glColor3f(biome.r, biome.g, biome.b);
					fill_shape(s);
				}
				glColor3f(1.0f, 1.0f, 1.0f);
				draw_shape(s);
				break;
			case MODE_FOEHN:
				if (s->type == surface_t::FACE_LAND) {
					glColor3f(s->foehn, s->foehn, s->foehn);
					fill_shape(s);
				}
				break;
			case MODE_DATA:
				if (s->type == surface_t::FACE_LAND) {
					glColor3f(s->aridity, s->height, 0.0f);
					fill_shape(s);
				}
				break;
			case MODE_FLAT:
				if (s->type == surface_t::FACE_LAND) {
					auto biome = s->get_biome();
					glColor3f(biome.r, biome.g, biome.b);
					fill_shape(s);
				}
				break;
		}
	}

	float m_x = (((float)mouse_x / (float)_screenWidth) * 2.0f - 1.0f) * _resRatio / _cam->rot[0][0];
	float m_y = (((float)mouse_y / (float)_screenHeight) * -2.0f + 1.0f) / _cam->rot[0][0];

	point3_t test(-m_x, -m_y, -std::sqrt(1.0f - std::sqrt(m_x * m_x + m_y * m_y)));
	test.coords = glm::inverse(rot_y(_cam->yaw)) * (glm::inverse(rot_x(_cam->pit)) * test.coords);

	float r = std::sqrt(test[0] * test[0] + test[1] * test[1] + test[2] * test[2]);
	polar_t mp(
		180.0f * std::atan2(test[1], test[0]) / M_PI + 180.0f,
		180.0f * std::asin(test[2] / r) / M_PI + 90.0f
	);

	_selected = get_face_from_point(mp, _set);
	if (_selected != NULL) {
		glColor3f(1.0f, 0.0f, 0.0f);
		draw_shape(_selected);
	}

	SDL_GL_SwapWindow(_window);
}

void engine::user_input()
{
	SDL_Event evnt;
	const Uint8 *keystate = SDL_GetKeyboardState(NULL);

	if (keystate[SDL_SCANCODE_A] || keystate[SDL_SCANCODE_LEFT]) {
		_cam->yaw = std::fmod(_cam->yaw - 0.0015f + 360.0f, 360);
	}
	if (keystate[SDL_SCANCODE_D] || keystate[SDL_SCANCODE_RIGHT]) {
		_cam->yaw = std::fmod(_cam->yaw + 0.0015f + 360.0f, 360);
	}
	if (keystate[SDL_SCANCODE_W] || keystate[SDL_SCANCODE_UP]) {
		_cam->pit = CLAMP(_cam->pit - 0.0015f, 0, 180);
	}
	if (keystate[SDL_SCANCODE_S] || keystate[SDL_SCANCODE_DOWN]) {
		_cam->pit = CLAMP(_cam->pit + 0.0015f, 0, 180);
	}
	if (keystate[SDL_SCANCODE_Q]) {
		_cam->rot[0][0] += 0.00005f;
		_cam->rot[1][1] += 0.00005f;
	}
	if (keystate[SDL_SCANCODE_E]) {
		_cam->rot[0][0] -= 0.00005f;
		_cam->rot[1][1] -= 0.00005f;
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
					case SDL_SCANCODE_1:
						mode = MODE_WIRE;
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
					case SDL_SCANCODE_ESCAPE:
						_windowState = windowState::EXIT;
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
	static float accumulator = 0.0f;

	static float accumulatorTime = 1.0f;
	while (_windowState != windowState::EXIT) {
		float frameStart = SDL_GetTicks();

		//render
		render_world();

		static float frameTime = SDL_GetTicks() - frameStart;
		//adjust simulation speed for framerate
		accumulator += accumulatorTime;
		while (accumulator >= 1.0f / 10.0f) {
			user_input();
			accumulator -= 1.0f / 10.0f;
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
		if (1000.0f / _fpsMax > frameTime) {
			SDL_Delay((1000.0f / _fpsMax) - frameTime);
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
	_set.clear();
}

void engine::fps_counter()
{
	static const int SAMPLES_PER_FPS = 10;
	static float frameTimes[SAMPLES_PER_FPS];
	static float prevTicks = SDL_GetTicks();
	static int currentFrame = 0;

	float currentTicks;
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

	float frameTimeAverage = 0;

	for (int i = 0; i < count; i++) {
		frameTimeAverage += frameTimes[i];
	}
	frameTimeAverage /= count;

	if (frameTimeAverage > 0) {
		_fps = 1000.0f / frameTimeAverage;
	} else {
		_fps = -1;
	}

	currentFrame++;
}