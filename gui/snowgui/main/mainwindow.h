/**
 * \brief global main window class
 *
 * (c) 2017 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <AstroDiscovery.h>
#include <QLabel>
#include <AstroImage.h>
#include "WindowsMenu.h"
//#include "configurationdialog.h"
#include <eventdisplaywidget.h>
#include <systemconfigurationwidget.h>

namespace snowgui {

namespace Ui {
	class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT
	const astro::discover::ServiceObject	_serviceobject;
	QLabel	*serviceLabel(astro::discover::ServiceSubset::service_type t);
	void	setServiceLabelEnabled(astro::discover::ServiceSubset::service_type t);

	astro::image::ImagePtr	_image;
	std::string	_imagestring;
public:
	const std::string&	servername() const { return _serviceobject.name(); }

	SystemConfigurationWidget	*_configurationwidget = NULL;
	EventDisplayWidget	*_eventdisplaywidget = NULL;
public:
	explicit MainWindow(QWidget *parent,
		const astro::discover::ServiceObject serviceobject);
	~MainWindow();

public slots:
	void	launchPreview();
	void	launchFocusing();
	void	launchGuiding();
	void	launchPointing();
	void	launchRepository();
	void	launchInstruments();
	void	launchTasks();
	void	launchConfiguration();
	void	forgetConfiguration();
	void	launchImages();
	void	launchExpose();
	void	launchEvents();
	void	forgetEvents();
	void	imageForSaving(astro::image::ImagePtr image, std::string);
	void	timecheck();

private:
	Ui::MainWindow *ui;

	QMenu	*fileMenu;

	QAction	*connectAction;
	void	connectFile();

	QAction	*openAction;
	void	openFile();

	QAction	*browseAction;
	void	browseDirectory();

	QAction	*saveAction;
	void	saveImage();

	WindowsMenu	*windowsMenu;

	QAction	*raiseAction;
	void	raiseMainwindow();
	
	void	createActions();
	void	createMenus();

	QMenu	*systemMenu;

	QAction	*restartAction;
	void	restartServer();
};

} // namespace snowgui

#endif // MAINWINDOW_H
