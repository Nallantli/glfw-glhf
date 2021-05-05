#pragma once

#include <vector>

#include "../point3/point3.h"
#include "../polar/polar.h"
#include "../biome/biome.h"

struct surface_t;
struct landmass_t;

struct surface_t
{
	const unsigned long long ID;

	const polar_t a;
	const polar_t b;
	const polar_t c;

	enum surface_type
	{
		FACE_LAND,
		FACE_WATER,
		FACE_OCEAN,
		FACE_FLOWING,
		FACE_STAGNANT,
		FACE_INLAND_LAKE,
	} type = FACE_WATER;

	double height = 0;
	double aridity = 0;
	double foehn = 0;

	std::vector<surface_t *> neighbors;

	landmass_t *landmass = NULL;

	surface_t(const unsigned long long &ID, const polar_t &, const polar_t &, const polar_t &);
	const bool operator==(const surface_t &);
	const bool operator<(const surface_t &);
	const polar_t get_center() const;
	const point3_t get_center_c() const;
	const biome_t get_biome() const;

private:
	polar_t s_center;
	point3_t c_center;
	void set_center();
};

struct landmass_t
{
	double r, g, b;
	std::vector<surface_t *> members;
};

inline double true_dist(const surface_t *a, const surface_t *b)
{
	return std::acos(glm::dot(a->get_center_c().coords, b->get_center_c().coords));
}

const bool does_share_side(const surface_t *, const surface_t *);
std::vector<surface_t *> get_highest_neighbors(surface_t *);
std::vector<surface_t *> get_lowest_neighbors(surface_t *);
bool borders_ocean(const surface_t *);
bool sees_ocean(const double &, surface_t *, std::vector<surface_t *> &);