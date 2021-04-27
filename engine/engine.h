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

#include "../generator/generator.h"

enum class windowState
{
    RUN, EXIT
};

struct camera
{
    float yaw;
    float pit;
    glm::mat2x3 rot;

    camera(const float &yaw, const float &pit, const float &dist);
};

class engine
{
public:
    engine(const long long &seed);
    ~engine();
    void run();

private:
    void init_engine();
    void draw_secant_line(const polar_t &a, const polar_t &b);
    void draw_shape(const surface_t *s);
    void fill_shape(const surface_t *s);
    std::vector<std::string> split(const std::string &s, char delimiter);
    void render_world();
    void user_input();
    void engine_loop();
    void fps_counter();

    SDL_Window *_window;
    SDL_GLContext glContext;
    double _screenWidth;
    double _screenHeight;
    float _resRatio;
    std::map<int, bool> _KEYS;
    std::vector<surface_t *> _set;
    camera *_cam;
    windowState _windowState;
    surface_t *_selected;
    float _frameTime;
    float _fps;
    float _fpsMax;

    int mouse_x;
    int mouse_y;
    time_t _seed;
};