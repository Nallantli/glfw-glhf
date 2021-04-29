#pragma once

/* -------- OPTIONS --------- */

#define HEIGHT_MULTIPLIER		1.0f
#define ARIDITY_MULTIPLIER		1.0f
#define INLAND_LAKE_SIZE		32
#define ISLAND_BRANCHING_SIZE	16
#define FACE_SIZE				2

/* -------------------------- */

#include <vector>

#include "../surface/surface.h"

bool borders_ocean(const surface_t *);
bool iterate_rivers(const std::vector<surface_t *> &);
bool sees_ocean(const float &, surface_t *, std::vector<surface_t *> &);
const bool does_share_side(const surface_t *, const surface_t *);
const float scale(const float &);
const float true_dist(const surface_t *, const surface_t *);
surface_t *find_closest(const std::vector<surface_t *> &, const float &, const float &);
surface_t *get_highest_neighbor(surface_t *);
surface_t *get_lowest_neighbor(surface_t *);
std::pair<surface_t *, float> find_nearest_lake(surface_t *, const std::vector<surface_t *> &);
std::pair<surface_t *, float> find_nearest_land(surface_t *, const std::vector<surface_t *> &);
std::pair<surface_t *, float> find_nearest_ocean(surface_t *, const std::vector<surface_t *> &);
std::vector<surface_t *> get_lake_edges(surface_t *, std::vector<surface_t *> &);
std::vector<surface_t *> get_water_extent(surface_t *, const std::vector<surface_t *> &);
void generate_world(std::vector<surface_t *> &, std::vector<landmass_t *> &, const int &);
void make_landmasses(surface_t *);
void iterate_land(surface_t *, int);
void propagate_wind_east(surface_t *, float, const float &, std::vector<surface_t *> &);
void propagate_wind_west(surface_t *, float, const float &, std::vector<surface_t *> &);
void set_foehn(const std::vector<surface_t *> &);
void stagnate_lake(const float &, surface_t *);