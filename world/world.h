#pragma once

/* -------- OPTIONS --------- */

#define HEIGHT_MULTIPLIER		1.0
#define ARIDITY_MULTIPLIER		2.0
#define INLAND_LAKE_SIZE		64
#define ISLAND_SEED_COUNT		16
#define ISLAND_BRANCHING_SIZE	32
#define FACE_SIZE				1

/* -------------------------- */

#include <vector>

#include "../surface/surface.h"

struct section_t
{
	int lon, lat;
	std::vector<surface_t *> members;
	const bool operator==(const section_t &s) const;
};

constexpr inline double scale(const double &pit)
{
	return 1.0 / std::sin((M_PI * pit) / 180.0);
}

struct world_t
{
private:
	std::vector<surface_t *> faces;
	std::vector<landmass_t *> landmasses;
	section_t sections[36][18];
public:
	world_t(const int &);
	world_t(const std::vector<surface_t *> &);
	~world_t();
	const std::vector<section_t> expand(const std::vector<section_t> &input, const std::vector<section_t> &explored);
	bool iterate_rivers();
	surface_t *find_closest(const double &, const double &);
	std::pair<surface_t *, double> find_nearest(surface_t *, const surface_t::surface_type &);
	std::vector<surface_t *> get_lake_edges(surface_t *, std::vector<surface_t *> &);
	std::vector<surface_t *> get_water_extent(surface_t *);
	void make_landmasses(surface_t *);
	void iterate_land(surface_t *, int);
	void propagate_wind_east(surface_t *, double, const double &, std::vector<surface_t *> &);
	void propagate_wind_west(surface_t *, double, const double &, std::vector<surface_t *> &);
	void set_foehn();
	void stagnate_lake(const double &, surface_t *);
	std::vector<surface_t *> get_faces() const;
};