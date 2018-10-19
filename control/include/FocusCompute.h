/*
 * FocusCompute.h -- class to compute the best focus position from measurements
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _FocusCompute_h
#define _FocusCompute_h

#include <map>

namespace astro {
namespace focusing {

/**
 * \brief Class to compute 
 */
class FocusCompute : public std::map<unsigned long, double> {
	std::pair<double, double>	solve(double *positions, double *values) const;
public:
	FocusCompute();
	double	focus() const;
};

} // namespace focusing
} // namespace astro

#endif /* _FocusCompute_h */
