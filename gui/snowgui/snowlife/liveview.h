/*
 * lifeview.h -- lifeview application main window header
 */
#ifndef SNOWGUI_LIVEVIEW_H
#define SNOWGUI_LIVEVIEW_H

#include <QMainWindow>
#include <AstroCamera.h>

namespace snowgui {

namespace Ui {
	class LiveView;
}

class LiveView : public QMainWindow, public astro::camera::ImageSink {
	Q_OBJECT

	std::list<std::string>	_ccdNames;
	std::list<std::string>	_focuserNames;

	QMenu	*_ccdMenu;
	std::list<QAction*>	_ccdActions;

	QMenu	*_focuserMenu;
	std::list<QAction*>	_focuserActions;

	astro::camera::CcdPtr		_ccd;
	astro::camera::FocuserPtr	_focuser;

	astro::camera::Exposure	_exposure;

public:
	explicit LiveView(QWidget *parent = 0);
	~LiveView();

	void	operator()(const astro::camera::ImageQueueEntry& entry);

private:
	Ui::LiveView *ui;

signals:
	void	newImage(astro::image::ImagePtr);

public slots:
	void	openCamera(std::string);
	void	openFocuser(std::string);

	void	addCamera(std::string);
	void	addFocuser(std::string);

	void	setSubframe(astro::image::ImageRectangle);
	void	setExposuretime(double);
	void	fullframeClicked();

	void	startStream();
	void	stopStream();
};

} // namespace snowgui

#endif // SNOWGUI_LIVEVIEW_H
