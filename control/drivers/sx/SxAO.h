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

class SxAO : public AdaptiveOptics {
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
private:	// prevent copy
	SxAO(const SxAO& other);
	SxAO&	operator=(const SxAO& other);
public:
	SxAO(const std::string& name);
	~SxAO();
protected:
	virtual void	set0(const Point& position);

	// methods related to the guider port
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
