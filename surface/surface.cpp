#include "surface.h"

surface_t::surface_t(const polar_t &a, const polar_t &b, const polar_t &c)
	: a{ a }
	, b{ b }
	, c{ c }
{
	set_center();
}

const polar_t surface_t::get_center() const
{
	return this->s_center;
}

const point3_t surface_t::get_center_c() const
{
	return this->c_center;
}

const bool surface_t::operator==(const surface_t &f)
{
	return f.a == a && f.b == b && f.c == c;
}

void surface_t::set_center()
{
	point3_t ca(a, 1.0f);
	point3_t cb(b, 1.0f);
	point3_t cc(c, 1.0f);
	c_center = point3_t((ca[0] + cb[0] + cc[0]) / 3.0f, (ca[1] + cb[1] + cc[1]) / 3.0f, (ca[2] + cb[2] + cc[2]) / 3.0f);
	float r = std::sqrt(c_center[0] * c_center[0] + c_center[1] * c_center[1] + c_center[2] * c_center[2]);
	s_center = polar_t(180.0f * std::atan2(c_center[1], c_center[0]) / M_PI + 180.0f, 180.0f * std::asin(c_center[2] / r) / M_PI + 90.0f);
}

const biome_t surface_t::get_biome() const
{
	int _height = CLAMP(height * 6.0f, 0.0f, 5.0f);
	int _aridity = CLAMP(aridity * 5.0f + foehn * 0.25, 0.0f, 6.0f);
	return biome_map[_height][6 - _aridity];
}

const bool surface_t::operator<(const surface_t &f)
{
	return a < f.a || b < f.b || c < f.c;
}