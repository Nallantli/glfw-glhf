#include "polar.h"

polar_t::polar_t()
	: coords{ 0, 0 }
{}

polar_t::polar_t(const double &yaw, const double &pit)
	: coords{ yaw, pit }
{}

const double polar_t::operator[](const size_t &i) const
{
	return coords[i];
}

const bool polar_t::operator==(const polar_t &p) const
{
	return coords[0] == p[0] && coords[1] == p[1];
}

const bool polar_t::operator!=(const polar_t &p) const
{
	return coords[0] != p[0] || coords[1] != p[1];
}

const bool polar_t::operator<(const polar_t &p) const
{
	if (coords[0] == p[0])
		return coords[1] < p[1];
	return coords[0] < p[0];
}