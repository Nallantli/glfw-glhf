#include "surface.h"
#include <algorithm>

surface_t::surface_t(const unsigned long long &ID, const polar_t &a, const polar_t &b, const polar_t &c)
	: ID{ ID }
	, a{ a }
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
	point3_t ca(a, 1.0);
	point3_t cb(b, 1.0);
	point3_t cc(c, 1.0);
	c_center = point3_t((ca[0] + cb[0] + cc[0]) / 3.0, (ca[1] + cb[1] + cc[1]) / 3.0, (ca[2] + cb[2] + cc[2]) / 3.0);
	double r = std::sqrt(c_center[0] * c_center[0] + c_center[1] * c_center[1] + c_center[2] * c_center[2]);
	s_center = polar_t(180.0 * std::atan2(c_center[1], c_center[0]) / M_PI + 180.0, 180.0 * std::asin(c_center[2] / r) / M_PI + 90.0);
}

const biome_t surface_t::get_biome() const
{
	switch (type) {
		case FACE_INLAND_LAKE:
			return RIVER;
		case FACE_STAGNANT:
			return LAKE;
		default:
			int _height = CLAMP<int>(height * 6.0, 0, 5);
			int _aridity = CLAMP<int>(aridity * 7.0 + foehn - 0.1, 0, 6);
			return biome_map[_height][6 - _aridity];
	}
}

const bool surface_t::operator<(const surface_t &f)
{
	return a < f.a || b < f.b || c < f.c;
}

const bool surface_t::does_share_side(const surface_t *b) const
{
	std::vector<polar_t> vertices = {
		this->a,
		this->b,
		this->c,
		b->a,
		b->b,
		b->c
	};

	if (std::count(vertices.begin(), vertices.end(), this->a) >= 2 && std::count(vertices.begin(), vertices.end(), this->b) >= 2)
		return true;
	if (std::count(vertices.begin(), vertices.end(), this->b) >= 2 && std::count(vertices.begin(), vertices.end(), this->c) >= 2)
		return true;
	if (std::count(vertices.begin(), vertices.end(), this->c) >= 2 && std::count(vertices.begin(), vertices.end(), this->a) >= 2)
		return true;
	return false;
}

std::vector<surface_t *> surface_t::get_lowest_neighbors() const
{
	std::vector<surface_t *> lowest;
	for (auto &n : this->neighbors) {
		if (n->type != surface_t::FACE_LAND)
			continue;
		if (lowest.empty()) {
			lowest = { n };
		} else if (n->height < lowest[0]->height) {
			lowest = { n };
		} else if (n->height == lowest[0]->height) {
			lowest.push_back(n);
		}
	}
	return lowest;
}

std::vector<surface_t *> surface_t::get_highest_neighbors() const
{
	std::vector<surface_t *> highest;
	for (auto &n : this->neighbors) {
		if (n->type != surface_t::FACE_LAND)
			continue;
		if (highest.empty()) {
			highest = { n };
		} else if (n->height > highest[0]->height) {
			highest = { n };
		} else if (n->height == highest[0]->height) {
			highest.push_back(n);
		}
	}
	return highest;
}

bool surface_t::borders_ocean() const
{
	for (auto &n : neighbors) {
		if (n->type == surface_t::FACE_OCEAN)
			return true;
	}
	return false;
}

bool surface_t::sees_ocean(const double &basin_height, std::vector<const surface_t *> &explored) const
{
	if (this->type == surface_t::FACE_OCEAN)
		return true;

	explored.push_back(this);
	for (auto &n : this->neighbors) {
		if (std::find(explored.begin(), explored.end(), n) == explored.end() && n->height <= basin_height && n->sees_ocean(basin_height, explored))
			return true;
	}
	return false;
}