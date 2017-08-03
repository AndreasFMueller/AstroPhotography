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
#include <Image2Pixmap.h>

namespace snowgui {

namespace Ui {
	class PreviewWindow;
}

class PreviewWindow : public QWidget {
	Q_OBJECT
	astro::discover::ServiceKey	_servicekey;
	snowstar::RemoteInstrument	_instrument;
	snowstar::CcdPrx		_ccd;
	snowstar::CoolerPrx		_cooler;
	snowstar::FilterWheelPrx	_filterwheel;
	snowstar::FocuserPrx		_focuser;
	snowstar::GuidePortPrx		_guideport;

	astro::image::ImagePtr		_image;
	snowstar::CallbackAdapterPtr	_adapter;
	snowstar::ImageSinkPtr		_previewimagesink;
	snowgui::Image2Pixmap	image2pixmap;

public:
	explicit PreviewWindow(QWidget *parent);
	void	instrumentSetup(astro::discover::ServiceObject serviceobject,
		snowstar::RemoteInstrument instrument);
	~PreviewWindow();

	void	setImage(astro::image::ImagePtr image);

private:
	void	setupCcd();
	void	setupFilterwheel();
	void	setupCooler();
	void	setupFocuser();
	void	setupGuideport();

	void	startStream();
	void	stopStream();

	astro::camera::Exposure	getExposure();

	void	displayGainSettings();
	void	displayBrightnessSettings();
	void	displayScaleSettings();

signals:
	void	imageUpdated();

public slots:

	void	processImage();
	void	imageSettingsChanged();

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

	void	guideportChanged(int guideportindex);
	void	guideportActivated();

	void	statusUpdate();
	
private:
	Ui::PreviewWindow *ui;
	QTimer	*statusTimer;

protected:
	void	changeEvent(QEvent *);
};

} // namespace snowgui

#endif // PREVIEWWINDOW_H
