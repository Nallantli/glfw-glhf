#include "generator.h"

#include <glm/glm.hpp>
#include <algorithm>
#include <chrono>
#include <map>

#include "../quickhull/QuickHull.hpp"
#include "../perlin/perlin.hpp"

const float scale(const float &pit)
{
	return 1.0 / std::sin((M_PI * pit) / 180.0f);
}

const float true_dist(const surface_t *a, const surface_t *b)
{
	point3_t cca = a->get_center_c();
	point3_t ccb = b->get_center_c();
	auto x = glm::dot(cca.coords, ccb.coords);
	return std::acos(x);
}

const bool does_share_side(const surface_t *a, const surface_t *b)
{
	std::vector<polar_t> vertices = {
		a->a,
		a->b,
		a->c,
		b->a,
		b->b,
		b->c
	};

	if (std::count(vertices.begin(), vertices.end(), a->a) >= 2 && std::count(vertices.begin(), vertices.end(), a->b) >= 2)
		return true;
	if (std::count(vertices.begin(), vertices.end(), a->b) >= 2 && std::count(vertices.begin(), vertices.end(), a->c) >= 2)
		return true;
	if (std::count(vertices.begin(), vertices.end(), a->c) >= 2 && std::count(vertices.begin(), vertices.end(), a->a) >= 2)
		return true;
	return false;
}

void iterate_land(surface_t *curr, int w)
{
	if (curr->type == surface_t::FACE_LAND)
		return;
	curr->type = surface_t::FACE_LAND;
	if (w > 0) {
		for (auto &n : curr->neighbors) {
			if (n->type == surface_t::FACE_WATER)
				iterate_land(n, w - 1);
		}
	}
}

std::vector<surface_t *> get_water_extent(surface_t *f, const std::vector<surface_t *> &faces)
{
	std::vector<surface_t *> closed;
	std::vector<surface_t *> open = { f };

	while (!open.empty()) {
		surface_t *curr = open.back();
		open.erase(std::remove(open.begin(), open.end(), curr), open.end());
		closed.push_back(curr);
		for (auto &n : curr->neighbors) {
			if (n->type == surface_t::FACE_WATER
				&& std::find(closed.begin(), closed.end(), n) == closed.end()
				&& std::find(open.begin(), open.end(), n) == open.end())
				open.push_back(n);
		}
	}

	return closed;
}

std::pair<surface_t *, float> find_nearest_land(surface_t *f, const std::vector<surface_t *> &faces)
{
	if (f->type == surface_t::FACE_LAND)
		return { f, 0 };
	surface_t *min = NULL;
	float d;
	for (auto &s : faces) {
		if (s->type != surface_t::FACE_LAND)
			continue;
		float t = true_dist(f, s);
		if (min == NULL || t < d) {
			d = t;
			min = s;
		}
	}
	return { min, d };
}

std::pair<surface_t *, float> find_nearest_ocean(surface_t *f, const std::vector<surface_t *> &faces)
{
	if (f->type == surface_t::FACE_WATER)
		return { f, 0 };
	surface_t *min = NULL;
	float d;
	for (auto &s : faces) {
		if (s->type != surface_t::FACE_WATER)
			continue;
		float t = true_dist(f, s);
		if (min == NULL || t < d) {
			d = t;
			min = s;
		}
	}
	return { min, d };
}

surface_t *get_lowest_neighbor(surface_t *f)
{
	surface_t *lowest = NULL;
	for (auto &n : f->neighbors) {
		if (n->type != surface_t::FACE_LAND)
			continue;
		if (lowest == NULL) {
			if (n->height < f->height)
				lowest = n;
		} else if (n->height < lowest->height)
			lowest = n;
	}
	return lowest;
}

surface_t *get_highest_neighbor(surface_t *f)
{
	surface_t *highest = NULL;
	for (auto &n : f->neighbors) {
		if (n->type != surface_t::FACE_LAND)
			continue;
		if (highest == NULL) {
			if (n->height > f->height)
				highest = n;
		} else if (n->height > highest->height)
			highest = n;
	}
	return highest;
}

bool borders_ocean(const surface_t *f)
{
	for (auto &n : f->neighbors) {
		if (n->type == surface_t::FACE_OCEAN)
			return true;
	}
	return false;
}

