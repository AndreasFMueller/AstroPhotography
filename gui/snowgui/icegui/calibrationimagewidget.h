/*
 * calibrationimagewidget.h -- base class for dark and flat cal images
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _calibrationimagewidget_h
#define _calibrationimagewidget_h

#include <QDialog>
#include <guider.h>
#include <QTimer>
#include <imagedisplaywidget.h>

namespace snowgui {

class calibrationimagewidget;

/**
 * \brief A monitor class for Calibration Image progress data
 */
class CalibrationImageMonitor : public QObject,
				public snowstar::CalibrationImageMonitor {
	Q_OBJECT
	calibrationimagewidget	*_calibrationimagewidget;
public:
	CalibrationImageMonitor(calibrationimagewidget *);

	virtual void	update(const snowstar::CalibrationImageProgress& prog,
				const Ice::Current& current);
	virtual void	stop(const Ice::Current& current);
signals:
	void	updateSignal(snowstar::CalibrationImageProgress);
	void	stopSignal();
};

/**
 * \brief Base class for calibration image dialogs
 */
class calibrationimagewidget : public QDialog {
	Q_OBJECT

	Ice::Identity	_monitoridentity;
	CalibrationImageMonitor	*_monitor;
protected:
	QTimer			statusTimer;
	snowstar::GuiderPrx	_guider;
	snowstar::GuiderState	_guiderstate;
	bool	_acquiring;
	imagedisplaywidget	*_imagedisplaywidget;
	astro::image::ImagePtr	_image;
public:
	explicit	calibrationimagewidget(QWidget *parent = 0);
	virtual ~calibrationimagewidget();

	void	do_unregister();

	void	guider(snowstar::GuiderPrx guider);
	ImagePtr	image() { return _image; }
	virtual void	checkImage() = 0;
	virtual	std::string	imagetype() = 0;

signals:
	void	newImage(astro::image::ImagePtr);
	void	closeWidget();
	void	offerImage(astro::image::ImagePtr, std::string);
	void	stopSignal();
	void	updateSignal(snowstar::CalibrationImageProgress);

public:
	void	closeEvent(QCloseEvent *);
	void	changeEvent(QEvent *);

public slots:
	void	viewClicked();
	void	imageClosed();
	void	stopped();
	void	signalUpdated(snowstar::CalibrationImageProgress);
};

} // namespace snowgui

#endif /* _calibrationimagewidget_h */
