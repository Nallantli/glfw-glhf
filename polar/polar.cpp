#include "polar.h"

polar_t::polar_t()
{
	coords << 0, 0;
}

polar_t::polar_t(const float &yaw, const float &pit)
{
	coords(0) = yaw;
	coords(1) = pit;
}

const float polar_t::operator[](const size_t &i) const
{
	return coords(i);
}

const bool polar_t::operator==(const polar_t &p) const
{
	return coords(0) == p[0] && coords(1) == p[1];
}

const bool polar_t::operator!=(const polar_t &p) const
{
	return coords(0) != p[0] || coords(1) != p[1];
}