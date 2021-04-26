#pragma once

#include <eigen3/Eigen/Dense>

class polar_t
{
private:
	Eigen::Matrix<float, 2, 1> coords;
public:
	polar_t();
	polar_t(const float &, const float &);
	const float operator[](const size_t &) const;
	const bool operator==(const polar_t &) const;
	const bool operator!=(const polar_t &) const;
};