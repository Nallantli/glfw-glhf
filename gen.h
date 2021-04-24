#ifndef GEN_H
#define GEN_H

/* -------- OPTIONS --------- */

#define HEIGHT_MULTIPLIER	2.0f
#define ARIDITY_MULTIPLIER	4.0f
#define INLAND_LAKE_SIZE	64
#define ISLAND_BRANCHING_SIZE	10
#define FACE_SIZE		3

/* -------------------------- */

#include<vector>
#include "mesh.h"

const float scale(const float &pit);
void generate_world(std::vector<face_t *> &set);
const float true_dist(const face_t *a, const face_t *b);
const bool does_share_side(const face_t *a, const face_t *b);
void iterate_land(face_t *curr, int w);
std::vector<face_t *> get_water_extent(face_t *f, const std::vector<face_t *> &faces);
std::pair<face_t *, float> find_nearest_lake(face_t *f, const std::vector<face_t *> &faces);
std::pair<face_t *, float> find_nearest_land(face_t *f, const std::vector<face_t *> &faces);
std::pair<face_t *, float> find_nearest_ocean(face_t *f, const std::vector<face_t *> &faces);
face_t *get_lowest_neighbor(face_t *f);
face_t *get_highest_neighbor(face_t *f);
bool borders_ocean(const face_t *f);
bool sees_ocean(const float basin_height, face_t *curr, std::vector<face_t *> &explored);
void stagnate_lake(const float basin_height, face_t *curr);
std::vector<face_t *> get_lake_edges(face_t *curr, std::vector<face_t *> &explored);
bool iterate_rivers(const std::vector<face_t *> &faces);
void propagate_wind_east(face_t *f, float p_factor, const float start_y, std::vector<face_t *> &explored);
void propagate_wind_west(face_t *f, float p_factor, const float start_y, std::vector<face_t *> &explored);
void set_foehn(const std::vector<face_t *> &set);
face_t *find_closest(const std::vector<face_t *> &faces, const float &yaw, const float &pit);

#endif