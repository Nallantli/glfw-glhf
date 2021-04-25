#pragma once

/* -------- OPTIONS --------- */

#define HEIGHT_MULTIPLIER		1.5f
#define ARIDITY_MULTIPLIER		3.0f
#define INLAND_LAKE_SIZE		64
#define ISLAND_BRANCHING_SIZE	10
#define FACE_SIZE				3

/* -------------------------- */

#include <vector>
#include "mesh.h"

bool borders_ocean(const face_t *);
bool iterate_rivers(const std::vector<face_t *> &);
bool sees_ocean(const float &, face_t *, std::vector<face_t *> &);
const bool does_share_side(const face_t *, const face_t *);
const float scale(const float &);
const float true_dist(const face_t *, const face_t *);
face_t *find_closest(const std::vector<face_t *> &, const float &, const float &);
face_t *get_highest_neighbor(face_t *);
face_t *get_lowest_neighbor(face_t *);
std::pair<face_t *, float> find_nearest_lake(face_t *, const std::vector<face_t *> &);
std::pair<face_t *, float> find_nearest_land(face_t *, const std::vector<face_t *> &);
std::pair<face_t *, float> find_nearest_ocean(face_t *, const std::vector<face_t *> &);
std::vector<face_t *> get_lake_edges(face_t *, std::vector<face_t *> &);
std::vector<face_t *> get_water_extent(face_t *, const std::vector<face_t *> &);
void generate_world(std::vector<face_t *> &);
void iterate_land(face_t *, int);
void propagate_wind_east(face_t *, float, const float &, std::vector<face_t *> &);
void propagate_wind_west(face_t *, float, const float &, std::vector<face_t *> &);
void set_foehn(const std::vector<face_t *> &);
void stagnate_lake(const float &, face_t *);