bool sees_ocean(const float &basin_height, surface_t *curr, std::vector<surface_t *> &explored)
{
	if (curr->type == surface_t::FACE_OCEAN)
		return true;

	explored.push_back(curr);
	for (auto &n : curr->neighbors) {
		if (std::find(explored.begin(), explored.end(), n) == explored.end() && n->height < basin_height && sees_ocean(basin_height, n, explored))
			return true;
	}
	return false;
}

std::pair<surface_t *, float> find_nearest_lake(surface_t *f, const std::vector<surface_t *> &faces)
{
	if (f->type == surface_t::FACE_INLAND_LAKE)
		return { f, 0 };
	surface_t *min = NULL;
	float d;
	for (auto &s : faces) {
		if (s->type != surface_t::FACE_INLAND_LAKE)
			continue;
		float t = true_dist(f, s);
		if (min == NULL || t < d) {
			d = t;
			min = s;
		}
	}
	return { min, d };
}

void stagnate_lake(const float &basin_height, surface_t *curr)
{
	curr->type = surface_t::FACE_STAGNANT;
	for (auto &n : curr->neighbors) {
		if (n->height < basin_height && n->type == surface_t::FACE_LAND)
			stagnate_lake(basin_height, n);
	}
}

std::vector<surface_t *> get_lake_edges(surface_t *curr, std::vector<surface_t *> &explored)
{
	explored.push_back(curr);
	std::vector<surface_t *> edges;
	bool flag = false;
	for (auto &n : curr->neighbors) {
		if (n->type == surface_t::FACE_LAND) {
			flag = true;
		} else if (std::find(explored.begin(), explored.end(), n) == explored.end() && n->type == surface_t::FACE_STAGNANT) {
			auto ex2 = get_lake_edges(n, explored);
			for (auto &e : ex2)
				edges.push_back(e);
		}
	}
	if (flag)
		edges.push_back(curr);
	return edges;
}

bool iterate_rivers(const std::vector<surface_t *> &faces)
{
	std::vector<surface_t *> rivers;
	for (auto &f : faces) {
		if (f->type == surface_t::FACE_FLOWING)
			rivers.push_back(f);
	}

	if (rivers.empty())
		return false;

	for (auto &s : rivers) {
		if (borders_ocean(s)) {
			s->type = surface_t::FACE_INLAND_LAKE;
		} else {
			auto ln = get_lowest_neighbor(s);
			if (ln != NULL) {
				std::vector<surface_t *> ex;
				s->type = surface_t::FACE_INLAND_LAKE;
				if (!sees_ocean(ln->height, ln, ex)) {
					ex.clear();
					stagnate_lake(ln->height, ln);
					std::vector<surface_t *> possible = get_lake_edges(ln, ex);
					for (auto &e : possible) {
						auto out = get_highest_neighbor(e);
						if (out != NULL)
							out->type = surface_t::FACE_FLOWING;
					}
				} else if (ln->height < s->height) {
					ln->type = surface_t::FACE_FLOWING;
				}
			} else {
				s->type = surface_t::FACE_INLAND_LAKE;
			}
		}
	}
	return true;
}

void propagate_wind_east(surface_t *f, float p_factor, const float &start_y, std::vector<surface_t *> &explored)
{
	explored.push_back(f);
	f->foehn += p_factor;
	p_factor -= 0.1;
	if (p_factor < 0)
		return;
	for (auto &n : f->neighbors) {
		if (
			((n->get_center()[0] > f->get_center()[0] && std::abs(n->get_center()[0] - f->get_center()[0]) <= 10) || (n->get_center()[0] < f->get_center()[0] && std::abs(n->get_center()[0] - f->get_center()[0]) > 10))
			&& n->height <= f->height && n->type == surface_t::FACE_LAND && std::find(explored.begin(), explored.end(), n) == explored.end()
			) {
			propagate_wind_east(
				n,
				p_factor * MAX(0, -std::sqrt(std::abs(n->get_center()[1] - start_y) / 10.0f) + 1.0f),
				start_y,
				explored
			);
		}
	}
}

