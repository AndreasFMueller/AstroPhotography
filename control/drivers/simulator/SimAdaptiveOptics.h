/*
 * SimAdaptiveOptics.h -- tip/tilt adaptive optics unit simulation
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCamera.h>
#include <SimLocator.h>

namespace astro {
namespace camera {
namespace simulator {

/**
 * \brief Simulator class to simulate an adaptive optics unit
 *
 * This class implements the interface for an adaptive optics unit.
 */
class SimAdaptiveOptics : public AdaptiveOptics {
	double	starttime;
	double	_amplitude;
	bool	_activated;
public:
	double	amplitude() const { return _amplitude; }
	void	amplitude(double a) { _amplitude = a; }
	SimAdaptiveOptics();
	virtual	~SimAdaptiveOptics();
	virtual void	set0(const Point& position);
	Point	offset() const;
}; 

} // namespace simulator
} // namespace camera
} // namespace astro
