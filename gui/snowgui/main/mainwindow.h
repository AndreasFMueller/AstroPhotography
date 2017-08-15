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
	explicit MainWindow(QWidget *parent,
		const astro::discover::ServiceObject serviceobject);
	~MainWindow();

public slots:
	void	launchPreview();
	void	launchFocusing();
	void	launchGuiding();
	void	launchRepository();
	void	launchInstruments();
	void	launchTasks();
	void	launchConfiguration();
	void	launchImages();
	void	launchExpose();
	void	launchEvents();
	void	imageForSaving(astro::image::ImagePtr image, std::string);

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

	void	createActions();
	void	createMenus();
};

} // namespace snowgui

#endif // MAINWINDOW_H
