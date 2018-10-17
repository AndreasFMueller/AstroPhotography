/*
 * CameraLister.h
 *
 * (c) 2018 Prof Dr Andreas Mülle, Hochschule Rapperswil
 */
#ifndef _CameraLister_h
#define _CameraLister_h

#include <QThread>
#include <AstroLocator.h>

namespace snowgui {

class CameraLister : public QThread {
	Q_OBJECT

	void	addCameras(astro::device::DeviceLocatorPtr locator);
	void	addFocusers(astro::device::DeviceLocatorPtr locator);
public:
	explicit	CameraLister(QObject *parent = NULL);
	~CameraLister();
	
	void	run();

signals:
	void	camera(std::string);
	void	focuser(std::string);

};


} // namespace snowgui

#endif /* _CameraLister_h */
