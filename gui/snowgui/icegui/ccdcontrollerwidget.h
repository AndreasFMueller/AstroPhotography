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
	astro::Angle	_azimut;
	std::string	_name;
	snowstar::CcdInfo	_ccdinfo;
public:
	ccddata() : _type(snowstar::InstrumentCCD), _index(-1),
		    _focallength(0), _azimut(0), _name("") { }
	ccddata(snowstar::InstrumentComponentType type, int index,
		double focallength, const astro::Angle& azimut,
		const std::string& name)
		: _type(type), _index(index), _focallength(focallength),
		  _azimut(azimut), _name(name) { }
	snowstar::InstrumentComponentType	type() const { return _type; }
	int	index() const { return _index; }
	double	focallength() const { return _focallength; }
	const astro::Angle&	azimut() const { return _azimut; }
	const std::string& 	name() const { return _name; }
	void	ccdinfo(const snowstar::CcdInfo& i) { _ccdinfo = i; }
	const snowstar::CcdInfo&	ccdinfo() const { return _ccdinfo; }
	std::string	toString() const;
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
 * \brief A reusable component to control a CCD
 */
class ccdcontrollerwidget : public InstrumentWidget {
	Q_OBJECT

	snowstar::CcdPrx	_ccd;
	snowstar::CcdInfo	_ccdinfo;
	astro::camera::Exposure	_exposure;

	std::recursive_mutex	_mutex;	// mutex to protect image state
	astro::image::ImagePtr	_image;
	astro::camera::Exposure	_imageexposure;
	snowstar::ImagePrx	_imageproxy;

	bool	_guiderccdonly;
	bool	_nosubframe;
	bool	_nobuttons;
	bool	_imageproxyonly;

	std::vector<ccddata>	_ccddata;
	ccddata	_current_ccddata;

	ImageRetrieverThread	*_imageretriever;
	HideWidget	*_hide;
	HideProgress	*_hideprogress;

	StateMonitoringWork	*_statemonitoringwork;
	QThread	*_statemonitoringthread;
	QTimer	_statemonitoringTimer;
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
	// changes taht come from the outside
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

	// test slot
	void	testSlot();

	// allow the ImageRetrieverThread access to private methods
	friend class ImageRetrieverThread;
	friend class StateMonitoringWork;
};

} // namespace snowgui

#endif // CCDCONTROLLERWIDGET_H
