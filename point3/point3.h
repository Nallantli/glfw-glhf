#pragma once

#include <eigen3/Eigen/Dense>
#include <vector>

#include "../biome/biome.h"
#include "../polar/polar.h"

#define M_PI           3.14159265358979323846  /* pi */

#define MAX(a, b) (a > b ? a : b)
#define MIN(a, b) (a < b ? a : b)
#define DCOS(x) ((float)std::cos(M_PI * x / 180.0f))
#define DSIN(x) ((float)std::sin(M_PI * x / 180.0f))
#define RAD(x) ((float)(M_PI / 180.0f) * x)
#define CLAMP(n, lower, upper) (n > upper ? upper : (n < lower ? lower : n))

struct point3_t
{
	Eigen::Matrix<float, 3, 1> coords;
	point3_t();
	point3_t(const float &, const float &, const float &);
	point3_t(const Eigen::Matrix<float, 3, 1> &);
	point3_t(const polar_t &, const float &);
	const float operator[](const size_t &) const;
};