/*
 * SxAO.h -- adaptive optics unit interface definition
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _SxAO_h
#define _SxAO_h

#include <AstroCamera.h>

namespace astro {
namespace camera {
namespace sx {

/**
 * \brief This is a driver class for the SX AO-LF adaptive optics unit
 *
 * The SX adaptive optics unit uses a serial interface. This has the
 * disadvantage that adaptive optics units are not discoverable, at least
 * not directly.
 */
class SxAO : public AdaptiveOptics {
	std::string	device;
	int	serial;
	void	initialize(const std::string& serialdevice);
	bool	move(char d, int steps = 1);
	bool	north(int steps = 1);
	bool	south(int steps = 1);
	bool	west(int steps = 1);
	bool	east(int steps = 1);
	int	offset[2];
	int	limits[2];
	bool	findcenter();
	bool	move2(int x, int y);
	char	response();
private:	// prevent copy
	SxAO(const SxAO& other);
	SxAO&	operator=(const SxAO& other);
public:
	SxAO(const DeviceName& devicename);
	~SxAO();
protected:
	virtual void	set0(const Point& position);

	// Methods related to the guider port. These methods will probably
	// disappear again, as it turns out that the guider port of the AO
	// unit is not really usable (it cannot activate more than one 
	// output at the same time, and any other command terminates the
	// guider port output.
public:
	bool	mountmove(char d, int steps = 1);
	bool	decplus(int steps = 1);
	bool	decminus(int steps = 1);
	bool	raplus(int steps = 1);
	bool	raminus(int steps = 1);
};

} // namespace sx
} // namespace camera
} // namespace astro

#endif /* _SxAO_h */
