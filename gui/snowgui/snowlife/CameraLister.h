/*
 * CameraLister.h -- axiliary class used to list devices
 *
 * (c) 2018 Prof Dr Andreas Mülle, Hochschule Rapperswil
 */
#ifndef _CameraLister_h
#define _CameraLister_h

#include <QThread>
#include <AstroLocator.h>

namespace snowgui {

/**
 * \brief Class to list cameras connected to a system
 *
 * This class' run method emits signals for all cameras connected
 * to the system. The LiveView class uses this to build the menu
 * of available devices.
 */
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
