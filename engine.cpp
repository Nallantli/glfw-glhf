#include "engine.h"


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

camera::camera(const float &yaw, const float &pit, const float &dist) : yaw{ yaw }, pit{ pit }
{
	rot(0, 0) = dist;
	rot(1, 1) = dist;
}

engine::engine()
{
    _window = nullptr;
    _screenWidth = 920;
    _screenHeight = 720;
    _resRatio = _screenWidth / _screenHeight;
    _cam = new camera(180, 90, 1);
    _windowState = windowState::RUN;
}

engine::~engine(){}

void engine::run()
{
	srand(time(0));
    init_engine();
}

void engine::init_engine()
{
	SDL_Init(SDL_INIT_EVERYTHING);
	_window = SDL_CreateWindow("My Title", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, _screenWidth, _screenHeight, SDL_WINDOW_OPENGL);	
	if(_window == nullptr){
		std::cerr << "SDL_window could not be created.";
	}
	
	SDL_GLContext glContext = SDL_GL_CreateContext(_window);		
	if(_window == nullptr){
		std::cerr << "SDL_GL context could not be created.";
	}	

	GLenum error = glewInit();	
	if(error != GLEW_OK){
		std::cerr << "Glew init failed.";
	}

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	engine_loop();
}

void engine::draw_secant_line(const s_point &a, const s_point &b)
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
		s_point p(a[0] + delta_yaw * i, a[1] + delta_pit * i);
		Eigen::Matrix<float, 2, 1> t = _cam->rot * (rot_x(_cam->pit) * (rot_y(_cam->yaw) * c_point(p, 1).coords));
		glVertex2f(t(0) / _resRatio, t(1));
	}
	Eigen::Matrix<float, 2, 1> t = _cam->rot * (rot_x(_cam->pit) * (rot_y(_cam->yaw) * c_point(b, 1).coords));
	glVertex2f(t(0) / _resRatio, t(1));
	glEnd();
}

void engine::draw_shape(const face_t *s)
{
	Eigen::Matrix<float, 3, 1> r1 = rot_x(_cam->pit) * (rot_y(_cam->yaw) * c_point(s->a, 1).coords);
	Eigen::Matrix<float, 2, 1> t1 = _cam->rot * r1;
	Eigen::Matrix<float, 3, 1> r2 = rot_x(_cam->pit) * (rot_y(_cam->yaw) * c_point(s->b, 1).coords);
	Eigen::Matrix<float, 2, 1> t2 = _cam->rot * r2;
	Eigen::Matrix<float, 3, 1> r3 = rot_x(_cam->pit) * (rot_y(_cam->yaw) * c_point(s->c, 1).coords);
	Eigen::Matrix<float, 2, 1> t3 = _cam->rot * r3;

	if (r1(2) < 0 || r2(2) < 0 || r3(2) < 0)
		return;

	glBegin(GL_LINE_STRIP);
	glVertex2f(t1(0) / _resRatio, t1(1));
	glVertex2f(t2(0) / _resRatio, t2(1));
	glVertex2f(t3(0) / _resRatio, t3(1));
	glVertex2f(t1(0) / _resRatio, t1(1));
	glEnd();
}

void engine::fill_shape(const face_t *s)
{
	Eigen::Matrix<float, 3, 1> r1 = rot_x(_cam->pit) * (rot_y(_cam->yaw) * c_point(s->a, 1).coords);
	Eigen::Matrix<float, 2, 1> t1 = _cam->rot * r1;
	Eigen::Matrix<float, 3, 1> r2 = rot_x(_cam->pit) * (rot_y(_cam->yaw) * c_point(s->b, 1).coords);
	Eigen::Matrix<float, 2, 1> t2 = _cam->rot * r2;
	Eigen::Matrix<float, 3, 1> r3 = rot_x(_cam->pit) * (rot_y(_cam->yaw) * c_point(s->c, 1).coords);
	Eigen::Matrix<float, 2, 1> t3 = _cam->rot * r3;

	if (r1(2) < 0 || r2(2) < 0 || r3(2) < 0)
		return;

	glBegin(GL_TRIANGLES);
	glVertex2f(t1(0) / _resRatio, t1(1));
	glVertex2f(t2(0) / _resRatio, t2(1));
	glVertex2f(t3(0) / _resRatio, t3(1));
	glEnd();
}

std::vector<std::string> engine::split(const std::string &s, char delimiter)
{
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(s);
	while (std::getline(tokenStream, token, delimiter)) {
		tokens.push_back(token);
	}
	return tokens;
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
			_cam->rot(0, 0) * cos(i * 2.0f * M_PI / 90.0f) / _resRatio, _cam->rot(0, 0) * sin(i * 2.0f * M_PI / 90.0f)
		);
	}
	glEnd(); //END

	for (auto &s : _set) {
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
				fill_shape(s);
				break;
			}
		}
		switch (mode) {
			case MODE_WIRE: {
				glColor3f(1.0f, 1.0f, 1.0f); // Red
				draw_shape(s);
				break;
			}
			case MODE_FLAT: {
				break;
			}
		}
	}
	SDL_GL_SwapWindow(_window);
}

void engine::user_input()
{
    SDL_Event evnt;
    const Uint8 *keystate = SDL_GetKeyboardState(NULL);

    if (keystate[SDL_SCANCODE_A])
    {
        _cam->yaw = std::fmod(_cam->yaw + 0.01f + 360.0f, 360);
    }
    if (keystate[SDL_SCANCODE_D])
    {
        _cam->yaw = std::fmod(_cam->yaw - 0.01f + 360.0f, 360);
    }
    if (keystate[SDL_SCANCODE_W])
    {
        _cam->pit = _cam->pit + 0.01f;
    }
    if (keystate[SDL_SCANCODE_S])
    {
        _cam->pit = _cam->pit - 0.01f;
    }
    if (keystate[SDL_SCANCODE_Q])
    {
        _cam->rot(0, 0) += 0.005f;
        _cam->rot(1, 1) += 0.005f;
    }
    if (keystate[SDL_SCANCODE_E])
    {
        _cam->rot(0, 0) -= 0.005f;
        _cam->rot(1, 1) -= 0.005f;
    }

    while(SDL_PollEvent(&evnt))
    {
        switch (evnt.type)
        {
            case SDL_QUIT:
                _windowState = windowState::EXIT;
                break;
            case SDL_MOUSEMOTION:
                std::cout << "mouse x: " << evnt.motion.x << "mouse y: " << evnt.motion.y << std::endl;
            case SDL_KEYDOWN:
                if (evnt.key.keysym.scancode == SDL_SCANCODE_1)
                {
                    mode = MODE_WIRE;
                    break;
                }
                if (evnt.key.keysym.scancode == SDL_SCANCODE_2)
                {
                    mode = MODE_FLAT;
                    break;
                }
                if (evnt.key.keysym.scancode == SDL_SCANCODE_3)
                {
                    mode = MODE_DATA;
                    break;
                }
                if (evnt.key.keysym.scancode == SDL_SCANCODE_4)
                {
                    mode = MODE_FOEHN;
                    break;
                }
        }
    }
}

void engine::engine_loop()
{
	generate_world(_set);
	
	while (_windowState != windowState::EXIT) {
		render_world();
		user_input();
	}

    delete _cam;
	for (auto &e : _set)
		delete e;
	_set.clear();
}