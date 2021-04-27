#pragma once

#include <glm/glm.hpp>
#include <glm/vec2.hpp>

class polar_t
{
private:
	glm::vec2 coords;
public:
	polar_t();
	polar_t(const float &, const float &);
	const float operator[](const size_t &) const;
	const bool operator==(const polar_t &) const;
	const bool operator!=(const polar_t &) const;
	const bool operator<(const polar_t &) const;
};