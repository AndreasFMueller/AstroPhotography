/*
 * imagercontrollerwidget.h -- common CCD controller
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef CCDCONTROLLERWIDGET_H
#define CCDCONTROLLERWIDGET_H

#include <InstrumentWidget.h>
#include <AstroCamera.h>
#include <image.h>
#include <guider.h>
#include "flatwidget.h"
#include "darkwidget.h"
#include <QTimer>

namespace snowgui {

namespace Ui {
	class imagercontrollerwidget;
}

/**
 * \brief A reusable component to control a CCD
 */
class imagercontrollerwidget : public InstrumentWidget {
	Q_OBJECT

	snowstar::GuiderPrx	_guider;
	snowstar::CcdInfo	_ccdinfo;
	astro::camera::Exposure	_exposure;

	astro::image::ImagePtr	_image;
	astro::camera::Exposure	_imageexposure;
	snowstar::ImagePrx	_imageproxy;

	bool	_nosubframe;
	bool	_nobuttons;

	flatwidget	*_flatwidget;
	darkwidget	*_darkwidget;

public:
	explicit imagercontrollerwidget(QWidget *parent = NULL);
	~imagercontrollerwidget();
	virtual void	instrumentSetup(
		astro::discover::ServiceObject serviceobject,
		snowstar::RemoteInstrument instrument);

	const astro::camera::Exposure&	exposure() const { return _exposure; }
	const astro::image::ImagePtr	image() const { return _image; }
	const astro::camera::Exposure&	imageexposure() const { return _imageexposure; }

signals:
	void	exposureChanged(astro::camera::Exposure);
	void	imageReceived(astro::image::ImagePtr image);
	void	imageproxyReceived(snowstar::ImagePrx image);
	void	ccdSelected(int);

private:
	void	setupCcd();
	void	ccdFailed(const std::exception&);

	// methods to synchronize the GUI state with the internal state
	void	displayExposure(astro::camera::Exposure);
	void	displayBinning(astro::image::Binning);
	void	displayBinning(int index);
	astro::image::Binning	getBinning(int index);
	void	displayExposureTime(double);
	astro::camera::Exposure::purpose_t	getPurpose(int index);
	void	displayFrame(astro::image::ImageRectangle);

	// retrieve an image
	void	retrieveImage();

	Ui::imagercontrollerwidget *ui;
	QTimer	statusTimer;
	snowstar::GuiderState	previousstate;
	bool	ourexposure;

public slots:
	// changes taht come from the outside
	void	setExposure(astro::camera::Exposure);
	void	setBinning(astro::image::Binning);
	void	setExposureTime(double);
	void	setFrame(astro::image::ImageRectangle);
	void	setSubframe(astro::image::ImageRectangle);
	void	setImage(astro::image::ImagePtr);

	// this slot is used by the GUI elements to update the settings 
	// from the GUI
	void	guiChanged();
	void	captureClicked();
	void	darkClicked();
	void	flatClicked();
	void	hideSubframe(bool);
	void	hideButtons(bool);
	void	toggleDark(bool);
	void	toggleFlat(bool);
	void	toggleInterpolate(bool);

	void	darkClosed();
	void	flatClosed();

	// needed internally for status udpates
	void	statusUpdate();

};

} // namespace snowgui

#endif // CCDCONTROLLERWIDGET_H
