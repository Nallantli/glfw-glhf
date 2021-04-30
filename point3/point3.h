#pragma once

#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <vector>

#include "../biome/biome.h"
#include "../polar/polar.h"

#define M_PI           3.14159265358979323846  /* pi */

template<typename T>
constexpr inline const T &MAX(const T &a, const T &b)
{
	return (a > b ? a : b);
}

template<typename T>
constexpr inline const T &MIN(const T &a, const T &b)
{
	return (a < b ? a : b);
}

constexpr inline double DCOS(const double &x)
{
	return std::cos(M_PI * x / 180.0);
}

constexpr inline double DSIN(const double &x)
{
	return std::sin(M_PI * x / 180.0);
}

constexpr inline double RAD(const double &x)
{
	return (M_PI / 180.0) * x;
}

template<typename T>
constexpr inline const T &CLAMP(const T &n, const T &lower, const T &upper)
{
	return (n > upper ? upper : (n < lower ? lower : n));
}

struct point3_t
{
	glm::vec3 coords;
	point3_t();
	point3_t(const double &, const double &, const double &);
	point3_t(const glm::vec3 &);
	point3_t(const polar_t &, const double &);
	const double operator[](const size_t &) const;
};