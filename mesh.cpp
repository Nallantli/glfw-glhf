#include "mesh.h"
#include <iostream>

c_point::c_point(const float &x, const float &y, const float &z)
{
	coords(0) = x;
	coords(1) = y;
	coords(2) = z;
}

c_point::c_point()
{
	coords << 0, 0, 0;
}

c_point::c_point(const Eigen::Matrix<float, 3, 1> &coords) : coords{ coords }
{}

const float c_point::operator[](const size_t &i) const
{
	return coords(i);
}

c_point::c_point(const s_point &p, const float &r)
{
	coords(0) = r * DCOS(p[0]) * DSIN(p[1]);
	coords(1) = r * DSIN(p[0]) * DSIN(p[1]);
	coords(2) = r * DCOS(p[1]);
}

s_point::s_point()
{
	coords << 0, 0;
}

s_point::s_point(const float &yaw, const float &pit)
{
	coords(0) = yaw;
	coords(1) = pit;
}

const float s_point::operator[](const size_t &i) const
{
	return coords(i);
}

const bool s_point::operator==(const s_point &p) const
{
	return coords(0) == p[0] && coords(1) == p[1];
}

const bool s_point::operator!=(const s_point &p) const
{
	return coords(0) != p[0] || coords(1) != p[1];
}

face_t::face_t(const s_point &a, const s_point &b, const s_point &c) : a{ a }, b{ b }, c{ c }
{set_center();}

const s_point face_t::get_center() const
{
	return this->s_center;
}

const c_point face_t::get_center_c() const
{
	return this->c_center;
}

const bool face_t::operator==(const face_t &f)
{
	return f.a == a && f.b == b && f.c == c;
}

void face_t::set_center()
{
	c_point ca(a, 1.0f);
	c_point cb(b, 1.0f);
	c_point cc(c, 1.0f);
	c_center = c_point((ca[0] + cb[0] + cc[0]) / 3.0f, (ca[1] + cb[1] + cc[1]) / 3.0f, (ca[2] + cb[2] + cc[2]) / 3.0f);
	float r = std::sqrt(c_center[0] * c_center[0] + c_center[1] * c_center[1] + c_center[2] * c_center[2]);
	s_center = s_point(180.0f * std::atan2(c_center[1], c_center[0]) / M_PI + 180.0f, 180.0f * std::asin(c_center[2] / r) / M_PI + 90.0f);
}

const biome_t face_t::get_biome() const
{
	int _height = clamp(height * 6.0f, 0.0f, 5.0f);
	int _aridity = clamp(aridity * 5.0f + foehn * 0.25, 0.0f, 6.0f);
	return biome_map[_height][6 - _aridity];
}