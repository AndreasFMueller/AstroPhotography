/*
 * ccdcontrollerwidget.h -- common CCD controller
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef CCDCONTROLLERWIDGET_H
#define CCDCONTROLLERWIDGET_H

#include <InstrumentWidget.h>
#include <AstroCamera.h>
#include <AstroCoordinates.h>
#include <image.h>
#include <QThread>
#include <QTimer>
#include <HideWidget.h>
#include <HideProgress.h>
#include <ImagerRectangle.h>

namespace snowgui {

namespace Ui {
	class ccdcontrollerwidget;
}

/**
 * \brief Holder class to collect information about each CCD
 */
class ccddata {
	snowstar::InstrumentComponentType	_type;
	int	_index;
	double	_focallength;
	astro::Angle	_azimuth;
	std::string	_name;
	snowstar::CcdInfo	_ccdinfo;
public:
	ccddata() : _type(snowstar::InstrumentCCD), _index(-1),
		    _focallength(0), _azimuth(0), _name("") { }
	ccddata(snowstar::InstrumentComponentType type, int index,
		double focallength, const astro::Angle& azimuth,
		const std::string& name)
		: _type(type), _index(index), _focallength(focallength),
		  _azimuth(azimuth), _name(name) { }
	snowstar::InstrumentComponentType	type() const { return _type; }
	int	index() const { return _index; }
	double	focallength() const { return _focallength; }
	const astro::Angle&	azimuth() const { return _azimuth; }
	const std::string& 	name() const { return _name; }
	void	ccdinfo(const snowstar::CcdInfo& i) { _ccdinfo = i; }
	const snowstar::CcdInfo&	ccdinfo() const { return _ccdinfo; }
	std::string	toString() const;
	astro::Angle	resolution() const;
	ImagerRectangle	imagerrectangle() const;
};

class ccdcontrollerwidget;
/**
 * \brief Thread to retrieve an image from a CCD device
 */
class ImageRetrieverThread : public QThread {
	Q_OBJECT
	ccdcontrollerwidget	*_ccdcontrollerwidget;
public:
	ImageRetrieverThread(ccdcontrollerwidget *c);
	virtual ~ImageRetrieverThread();
	void	run();
signals:
	void	failed(QString);
};

/**
 * \brief Thread to monitor the exposure state
 */
class StateMonitoringWork : public QObject {
	Q_OBJECT
	ccdcontrollerwidget	*_ccdcontrollerwidget;
	snowstar::ExposureState	_previousstate;
public:
	StateMonitoringWork(ccdcontrollerwidget *c);
	virtual ~StateMonitoringWork();
public slots:
	void	updateStatus();
signals:
	void	stateChanged(snowstar::ExposureState);
};

/**
 * \brief Callback for CCD state updates
 */
class CcdCallbackI : public QObject, public snowstar::CcdCallback {
	Q_OBJECT
	ccdcontrollerwidget&	_ccdcontrollerwidget;
public:
	CcdCallbackI(ccdcontrollerwidget& c);
	void	state(snowstar::ExposureState s,
			const Ice::Current& /* current */);
	void	stop(const Ice::Current& /* current */);
signals:
	void	stateChanged(snowstar::ExposureState);
};

/**
 * \brief A reusable component to control a CCD
 */
class ccdcontrollerwidget : public InstrumentWidget {
	Q_OBJECT

	snowstar::CcdPrx	_ccd;
	snowstar::CcdInfo	_ccdinfo;
	astro::camera::Exposure	_exposure;

	std::pair<float,float>	_gaininterval;

	std::recursive_mutex	_mutex;	// mutex to protect image state
	astro::image::ImagePtr	_image;
	astro::camera::Exposure	_imageexposure;
	snowstar::ImagePrx	_imageproxy;

	Ice::ObjectPtr	_ccd_callback;
	Ice::Identity	_ccd_identity;

	bool	_guiderccdonly;
	bool	_nosubframe;
	bool	_nobuttons;
	bool	_imageproxyonly;

	std::vector<ccddata>	_ccddata;
	ccddata	_current_ccddata;

	ImageRetrieverThread	*_imageretriever;
	HideWidget	*_hide;
	HideProgress	*_hideprogress;

#if 0
	StateMonitoringWork	*_statemonitoringwork;
	QThread	*_statemonitoringthread;
	QTimer	_statemonitoringTimer;
#endif
public:
	explicit ccdcontrollerwidget(QWidget *parent = NULL);
	~ccdcontrollerwidget();
	virtual void	instrumentSetup(
		astro::discover::ServiceObject serviceobject,
		snowstar::RemoteInstrument instrument);
	virtual void	setupComplete();

	const astro::camera::Exposure&	exposure() const { return _exposure; }
	const astro::image::ImagePtr	image() const { return _image; }
	const astro::camera::Exposure&	imageexposure() const { return _imageexposure; }

	bool	guiderccdonly() const { return _guiderccdonly; }
	void	guiderccdonly(bool g) { _guiderccdonly = g; }

	bool	imageproxyonly() const { return _imageproxyonly; }
	void	imageproxyonly(bool i) { _imageproxyonly = i; }

signals:
	void	exposureChanged(astro::camera::Exposure);
	void	imageReceived(astro::image::ImagePtr image);
	void	imageproxyReceived(snowstar::ImagePrx image);
	void	ccdSelected(int);
	void	ccddataSelected(ccddata);
	void	ccdprxSelected(snowstar::CcdPrx);
	void	imageNotReceived(QString);
	void	streamStart();
	void	finderResolution(astro::Angle);
	void	guiderResolution(astro::Angle);
	void	imagerResolution(astro::Angle);
	void	finderRectangle(snowgui::ImagerRectangle);
	void	guiderRectangle(snowgui::ImagerRectangle);
	void	imagerRectangle(snowgui::ImagerRectangle);

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
	void	displayPurpose(astro::camera::Exposure::purpose_t);
	void	displayShutter(astro::camera::Shutter::state);
	void	displayFrame(astro::image::ImageRectangle);

	// retrieve an image
	void	retrieveImageStart();
	void	retrieveImageWork();

	Ui::ccdcontrollerwidget *ui;
	bool	ourexposure;

public slots:
	// changes that come from the outside
	void	setExposure(astro::camera::Exposure);
	void	setBinning(astro::image::Binning);
	void	setExposureTime(double);
	void	setPurpose(astro::camera::Exposure::purpose_t);
	void	setShutter(astro::camera::Shutter::state);
	void	setFrame(astro::image::ImageRectangle);
	void	setSubframe(astro::image::ImageRectangle);
	void	setImage(astro::image::ImagePtr);

	// this slot is used by the GUI elements to update the settings 
	// from the GUI
	void	guiChanged();
	void	captureClicked();
	void	cancelClicked();
	void	streamClicked();
	void	ccdChanged(int);
	void	gainChanged(int);
	void	hideSubframe(bool);
	void	hideButtons(bool);

	// start the stream
	void	startStream();

	// needed internally for status udpates
	void	statusUpdate(snowstar::ExposureState);

	// needed by the image retrieval thread 
	void	retrieveImageComplete();
	void	retrieveImageFailed(QString);

	// slots to change the subframe
	void	subframeWidth(int);
	void	subframeHeight(int);
	void	subframeOriginX(int);
	void	subframeOriginY(int);

	// set the gain
	void	setGainSlider(float gain);
	void	setGain(float gain);

	// test slot
	void	testSlot();

	// allow the ImageRetrieverThread access to private methods
	friend class ImageRetrieverThread;
	friend class StateMonitoringWork;
};

} // namespace snowgui

#endif // CCDCONTROLLERWIDGET_H
