#include "point3.h"

#include <iostream>

point3_t::point3_t(const double &x, const double &y, const double &z)
	: coords{ x, y, z }
{}

point3_t::point3_t()
	: coords{ 0,0,0 }
{}

point3_t::point3_t(const glm::vec3 &coords)
	: coords{ coords }
{}

const double point3_t::operator[](const size_t &i) const
{
	return coords[i];
}

point3_t::point3_t(const polar_t &p, const double &r)
	: coords{
		r * DCOS(p[0]) * DSIN(p[1]),
		r * DSIN(p[0]) * DSIN(p[1]),
		r * DCOS(p[1]) }
{}