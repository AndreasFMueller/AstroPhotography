/*
 * AsiCamera.hh -- ASI camera class
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _AsiCamera_hh
#define _AsiCamera_hh

#include <AstroCamera.h>
#include <AsiCamera2.h>

namespace astro {
namespace camera {
namespace asi {

/**
 * \brief AsiCamera class
 */
class AsiCamera : public Camera {
	int	_index;
public:
	int	index() const { return _index; }
private:
	bool	_hasCooler;
public:
	AsiCamera(int index);
	~AsiCamera();

	// prevent copying of the camera class
private:
	AsiCamera(const AsiCamera& other);
	AsiCamera&	operator=(const AsiCamera& other);

protected:
	virtual CcdPtr	getCcd0(size_t id);

private:
	bool	_hasGuiderPort;
public:
	virtual bool	hasGuiderPort() const;
protected:
	virtual GuiderPortPtr	getGuiderPort0();

private:
	bool	_isColor;
public:
	bool	isColor() const;
};

} // namespace asi
} // namespace camera
} // namespace astro

#endif /* _AsiCamera_hh */
