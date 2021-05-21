#include "world.h"

#include <glm/glm.hpp>
#include <algorithm>
#include <chrono>
#include <map>

#include "../quickhull/QuickHull.hpp"
#include "../SimplexNoise/SimplexNoise.h"

void world_t::iterate_land(surface_t *curr, int w)
{
	if (curr->type == surface_t::FACE_LAND)
		return;
	curr->type = surface_t::FACE_LAND;
	if (w > 0) {
		for (auto &n : curr->neighbors) {
			if (n->type == surface_t::FACE_WATER || n->type == surface_t::FACE_OCEAN || n->type == surface_t::FACE_DEEP_OCEAN)
				iterate_land(n, w - 1);
		}
	}
}

std::vector<surface_t *> world_t::get_water_extent(surface_t *f)
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

std::pair<surface_t *, double> world_t::find_nearest(surface_t *f, const surface_t::surface_type &type)
{
	if (f->type == type)
		return { f, 0 };

	std::vector<section_t> curr = { sections[(size_t)(f->get_center()[0] / 10.0)][(size_t)(f->get_center()[1] / 10.0)] };
	std::vector<section_t> explored;

	bool check_flag = false;
	surface_t *min = NULL;
	double d = INFINITY;

	while (!curr.empty()) {
		for (auto &sub : curr) {
			for (auto &s : sub.members) {
				if (s->type != type)
					continue;
				double t = true_dist(f, s);
				if (min == NULL || t < d) {
					d = t;
					min = s;
				}
			}
			explored.push_back(sub);
		}
		if (min != NULL && check_flag)
			return { min, d };
		else if (min != NULL)
			check_flag = true;
		curr = expand(curr, explored);
	}
	return { min, d };
}

void world_t::stagnate_lake(const double &basin_height, surface_t *curr)
{
	curr->type = surface_t::FACE_STAGNANT;
	for (auto &n : curr->neighbors) {
		if (n->height <= basin_height && n->type == surface_t::FACE_LAND)
			stagnate_lake(basin_height, n);
	}
}

std::vector<surface_t *> world_t::get_lake_edges(surface_t *curr, std::vector<const surface_t *> &explored)
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

bool world_t::iterate_rivers()
{
	std::vector<surface_t *> rivers;
	for (auto &f : faces) {
		if (f->type == surface_t::FACE_FLOWING)
			rivers.push_back(f);
	}

	if (rivers.empty())
		return false;

	for (auto &s : rivers) {
		if (s->borders_ocean()) {
			s->type = surface_t::FACE_INLAND_LAKE;
		} else {
			auto lv = s->get_lowest_neighbors();
			if (!lv.empty()) {
				surface_t *ln = NULL;
				for (auto &e : lv) {
					if (ln == NULL || find_nearest(e, surface_t::FACE_OCEAN).second < find_nearest(ln, surface_t::FACE_OCEAN).second)
						ln = e;
				}
				std::vector<const surface_t *> ex;
				s->type = surface_t::FACE_INLAND_LAKE;
				if (!ln->sees_ocean(ln->height, ex)) {
					ex.clear();
					stagnate_lake(ln->height, ln);
					std::vector<surface_t *> possible = get_lake_edges(ln, ex);
					for (auto &e : possible) {
						auto hv = e->get_highest_neighbors();
						if (!hv.empty()) {
							surface_t *hn = NULL;
							for (auto &e : hv) {
								if (hn == NULL || find_nearest(e, surface_t::FACE_OCEAN).second > find_nearest(hn, surface_t::FACE_OCEAN).second)
									hn = e;
							}
							hn->type = surface_t::FACE_FLOWING;
						}
					}
				} else if (ln->height <= s->height) {
					ln->type = surface_t::FACE_FLOWING;
				} else {
					s->type = surface_t::FACE_INLAND_LAKE;
				}
			} else {
				s->type = surface_t::FACE_INLAND_LAKE;
			}
		}
	}
	return true;
}

void world_t::propagate_wind_east(surface_t *f, double p_factor, const double &start_y, std::vector<const surface_t *> &explored)
{
	explored.push_back(f);
	f->foehn += p_factor;
	p_factor -= 0.025;
	if (p_factor < 0)
		return;
	for (auto &n : f->neighbors) {
		if (
			((n->get_center()[0] > f->get_center()[0] && std::abs(n->get_center()[0] - f->get_center()[0]) <= 10) || (n->get_center()[0] < f->get_center()[0] && std::abs(n->get_center()[0] - f->get_center()[0]) > 10))
			&& n->height < f->height && n->type != surface_t::FACE_INLAND_LAKE && std::find(explored.begin(), explored.end(), n) == explored.end()
			) {
			propagate_wind_east(
				n,
				p_factor * MAX<double>(0, -std::sqrt(std::abs(n->get_center()[1] - start_y) / 10.0) + 1.0),
				start_y,
				explored
			);
		}
	}
}

