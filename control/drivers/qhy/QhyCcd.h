/*
 * QhyCcd.h -- QHY device
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _QhyCcd_h
#define _QhyCcd_h

#include <AstroCamera.h>
#include <QhyCamera.h>
#include <qhylib.h>
#include <QhyUtils.h>

namespace astro {
namespace camera {
namespace qhy {

/**
 * \brief QHY ccd class
 */
class QhyCcd : public Ccd {
	std::thread	thread;
	ImagePtr	image;
	::qhy::DevicePtr	deviceptr;
	QhyCamera&	camera;
public:
	QhyCcd(const CcdInfo& info, const ::qhy::DevicePtr devptr,
		QhyCamera& _camera);
	virtual ~QhyCcd();
public:
	virtual void	startExposure(const Exposure& exposure);
private:
	static void	main(QhyCcd *ccd) noexcept;
public:
	virtual void	getImage0();
	virtual ImagePtr	getRawImage();
	virtual bool	hasCooler() const { return true; }
protected:
	virtual CoolerPtr	getCooler0();
};

} // namespace qhy
} // namespace camera
} // namespace astro

#endif /* _QhyCcd_h */
