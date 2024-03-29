#pragma once

#define GLFW_INCLUDE_NONE
#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1

#include <windows.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/mat2x3.hpp>
#include <SDL2/SDL.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <chrono>
#include <map>
#include <cfloat>
#include <fstream>
#include <map>

#include "../world/world.h"

enum class windowState
{
    RUN, EXIT
};

struct camera
{
    double yaw;
    double pit;
    glm::mat2x3 rot;

    camera(const double &yaw, const double &pit, const double &dist);
};

class engine_t
{
public:
    engine_t(const int &seed);
    ~engine_t();
    void run();

private:
    void draw_letter(const char &c, const double &size, const double &x, const double &y);
    void draw_string(const std::string &s, const double &size, const double &x, const double &y);
    void init_engine();
    void draw_secant_line(const polar_t &a, const polar_t &b);
    void draw_shape(const surface_t *s);
    std::vector<std::string> split(const std::string &s, char delimiter);
    void render_world();
    void user_input();
    void engine_loop();
    void fps_counter();

    void serialize(const std::string &filename);
    void load_file(const std::string &filename);

    world_t * world;

    SDL_Window *_window;
    SDL_GLContext glContext;
    int _screenWidth;
    int _screenHeight;
    double _resRatio;
    std::map<int, bool> _KEYS;
    camera *_cam;
    windowState _windowState;
    surface_t *_selected;
    double _frameTime;
    double _fps;
    double _fpsMax;

    enum projection_t
    {
        PROJ_SPHERE,
        PROJ_MERC
    } projection = PROJ_SPHERE;

    int mouse_x;
    int mouse_y;
    int _seed;
};