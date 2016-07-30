/*
 * previewwindow.h -- application to preview images from the camera
 *
 * This is most useful for focusing or pointing the telescope
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef PREVIEWWINDOW_H
#define PREVIEWWINDOW_H

#include <QWidget>
#include <AstroDiscovery.h>
#include <AstroImage.h>
#include <RemoteInstrument.h>
#include <camera.h>
#include <CommonClientTasks.h>

namespace Ui {
	class PreviewWindow;
}

class PreviewWindow : public QWidget {
	Q_OBJECT
	astro::discover::ServiceObject	_serviceobject;
	snowstar::RemoteInstrument	_instrument;
	snowstar::CcdPrx		_ccd;
	snowstar::CoolerPrx		_cooler;
	snowstar::FilterWheelPrx	_filterwheel;
	snowstar::FocuserPrx		_focuser;
	snowstar::GuiderPortPrx		_guiderport;

	astro::image::ImagePtr		_image;
	snowstar::CallbackAdapterPtr	_adapter;
	snowstar::ImageSinkPtr		_previewimagesink;

public:
	explicit PreviewWindow(QWidget *parent,
		astro::discover::ServiceObject serviceobject,
		snowstar::RemoteInstrument instrument);
	~PreviewWindow();

	void	setImage(astro::image::ImagePtr image);

private:
	void	setupCcd();
	void	setupFilterwheel();
	void	setupCooler();
	void	setupFocuser();
	void	setupGuiderport();

	void	startStream();
	void	stopStream();

	astro::camera::Exposure	getExposure();

signals:
	void	imageUpdated();

public slots:

	void	processImage();

	void	ccdChanged(int ccdindex);
	void	exposureChanged();
	void	toggleStream();

	void	filterwheelChanged(int filterwheelindex);
	void	filterwheelFilterChanged(int filterindex);

	void	coolerChanged(int coolerindex);
	void	coolerTemperatureChanged(double settemperature);
	void	coolerOnOff();

	void	focuserChanged(int focuserindex);
	void	focuserSetChanged(int focusposition);

	void	guiderportChanged(int guiderportindex);
	void	guiderportActivated();

	void	statusUpdate();
private:
	Ui::PreviewWindow *ui;
	QTimer	*statusTimer;
};

#endif // PREVIEWWINDOW_H
