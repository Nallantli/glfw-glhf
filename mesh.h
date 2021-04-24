#pragma once

#include <eigen3/Eigen/Dense>
#include <vector>
#include "biome.h"

#define M_PI           3.14159265358979323846  /* pi */

#define MAX(a, b) (a > b ? a : b)
#define MIN(a, b) (a < b ? a : b)
#define DCOS(x) ((float)std::cos(M_PI * x / 180.0f))
#define DSIN(x) ((float)std::sin(M_PI * x / 180.0f))
#define RAD(x) ((float)(M_PI / 180.0f) * x)

class s_point;
struct face_t;
struct c_point;


struct c_point
{
	Eigen::Matrix<float, 3, 1> coords;
	c_point();
	c_point(const float &, const float &, const float &);
	c_point(const Eigen::Matrix<float, 3, 1> &);
	c_point(const s_point &, const float &);
	const float operator[](const size_t &) const;
};

class s_point
{
private:
	Eigen::Matrix<float, 2, 1> coords;
public:
	s_point();
	s_point(const float &, const float &);
	const float operator[](const size_t &) const;
	const bool operator==(const s_point &) const;
	const bool operator!=(const s_point &) const;
};

struct face_t
{
	const s_point a;
	const s_point b;
	const s_point c;

	enum face_type
	{
		FACE_LAND,
		FACE_WATER,
		FACE_OCEAN,
		FACE_FLOWING,
		FACE_STAGNANT,
		FACE_INLAND_LAKE,
	} type = FACE_WATER;

	float height = 0;
	float aridity = 0;
	float foehn = 0;

	std::vector<face_t *> neighbors;

	face_t(const s_point &, const s_point &, const s_point &);
	const bool operator==(const face_t &);
	const s_point get_center() const;
	const c_point get_center_c() const;
	const biome_t get_biome() const;

private:
	s_point s_center;
	c_point c_center;
	void set_center();
};