void propagate_wind_west(surface_t *f, float p_factor, const float &start_y, std::vector<surface_t *> &explored)
{
	explored.push_back(f);
	f->foehn += p_factor;
	p_factor -= 0.1;
	if (p_factor < 0)
		return;
	for (auto &n : f->neighbors) {
		if (
			((n->get_center()[0] < f->get_center()[0] && std::abs(n->get_center()[0] - f->get_center()[0]) <= 10) || (n->get_center()[0] > f->get_center()[0] && std::abs(n->get_center()[0] - f->get_center()[0]) > 10))
			&& n->height <= f->height && n->type == surface_t::FACE_LAND && std::find(explored.begin(), explored.end(), n) == explored.end()
			) {
			propagate_wind_west(
				n,
				p_factor * MAX(0, -std::sqrt(std::abs(n->get_center()[1] - start_y) / 10.0f) + 1.0f),
				start_y,
				explored
			);
		}
	}
}

void set_foehn(const std::vector<surface_t *> &set)
{
	for (auto &f : set) {
		if (f->type == surface_t::FACE_LAND) {
			float w_factor = CLAMP(std::pow(DSIN(3.0f * (f->get_center()[1] - 90.0f)), 2) / DCOS(3.0f * (f->get_center()[1] - 90.0f)), -1, 1) / 2.0f;
			float h_factor = f->height;
			float p_factor = w_factor * h_factor;
			std::vector<surface_t *> explored;
			if (p_factor > 0) {
				propagate_wind_east(f, p_factor, f->get_center()[1], explored);
			} else {
				propagate_wind_west(f, -p_factor, f->get_center()[1], explored);
			}
		}
	}
}

void make_landmasses(surface_t *curr)
{
	for (auto &n : curr->neighbors) {
		if (n->type != surface_t::FACE_LAND || n->landmass != NULL)
			continue;
		n->landmass = curr->landmass;
		make_landmasses(n);
	}
}