void world_t::propagate_wind_west(surface_t *f, double p_factor, const double &start_y, std::vector<const surface_t *> &explored)
{
	explored.push_back(f);
	f->foehn += p_factor;
	p_factor -= 0.025;
	if (p_factor < 0)
		return;
	for (auto &n : f->neighbors) {
		if (
			((n->get_center()[0] < f->get_center()[0] && std::abs(n->get_center()[0] - f->get_center()[0]) <= 10) || (n->get_center()[0] > f->get_center()[0] && std::abs(n->get_center()[0] - f->get_center()[0]) > 10))
			&& n->height < f->height && n->type != surface_t::FACE_INLAND_LAKE && std::find(explored.begin(), explored.end(), n) == explored.end()
			) {
			propagate_wind_west(
				n,
				p_factor * MAX<double>(0, -std::sqrt(std::abs(n->get_center()[1] - start_y) / 10.0) + 1.0),
				start_y,
				explored
			);
		}
	}
}

void world_t::set_foehn()
{
	for (auto &f : faces) {
		if (f->type == surface_t::FACE_LAND) {
			double w_factor = CLAMP<double>(std::pow(DSIN(3.0 * (f->get_center()[1] - 90.0)), 2) / DCOS(3.0 * (f->get_center()[1] - 90.0)), -1, 1) / 2.0;
			double h_factor = std::pow(f->height, 1.25) * 1.0;
			double p_factor = w_factor * h_factor;
			std::vector<const surface_t *> explored;
			if (p_factor > 0) {
				propagate_wind_east(f, p_factor, f->get_center()[1], explored);
			} else {
				propagate_wind_west(f, -p_factor, f->get_center()[1], explored);
			}
		}
	}
}

void world_t::make_landmasses(surface_t *curr)
{
	for (auto &n : curr->neighbors) {
		if (n->type != surface_t::FACE_LAND || n->landmass != NULL)
			continue;
		n->landmass = curr->landmass;
		make_landmasses(n);
	}
}

const std::vector<section_t> world_t::expand(const std::vector<section_t> &input, const std::vector<section_t> &explored)
{
	std::vector<section_t> output;
	for (auto &s : input) {
		for (int j = -1; j <= 1; j++) {
			int lat = MAX<int>(0, MIN<int>(17, s.lat + j));
			int sc = MAX<int>(0, MIN<int>(18, scale(lat * 10)));
			for (int i = -sc; i <= sc; i++) {
				if (i == 0 && j == 0)
					continue;
				int lon = (s.lon + i + 36) % 36;
				if (std::find(output.begin(), output.end(), sections[lon][lat]) == output.end()
					&& std::find(explored.begin(), explored.end(), sections[lon][lat]) == explored.end())
					output.push_back(sections[lon][lat]);
			}
		}
	}

	return output;
}


const bool section_t::operator==(const section_t &s) const
{
	return lon == s.lon && lat == s.lat;
}

