#pragma once

#include <vector>

#include "../point3/point3.h"
#include "../polar/polar.h"
#include "../biome/biome.h"

struct surface_t;
struct landmass_t;

struct surface_t
{
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

	float height = 0;
	float aridity = 0;
	float foehn = 0;

	std::vector<surface_t *> neighbors;

	landmass_t * landmass = NULL;

	surface_t(const polar_t &, const polar_t &, const polar_t &);
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
	float r, g, b;
	std::vector<surface_t *> members;
};