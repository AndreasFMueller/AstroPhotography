/*
 * LinearRegression.h
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef LinearRegression_h
#define LinearRegression_h

#include <vector>

namespace astro {
namespace linear {

class LinearRegression {
	double	_a;
	double	_b;
public:
	LinearRegression(const std::vector<std::pair<double, double> >& data);
	double	a() const { return _a; }
	double	b() const { return _b; }
};

} // namespace linear
} // namespace astro

#endif /* LinearRegression_h */