world_t::world_t(const int &SEED)
{
	for (int i = 0; i < 36; i++) {
		for (int j = 0; j < 18; j++) {
			sections[i][j].lon = i;
			sections[i][j].lat = j;
		}
	}

	int noise_offset = rand();
	std::vector<polar_t> ps;

	double size = (double)FACE_SIZE;

	for (int i = size; i <= 180 - size; i += size) {
		for (double j = size; j < 360;) {
			double x = j + ((double)rand() / (double)RAND_MAX) * (size / 2.0);
			double y = (double)i + ((double)rand() / (double)RAND_MAX) * (size / 2.0);
			ps.push_back(polar_t(x, y));
			j += scale(i) * size;
		}
	}

	ps.push_back(polar_t(0, 0));
	ps.push_back(polar_t(0, 180));

	quickhull::QuickHull<double> qh;
	std::vector<quickhull::Vector3<double>> qhpoints;

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
		double r = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
		translated_vertices.push_back(
			polar_t(180.0 * std::atan2(v.y, v.x) / M_PI + 180.0, 180.0 * std::asin(v.z / r) / M_PI + 90.0)
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
		faces.push_back(s);

		sections[(size_t)(s->get_center()[0] / 10.0f)][(size_t)(s->get_center()[1] / 10.0f)].members.push_back(s);

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
	for (auto i = 0; i < ISLAND_SEED_COUNT; i++) {
		auto origin = faces[rand() % faces.size()];
		double size = ((double)rand() / (double)RAND_MAX) * 0.4 + 0.1;
		for (auto &e : faces) {
			if (true_dist(origin, e) < size)
				iterate_land(e, ISLAND_BRANCHING_SIZE);
		}
	}
	end = std::chrono::steady_clock::now();
	std::cout << "Elapsed: "
		<< std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()
		<< "[us]" << std::endl;

	std::cout << "Setting Deep Ocean Islands...\n";
	std::vector<surface_t *> deep;
	std::vector<surface_t *> deep2;
	for (auto &f : faces) {
		if (f->type != surface_t::FACE_WATER)
			continue;
		auto dist = find_nearest(f, surface_t::FACE_LAND);
		point3_t cc = f->get_center_c();
		double pm = SimplexNoise::noise(200 + noise_offset + cc[0] / 2.0, cc[1] / 2.0, cc[2] / 2.0);
		double pm2 = SimplexNoise::noise(400 + noise_offset + cc[0] / 2.0, cc[1] / 2.0, cc[2] / 2.0);
		if (dist.second > 0.2) {
			if (pm > -0.1 && pm < 0.1) {
				f->type = surface_t::FACE_DEEP_OCEAN;
				deep.push_back(f);
			} else if (pm2 > -0.1 && pm2 < 0.1) {
				f->type = surface_t::FACE_DEEP_OCEAN;
				deep2.push_back(f);
			}
		}
	}
	begin = std::chrono::steady_clock::now();
	std::random_shuffle(deep.begin(), deep.end());

	std::vector<surface_t *> roots;

	for (int i = 0; i < 64; i++) {
		if (deep.empty())
			break;
		auto root = deep.back();
		deep.pop_back();
		point3_t cc = root->get_center_c();
		double pm = SimplexNoise::noise(300 + noise_offset + cc[0], cc[1], cc[2]);
		if (root->type != surface_t::FACE_DEEP_OCEAN || pm < 0) {
			i--;
			continue;
		}
		roots.push_back(root);
		iterate_land(root, (double)rand() / (double)RAND_MAX * 4.0);
	}

	for (int i = 0; i < 32; i++) {
		if (deep2.empty())
			break;
		auto root = deep2.back();
		deep2.pop_back();
		point3_t cc = root->get_center_c();
		double pm = SimplexNoise::noise(500 + noise_offset + cc[0], cc[1], cc[2]);
		if (root->type != surface_t::FACE_DEEP_OCEAN || pm < 0) {
			i--;
			continue;
		}
		roots.push_back(root);
		iterate_land(root, (double)rand() / (double)RAND_MAX * 8.0);
	}

	for (auto &f : deep) {
		if (f->type == surface_t::FACE_DEEP_OCEAN)
			f->type = surface_t::FACE_WATER;
	}

	for (auto &f : deep2) {
		if (f->type == surface_t::FACE_DEEP_OCEAN)
			f->type = surface_t::FACE_WATER;
	}

	deep.clear();
	deep2.clear();
	end = std::chrono::steady_clock::now();
	std::cout << "Elapsed: "
		<< std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()
		<< "[us]" << std::endl;

	std::cout << "Setting Height Map...\n";
	begin = std::chrono::steady_clock::now();
	for (auto &f : faces) {
		if (f->type != surface_t::FACE_LAND)
			continue;
		std::pair<surface_t *, double> n = find_nearest(f, surface_t::FACE_WATER);
		point3_t cc = f->get_center_c();
		double pm =
			SimplexNoise::noise(noise_offset + cc[0], cc[1], cc[2]) * 0.5 +
			SimplexNoise::noise(noise_offset + cc[0] * 2.0, cc[1] * 2.0, cc[2] * 2.0) * 0.25 +
			SimplexNoise::noise(noise_offset + cc[0] * 4.0, cc[1] * 4.0, cc[2] * 4.0) * 0.15 +
			SimplexNoise::noise(noise_offset + cc[0] * 8.0, cc[1] * 8.0, cc[2] * 8.0) * 0.1;
		f->height = MAX<double>(0.0, n.second * 2.0 + pm / 3.0) * HEIGHT_MULTIPLIER;
	}
	end = std::chrono::steady_clock::now();
	std::cout << "Elapsed: "
		<< std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()
		<< "[us]" << std::endl;

	std::cout << "Setting Water Types...\n";
	begin = std::chrono::steady_clock::now();

	for (auto &f : roots) {
		if (!f->borders_ocean() && rand() % 2 == 0)
			f->type = surface_t::FACE_FLOWING;
	}
	roots.clear();
	for (auto &f : faces) {
		if (f->type != surface_t::FACE_WATER)
			continue;
		auto water = get_water_extent(f);
		if (water.size() < INLAND_LAKE_SIZE) {
			for (auto &e : water) {
				e->height = find_nearest(e, surface_t::FACE_LAND).first->height;
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
	for (auto &f : faces) {
		if (f->type != surface_t::FACE_LAND || (f->height < 0.4 && f->height > 0.5)) {
			continue;
		}
		if (rand() % 128 == 0)
			f->type = surface_t::FACE_FLOWING;
	}
	end = std::chrono::steady_clock::now();
	std::cout << "Elapsed: "
		<< std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()
		<< "[us]" << std::endl;

	std::cout << "Setting Rivers...\n";
	begin = std::chrono::steady_clock::now();
	//iterate_rivers(set);
	while (iterate_rivers());

	for (auto &f : faces) {
		if (f->type == surface_t::FACE_STAGNANT)
			f->type = surface_t::FACE_INLAND_LAKE;
	}
	end = std::chrono::steady_clock::now();
	std::cout << "Elapsed: "
		<< std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()
		<< "[us]" << std::endl;

	std::cout << "Setting Aridity Map...\n";
	begin = std::chrono::steady_clock::now();
	for (auto &f : faces) {
		if (f->type != surface_t::FACE_LAND)
			continue;
		std::pair<surface_t *, double> n = find_nearest(f, surface_t::FACE_INLAND_LAKE);
		point3_t cc = f->get_center_c();
		double pm =
			SimplexNoise::noise(noise_offset + cc[0] + 100, cc[1], cc[2]) * 0.5 +
			SimplexNoise::noise(noise_offset + cc[0] * 2.0 + 100, cc[1] * 2.0, cc[2] * 2.0) * 0.25 +
			SimplexNoise::noise(noise_offset + cc[0] * 4.0 + 100, cc[1] * 4.0, cc[2] * 4.0) * 0.15 +
			SimplexNoise::noise(noise_offset + cc[0] * 8.0 + 100, cc[1] * 8.0, cc[2] * 8.0) * 0.1;
		f->aridity = MAX<double>(0.0, std::pow(n.second, 0.6) * 2.0 + pm / 2.0) * ARIDITY_MULTIPLIER;
	}
	end = std::chrono::steady_clock::now();
	std::cout << "Elapsed: "
		<< std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()
		<< "[us]" << std::endl;

	std::cout << "Setting Foehn Map...\n";
	begin = std::chrono::steady_clock::now();
	set_foehn();
	end = std::chrono::steady_clock::now();
	std::cout << "Elapsed: "
		<< std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()
		<< "[us]" << std::endl;

	std::cout << "Setting Landmass Map...\n";
	begin = std::chrono::steady_clock::now();
	for (auto &s : faces) {
		if (s->landmass == NULL) {
			landmass_t *l = new landmass_t{
				(double)rand() / (double)RAND_MAX,
				(double)rand() / (double)RAND_MAX,
				(double)rand() / (double)RAND_MAX
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
	std::cout << "Face Count: " << faces.size() << "\n";
	std::cout << "---------------------------------\n";

	std::cout << "Done.\n";
}


world_t::world_t(const std::vector<surface_t *> &faces)
	: faces(faces)
{
	for (auto &s : faces) {
		if (s->landmass == NULL) {
			landmass_t *l = new landmass_t{
				(double)rand() / (double)RAND_MAX,
				(double)rand() / (double)RAND_MAX,
				(double)rand() / (double)RAND_MAX
			};
			s->landmass = l;
			landmasses.push_back(l);
			l->members.push_back(s);
			make_landmasses(s);
		}
	}
}

surface_t *world_t::find_closest(const double &yaw, const double &pit)
{
	double d = 100.0;
	surface_t *curr = NULL;

	for (auto &f : faces) {
		auto center = f->get_center();

		double t = std::sqrt(std::pow(center[0] - yaw, 2) + std::pow(center[1] - pit, 2));
		if (t < d) {
			curr = f;
			d = t;
		}
	}

	return curr;
}

std::vector<surface_t *> world_t::get_faces() const
{
	return faces;
}

world_t::~world_t()
{
	for (auto &e : faces)
		delete e;
	for (auto &e : landmasses)
		delete e;
}