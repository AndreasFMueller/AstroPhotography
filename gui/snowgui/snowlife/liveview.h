/*
 * lifeview.h -- lifeview application main window header
 */
#ifndef SNOWGUI_LIVEVIEW_H
#define SNOWGUI_LIVEVIEW_H

#include <QMainWindow>
#include <AstroCamera.h>
#include <atomic>

namespace snowgui {

namespace Ui {
	class LiveView;
}

class LiveView;

/**
 * \brief Work class to do the exposing in a separate thread
 */
class ExposureWork : public QObject {
	Q_OBJECT
	LiveView	*_liveview;
public:
	explicit ExposureWork(LiveView *liveview);
	virtual ~ExposureWork();

public slots:
	void	doExposure();
};

/**
 * \brief StreamWork class for "Qt-based streaming"
 *
 * This class uses a single shot timer that 
 */
class StreamWork : public QObject {
	Q_OBJECT
	LiveView		*_liveview;
	int			_interval;
	std::atomic_bool	_running;
public:
	explicit StreamWork(LiveView *liveview);
	virtual ~StreamWork();
public:
	bool	running() const { return _running; }
	void	stop();
public slots:
	void	start();
	void	nextExposure();
	void	interval(double i);
};

/**
 * \brief LiveView main window class
 */
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

	// thread to use for exposing, single or otherwise
	QThread			*_thread;

	// variables to handle single exposures
	std::atomic_bool	_single;
	ExposureWork		*_work;

	// variables to handle our own streaming
	StreamWork	*_streamwork;

public:
	explicit LiveView(QWidget *parent = 0);
	~LiveView();

	void	operator()(const astro::camera::ImageQueueEntry& entry);


private:
	Ui::LiveView *ui;

	void	startStreamPrivate();
	void	stopStreamPrivate();

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

	void	doExposure();
	void	doSingleExposure();

	void	startStream();
	void	stopStream();

	void	singleClicked();
	void	receiveImage(astro::image::ImagePtr);
	void	threadFinished();
};

} // namespace snowgui

#endif // SNOWGUI_LIVEVIEW_H
