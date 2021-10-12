/*
 * lifeview.h -- lifeview application main window header
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef SNOWGUI_LIVEVIEW_H
#define SNOWGUI_LIVEVIEW_H

#include <QMainWindow>
#include <QTimer>
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
	std::pair<float,float>		_gaininterval;

	// thread to use for exposing, single or otherwise
	QThread			*_thread;

	// variables to handle single exposures
	typedef enum { idle, single, streaming } mode_type;
	std::atomic<mode_type>	_mode;

	// single exposure work
	ExposureWork		*_exposurework;

	// variables to handle our own streaming
	StreamWork	*_streamwork;

	// focuser status timer	
	QTimer	_timer;

public:
	explicit LiveView(QWidget *parent = 0);
	~LiveView();

	void	operator()(const astro::camera::ImageQueueEntry& entry);


private:
	Ui::LiveView *ui;

	void	startStreamPrivate();
	void	stopStreamPrivate();
	void	updateTitle();

signals:
	void	newImage(astro::image::ImagePtr);
	void	triggerExposure();

public slots:
	void	openCamera(std::string);
	void	openFocuser(std::string);

	void	addCamera(std::string);
	void	addFocuser(std::string);

	void	focusChanged(int);

	void	setSubframe(astro::image::ImageRectangle);
	void	setExposuretime(double);
	void	fullframeClicked();

	void	setGain(int);

	void	doExposure();

	void	startStream();
	void	stopStream();

	void	singleClicked();
	void	receiveImage(astro::image::ImagePtr);
	void	threadFinished();

	void	focuserUpdate();

	// custom context menu für the focuser
	void	showFocuserStepsMenu(const QPoint& p);
	void	stepsizeChanged();
};

} // namespace snowgui

#endif // SNOWGUI_LIVEVIEW_H