void generate_world(std::vector<surface_t *> &set, std::vector<landmass_t *> &landmasses, const int &SEED)
{
	perlin p(SEED);
	std::vector<polar_t> ps;

	float size = (float)FACE_SIZE;

	for (int i = size; i <= 180 - size; i += size) {
		for (float j = size; j < 360;) {
			float x = j + ((float)rand() / (float)RAND_MAX) * (size / 2.0f);
			float y = (float)i + ((float)rand() / (float)RAND_MAX) * (size / 2.0f);
			ps.push_back(polar_t(x, y));
			j += scale(i) * size;
		}
	}

	ps.push_back(polar_t(0, 0));
	ps.push_back(polar_t(0, 180));

	quickhull::QuickHull<float> qh;
	std::vector<quickhull::Vector3<float>> qhpoints;

	for (auto &p : ps) {
		point3_t c(p, 1);
		qhpoints.push_back({
			c[0],
			c[1],
			c[2]
			});
	}

	std::cout << "Building convex hull...\n";
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	auto hull = qh.getConvexHull(qhpoints, true, false);
	const auto &indexBuffer = hull.getIndexBuffer();
	const auto &vertexBuffer = hull.getVertexBuffer();
	qhpoints.clear();
	ps.clear();
	std::vector<polar_t> translated_vertices;
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	std::cout << "Elapsed: "
		<< std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()
		<< "[us]" << std::endl;

	std::cout << "Translating vertices...\n";
	begin = std::chrono::steady_clock::now();

	for (auto &v : vertexBuffer) {
		float r = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
		translated_vertices.push_back(
			polar_t(180.0f * std::atan2(v.y, v.x) / M_PI + 180.0f, 180.0f * std::asin(v.z / r) / M_PI + 90.0f)
		);
	}

	end = std::chrono::steady_clock::now();
	std::cout << "Elapsed: "
		<< std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()
		<< "[us]" << std::endl;

	std::cout << "Building triangle surfaces...\n";
	begin = std::chrono::steady_clock::now();

	std::map<std::pair<polar_t, polar_t>, std::vector<surface_t *>> edge_map;

	unsigned long long count = 0;
	for (unsigned int i = 0; i < indexBuffer.size(); i += 3) {
		std::vector<polar_t> ps = {
			translated_vertices[indexBuffer[i]],
			translated_vertices[indexBuffer[i + 1]],
			translated_vertices[indexBuffer[i + 2]] };
		surface_t *s;
		if (ps[0][0] < ps[1][0] && ps[0][0] < ps[2][0]) {
			s = new surface_t{
			count,
			ps[0],
			ps[1],
			ps[2] };
		} else if (ps[1][0] < ps[0][0] && ps[1][0] < ps[2][0]) {
			s = new surface_t{
			count,
			ps[1],
			ps[2],
			ps[0] };
		} else {
			s = new surface_t{
			count,
			ps[2],
			ps[0],
			ps[1] };
		}

		if (s->a < s->b)
			edge_map[{s->a, s->b}].push_back(s);
		else
			edge_map[{s->b, s->a}].push_back(s);
		if (s->b < s->c)
			edge_map[{s->b, s->c}].push_back(s);
		else
			edge_map[{s->c, s->b}].push_back(s);
		if (s->c < s->a)
			edge_map[{s->c, s->a}].push_back(s);
		else
			edge_map[{s->a, s->c}].push_back(s);
		set.push_back(s);
		count++;
	}

	translated_vertices.clear();
	end = std::chrono::steady_clock::now();
	std::cout << "Elapsed: "
		<< std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()
		<< "[us]" << std::endl;

	std::cout << "Setting neighbors...\n";
	begin = std::chrono::steady_clock::now();
	for (auto &e : edge_map) {
		for (auto &s : e.second) {
			for (auto &s2 : e.second) {
				if (s == s2)
					continue;
				s->neighbors.push_back(s2);
			}
		}
	}
	edge_map.clear();
	end = std::chrono::steady_clock::now();
	std::cout << "Elapsed: "
		<< std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()
		<< "[us]" << std::endl;

	std::cout << "Setting Islands...\n";
	begin = std::chrono::steady_clock::now();
	for (auto i = 0; i < 16; i++) {
		auto origin = set[rand() % set.size()];
		float size = ((float)rand() / (float)RAND_MAX) * 0.4f + 0.1f;
		for (auto &e : set) {
			if (true_dist(origin, e) < size)
				iterate_land(e, ISLAND_BRANCHING_SIZE);
		}
	}
	end = std::chrono::steady_clock::now();
	std::cout << "Elapsed: "
		<< std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()
		<< "[us]" << std::endl;

	std::cout << "Setting Height Map...\n";
	begin = std::chrono::steady_clock::now();
	for (auto &f : set) {
		if (f->type != surface_t::FACE_LAND)
			continue;
		std::pair<surface_t *, float> n = find_nearest_ocean(f, set);
		point3_t cc = f->get_center_c();
		float pm =
			p.get(p.get((cc[0] + 1.0f) / 32.0f, (cc[1] + 1.0f) / 32.0f, 2.0f, 4), (cc[2] + 1.0f) / 32.0f, 2.0f, 4) +
			p.get(p.get((cc[0] + 1.0f) / 16.0f, (cc[1] + 1.0f) / 16.0f, 4.0f, 4), (cc[2] + 1.0f) / 16.0f, 4.0f, 4) +
			p.get(p.get((cc[0] + 1.0f) / 8.0f, (cc[1] + 1.0f) / 8.0f, 8.0f, 4), (cc[2] + 1.0f) / 8.0f, 8.0f, 4) +
			p.get(p.get((cc[0] + 1.0f) / 4.0f, (cc[1] + 1.0f) / 4.0f, 16.0f, 4), (cc[2] + 1.0f) / 4.0f, 16.0f, 4);
		f->height = (n.second * 1.8f + MAX(0, pm / 5.0f - 0.25f)) * HEIGHT_MULTIPLIER;
	}
	end = std::chrono::steady_clock::now();
	std::cout << "Elapsed: "
		<< std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()
		<< "[us]" << std::endl;

	std::cout << "Setting Water Types...\n";
	begin = std::chrono::steady_clock::now();
	for (auto &f : set) {
		if (f->type != surface_t::FACE_WATER)
			continue;
		auto water = get_water_extent(f, set);
		if (water.size() < INLAND_LAKE_SIZE) {
			for (auto &e : water) {
				e->height = find_nearest_land(e, set).first->height;
				e->type = surface_t::FACE_LAND;
			}
		} else {
			for (auto &e : water) {
				e->type = surface_t::FACE_OCEAN;
			}
		}
	}
	end = std::chrono::steady_clock::now();
	std::cout << "Elapsed: "
		<< std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()
		<< "[us]" << std::endl;

	std::cout << "Setting Springs...\n";
	begin = std::chrono::steady_clock::now();
	for (auto &f : set) {
		if (f->type != surface_t::FACE_LAND || (f->height < 0.5f && f->height > 0.6f)) {
			continue;
		}
		if (rand() % 64 == 0)
			f->type = surface_t::FACE_FLOWING;
	}
	end = std::chrono::steady_clock::now();
	std::cout << "Elapsed: "
		<< std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()
		<< "[us]" << std::endl;

	std::cout << "Setting Rivers...\n";
	begin = std::chrono::steady_clock::now();
	//iterate_rivers(set);
	while (iterate_rivers(set));

	for (auto &f : set) {
		if (f->type == surface_t::FACE_STAGNANT)
			f->type = surface_t::FACE_INLAND_LAKE;
	}
	end = std::chrono::steady_clock::now();
	std::cout << "Elapsed: "
		<< std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()
		<< "[us]" << std::endl;

	std::cout << "Setting Aridity Map...\n";
	begin = std::chrono::steady_clock::now();
	for (auto &f : set) {
		if (f->type != surface_t::FACE_LAND)
			continue;
		std::pair<surface_t *, float> n = find_nearest_lake(f, set);
		point3_t cc = f->get_center_c();
		float pm =
			p.get(p.get((cc[0] + 101.0f) / 32.0f, (cc[1] + 101.0f) / 32.0f, 2.0f, 4), (cc[2] + 101.0f) / 32.0f, 2.0f, 4) +
			p.get(p.get((cc[0] + 101.0f) / 16.0f, (cc[1] + 101.0f) / 16.0f, 4.0f, 4), (cc[2] + 101.0f) / 16.0f, 4.0f, 4) +
			p.get(p.get((cc[0] + 101.0f) / 8.0f, (cc[1] + 101.0f) / 8.0f, 8.0f, 4), (cc[2] + 101.0f) / 8.0f, 8.0f, 4) +
			p.get(p.get((cc[0] + 101.0f) / 4.0f, (cc[1] + 101.0f) / 4.0f, 16.0f, 4), (cc[2] + 101.0f) / 4.0f, 16.0f, 4);
		f->aridity = (n.second * 1.8f + MAX(0, pm / 5.0f - 0.25f)) * ARIDITY_MULTIPLIER;
	}
	end = std::chrono::steady_clock::now();
	std::cout << "Elapsed: "
		<< std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()
		<< "[us]" << std::endl;

	std::cout << "Setting Foehn Map...\n";
	begin = std::chrono::steady_clock::now();
	set_foehn(set);
	end = std::chrono::steady_clock::now();
	std::cout << "Elapsed: "
		<< std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()
		<< "[us]" << std::endl;

	std::cout << "Setting Landmass Map...\n";
	begin = std::chrono::steady_clock::now();
	for (auto &s : set) {
		if (s->landmass == NULL) {
			landmass_t *l = new landmass_t{
				(float)rand() / (float)RAND_MAX,
				(float)rand() / (float)RAND_MAX,
				(float)rand() / (float)RAND_MAX
			};
			s->landmass = l;
			landmasses.push_back(l);
			l->members.push_back(s);
			make_landmasses(s);
		}
	}
	end = std::chrono::steady_clock::now();
	std::cout << "Elapsed: "
		<< std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()
		<< "[us]" << std::endl;

	std::cout << "---------------------------------\n";
	std::cout << "Face Count: " << set.size() << "\n";
	std::cout << "---------------------------------\n";

	std::cout << "Done.\n";
}

surface_t *find_closest(const std::vector<surface_t *> &faces, const float &yaw, const float &pit)
{
	float d = 100.0f;
	surface_t *curr = NULL;

	for (auto &f : faces) {
		auto center = f->get_center();

		float t = std::sqrt(std::pow(center[0] - yaw, 2) + std::pow(center[1] - pit, 2));
		if (t < d) {
			curr = f;
			d = t;
		}
	}

	return curr;
}