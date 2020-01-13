/*
 * LuminanceFunctions.h
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroTonemapping.h>

namespace astro {
namespace adapter {

/**
 * \brief Luminance stretching using the gamma 
 *
 * The Gamma function scales values between [x1,x2] to [0,1], applies the
 * x^gamma function to it and rescales the resulting values from [0,1] to
 * [y1,y2]
 */
class GammaFunction : public LuminanceFunction {
	double	_gamma;
public:
	GammaFunction(const LuminanceFunction::parameters_t& parameters);
	virtual double	operator()(double l);
};

/**
 * \brief Luminance stretching function using asinh
 *
 * Convert luminance values using the atanh function.
 * The argument x1 is mapped to y1, the argument x2 is mapped to
 * asinh(1) * (y2-y1) + y1.
 */
class AsinhFunction : public LuminanceFunction {
public:
	AsinhFunction(const LuminanceFunction::parameters_t& parameters);
	virtual double	operator()(double l);
};

/**
 * \brief Luminance stretching using the atan function
 *
 * Convert luminance values using the atan function.
 * The argument x1 is mapped to y1, the maximum possible value is y2.
 * The argument x2 is mapped to (atan(1) / (M_PI/2)) * (y2-y1) + y1.
 */
class AtanFunction : public LuminanceFunction {
public:
	AtanFunction(const LuminanceFunction::parameters_t& parameters);
	virtual double	operator()(double l);
};

/**
 * \brief Luminance stretching using the atanh function
 *
 * Convert luminance values using the atanh function.
 * The argument x1 is mapped to y1, the maximum possible value is y2.
 * The argument x2 is mapped to atanh(1) * (y2-y1) + y1.
 */
class AtanhFunction : public LuminanceFunction {
public:
	AtanhFunction(const LuminanceFunction::parameters_t& parameters);
	virtual double	operator()(double l);
};

/**
 * \brief Luminance stretching using the atanh function
 *
 * Convert luminance values above x1 using the binary logarithm function
 * log2(x + 1) so that the function value on x1 is y1 and the value on
 * x2 is y2.
 */
class LogFunction : public LuminanceFunction {
public:
	LogFunction(const LuminanceFunction::parameters_t& parameters);
	virtual double	operator()(double l);
};

} // namespace adapter
} // namespace astro
