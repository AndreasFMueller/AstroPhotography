/*
 * SbigFilterWheel.h -- SBIG filter wheel implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SbigCamera.h>
#include <SbigDevice.h>

namespace astro {
namespace camera {
namespace sbig {

class SbigFilterWheelTimeout : public std::runtime_error {
public:
	SbigFilterWheelTimeout(const char *cause) : std::runtime_error(cause) {}
};

class SbigFilterWheel : public FilterWheel, public SbigDevice {
	unsigned int	npositions;
	unsigned int	currentindex;
	void	init();
	void	wait();
	FilterWheel::State	state();
	void	cfw(CFWParams *params, CFWResults *results,
			const std::string& msg);
public:
	SbigFilterWheel(SbigCamera& camera);
	virtual ~SbigFilterWheel();
	virtual unsigned int	nFilters();
	virtual unsigned int	currentPosition();
	virtual void	select(size_t filterindex);
	virtual std::string	filterName(size_t filterindex);
	virtual FilterWheel::State	getState();
};

} // namespace sbig
} // namespace camera
} // namespace astro
