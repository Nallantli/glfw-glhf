#include "point3.h"

#include <iostream>

point3_t::point3_t(const float &x, const float &y, const float &z)
{
	coords(0) = x;
	coords(1) = y;
	coords(2) = z;
}

point3_t::point3_t()
{
	coords << 0, 0, 0;
}

point3_t::point3_t(const Eigen::Matrix<float, 3, 1> &coords) : coords{ coords }
{}

const float point3_t::operator[](const size_t &i) const
{
	return coords(i);
}

point3_t::point3_t(const polar_t &p, const float &r)
{
	coords(0) = r * DCOS(p[0]) * DSIN(p[1]);
	coords(1) = r * DSIN(p[0]) * DSIN(p[1]);
	coords(2) = r * DCOS(p[1]);
}