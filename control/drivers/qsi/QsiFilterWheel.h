/*
 * QsiFilterWheel.h -- QSI cameras with filter wheels
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _QsiFilterWheel_h
#define _QsiFilterWheel_h

#include <AstroCamera.h>
#include <QsiCamera.h>
#include <vector>

namespace astro {
namespace camera {
namespace qsi {

class QsiFilterWheel : public FilterWheel {
	QsiCamera&	_camera;
	unsigned int	nfilters;
	std::vector<std::string>	filternames;
public:
	QsiFilterWheel(QsiCamera& camera);
	virtual ~QsiFilterWheel();
	virtual unsigned int	nFilters();
	virtual unsigned int	currentPosition();
	virtual void	select(size_t filterindex);
	virtual void	select(const std::string& filtername);
	virtual std::string	filterName(size_t filterindex);
	virtual FilterWheel::State	getState();
};

} // namespace qsi
} // namespace camera
} // namespace astro

#endif /* _QsiFilterWheel_h */
