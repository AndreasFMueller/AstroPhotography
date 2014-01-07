#include "guidermonitordialog.h"
#include "ui_guidermonitordialog.h"
#include <AstroDebug.h>
#include <cassert>
#include <AstroUtils.h>
#include <math.h>
#include <time.h>
#include "connectiondialog.h"

void	GuiderMonitorDialog::registerServants() {
	// if the implementation pointers are already set, we don't need to
	// do anything
	if (tm_impl) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "tracking monitor already up");
		return;
	}

	// create a servant
	tm_impl = new guidermonitor::TrackingMonitor_impl(*this);
	tim_impl = new guidermonitor::TrackingImageMonitor_impl(*this);

	// get the root POA
	CORBA::Object_var	obj
		= ConnectionDialog::orb->resolve_initial_references("RootPOA");
	PortableServer::POA_var	root_poa = PortableServer::POA::_narrow(obj);
	assert(!CORBA::is_nil(root_poa));

	// activate the servant, we alos remove a reference from the servant
	// so that the destructor is called when the servant is deactivated
	// in the unregisterServant() method
	PortableServer::ObjectId_var	tmid
		= root_poa->activate_object(tm_impl);
	tm_impl->_remove_ref();
	PortableServer::ObjectId_var	timid
		= root_poa->activate_object(tim_impl);
	tim_impl->_remove_ref();

	// get a reference to the object, we need it for the registration
	CORBA::Object_var	tmobj
		= root_poa->id_to_reference(tmid);
	Astro::TrackingMonitor_var	tmvar
		= Astro::TrackingMonitor::_narrow(tmobj);
	CORBA::Object_var	timobj
		= root_poa->id_to_reference(timid);
	Astro::TrackingImageMonitor_var	timvar
		= Astro::TrackingImageMonitor::_narrow(timobj);

	// now register the servant with the guider
	monitorid = _guider->registerMonitor(tmvar);
	imagemonitorid = _guider->registerImageMonitor(timvar);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "monitors registered as %ld, %ld",
		monitorid, imagemonitorid);
}

void	GuiderMonitorDialog::unregisterServants() {
	if (monitorid < 0) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "nothing to unregister");
		return;
	}
	// unregister the callbacks
	debug(LOG_DEBUG, DEBUG_LOG, 0, "unregister callbacks");
	_guider->unregisterMonitor(monitorid);
	monitorid = -1;
	_guider->unregisterImageMonitor(imagemonitorid);
	imagemonitorid = -1;

	// remove the servants from the POA
	debug(LOG_DEBUG, DEBUG_LOG, 0, "destroy TrackingMonitor servant");
	PortableServer::POA_var	poa = tm_impl->_default_POA();
	PortableServer::ObjectId_var	tmid = poa->servant_to_id(tm_impl);
	poa->deactivate_object(tmid);
	tm_impl = NULL;

	debug(LOG_DEBUG, DEBUG_LOG, 0, "destroy TrackingImageMonitor servant");
	poa = tim_impl->_default_POA();
	PortableServer::ObjectId_var	timid = poa->servant_to_id(tim_impl);
	poa->deactivate_object(timid);
	tim_impl = NULL;

	debug(LOG_DEBUG, DEBUG_LOG, 0, "Tracking servants removed");
}

/**
 * \brief construct a new GuiderMonitorDialog
 */
GuiderMonitorDialog::GuiderMonitorDialog(Astro::Guider_var guider,
	QWidget *parent) : QDialog(parent),
		_guider(guider),
		ui(new Ui::GuiderMonitorDialog)
{
	ui->setupUi(this);

	// set the color in which the curves will be drawn
	ui->xhistoryWidget->setColor(QColor(255., 0., 0.));
	ui->yhistoryWidget->setColor(QColor(0., 0., 255.));

	// get the history data from the guider
	Astro::TrackingHistory_var	history = guider->getTrackingHistory(-1);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "history has %d entries",
		history->length());
	
	// get a list of values
	std::list<double>	xvalues;
	std::list<double>	yvalues;
	unsigned int	startindex = 0;
	if (history->length() > ui->xhistoryWidget->width()) {
		startindex = history->length() - ui->xhistoryWidget->width();
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "use history data from index %lu",
		startindex);
	for (unsigned int i = startindex; i < history->length(); i++) {
		Astro::TrackingInfo	ti = (history)[i];
		xvalues.push_back(ti.trackingoffset.x);
		yvalues.push_back(ti.trackingoffset.y);
	}

	// add all values to the 
	ui->xhistoryWidget->add(xvalues);
	ui->yhistoryWidget->add(yvalues);

	// initialize 
	data = NULL;

	// create the mutex
	pthread_mutexattr_t	mattr;
	pthread_mutexattr_init(&mattr);
	pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&mutex, &mattr);
	pthread_mutex_lock(&mutex);

	// make sure these are properly initialized
	tm_impl = NULL;
	monitorid = -1;
	tim_impl = NULL;
	imagemonitorid = -1;

	// register servants
	registerServants();

	// connect signals
	connect(this, SIGNAL(trackingInfoUpdated()), this,
		SLOT(displayTrackingInfo()), Qt::QueuedConnection);
	connect(this, SIGNAL(trackingImageUpdated()), this,
		SLOT(displayTrackingImage()), Qt::QueuedConnection);

	connect(this, SIGNAL(stop()), this, SLOT(terminate()),
		Qt::QueuedConnection);

	connect(this, SIGNAL(xUpdate(double)), ui->xhistoryWidget,
		SLOT(add(double)), Qt::QueuedConnection);
	connect(this, SIGNAL(yUpdate(double)), ui->yhistoryWidget,
		SLOT(add(double)), Qt::QueuedConnection);

	// release the lock
	pthread_mutex_unlock(&mutex);
}

/**
 * \brief Destroy the GuiderMonitorDialog
 */
GuiderMonitorDialog::~GuiderMonitorDialog() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "destroy guider monitor dialog");
	// unregister the callback servants
	unregisterServants();

	// destroy the GUI part
	delete ui;
}

/**
 * \brief Update the Tracking info
 */
void	GuiderMonitorDialog::update(const Astro::TrackingInfo& ti) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "trackingInfo udpated");
	trackinginfo = ti;
	emit trackingInfoUpdated();
	emit xUpdate(ti.trackingoffset.x);
	emit yUpdate(ti.trackingoffset.y);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "trackingInfoUdpated signal emited");
}

/**
 * \brief Display the Trakicng info
 *
 * This slot should always be called on the main thread.
 */
void	GuiderMonitorDialog::displayTrackingInfo() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "displayTrackingInfo");
	double	t = astro::Timer::gettime() - trackinginfo.timeago;
	time_t	t0 = trunc(t);
	char	buffer[30];
	struct tm	*tmp = localtime(&t0);
	int	bytes = strftime(buffer, sizeof(buffer), "%H:%M:%S", tmp);
	snprintf(buffer + bytes, sizeof(buffer) - bytes, ".%03d",
		(int)trunc(1000 * (t - t0)));
	ui->timeField->setText(buffer);
	snprintf(buffer, sizeof(buffer), "%.4f", trackinginfo.trackingoffset.x);
	ui->xField->setText(buffer);
	snprintf(buffer, sizeof(buffer), "%.4f", trackinginfo.trackingoffset.y);
	ui->yField->setText(buffer);
	snprintf(buffer, sizeof(buffer), "%.4f", trackinginfo.activation.x);
	ui->raField->setText(buffer);
	snprintf(buffer, sizeof(buffer), "%.4f", trackinginfo.activation.y);
	ui->decField->setText(buffer);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "displayTrackingInfo complete");
}

/**
 * \brief Update the Image info
 *
 * This method uses locking to ensure that updating the data and
 * displaying are properly serialized. Not doing this results in
 * crashes.
 */
void	GuiderMonitorDialog::update(const Astro::ImageSize& _size,
		const Astro::ShortSequence& imagedata) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new %dx%d image received",
		_size.width, _size.height);
	pthread_mutex_lock(&mutex);
	imagesize = _size;
	unsigned long	l = imagesize.width * imagesize.height;
	unsigned short	*newdata
		= new unsigned short[l];
	for (unsigned int i = 0; i < l; i++) {
		newdata[i] = imagedata[i];
	}
	unsigned short	*olddata = data;
	data = newdata;
	if (olddata) {
		delete olddata;
	}
	pthread_mutex_unlock(&mutex);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "emit ImageUpdated signal");
	emit trackingImageUpdated();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "signal emitted");
}

/**
 * \brief convert a CORBA image into a QPixmap
 *
 * As side effect, this method accumulates statistics data in the stats
 * argument. The method can handle byte or unsigned short data images.
 * \param image	Image object reference.
 * \param stats	Statistics about the image
 */
QPixmap	GuiderMonitorDialog::image2pixmap(const Astro::ImageSize& size,
		const unsigned short *imagedata) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "convert image of size %dx%d to QPixmap",
		size.width, size.height);
	QImage	qimage(size.width, size.height, QImage::Format_RGB32);
	unsigned int	l = size.width * size.height;

	// find maximum, minimum and mean value
	double	max = 0;
	double	min = 65536;
	for (unsigned int i = 0; i < l; i++) {
		double	v = imagedata[i];
		if (max < v) { max = v; }
		if (min > v) { min = v; }
	}
	double	scale = 255. / ((double)max - (double)min);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "scale = %f", scale);

	// convert the image to pixmap to display in the image label
	for (int x = 0; x < size.width; x++) {
		for (int y = 0; y < size.height; y++) {
			double	dv = imagedata[x + size.width * y];
			unsigned char	v = round(scale * (dv - min));
			unsigned long	value = (0xff000000) | (v << 16) | (v << 8) | v;
			qimage.setPixel(x, size.height - 1 - y, value);
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "pixels copied");

	// convert data into an image
	QPixmap	pixmap(size.width, size.height);
	pixmap.convertFromImage(qimage);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image of size %d x %d created",
		size.width, size.height);
	return pixmap;
}

/**
 * \brief Display the tracking image
 */
void	GuiderMonitorDialog::displayTrackingImage() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "display tracking image");
	pthread_mutex_lock(&mutex);
	if (data) {
		QPixmap	pixmap = image2pixmap(imagesize, data);
		ui->imageLabel->setPixmap(pixmap);
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no image data");
	}
	pthread_mutex_unlock(&mutex);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "tracking image complete");
}

/**
 * \brief Handle closing the window
 *
 * When the window is closed, the callback servants associated with this
 * monitor should be removed.
 */
void	GuiderMonitorDialog::closeEvent(QCloseEvent *event) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "closing the dialog");
	unregisterServants();
}

void	GuiderMonitorDialog::requestStop() {
	emit stop();
}

void	GuiderMonitorDialog::terminate() {
	close();
}

namespace guidermonitor {
//////////////////////////////////////////////////////////////////////
// TrackingMonitor implementation, inside separate namespace
//////////////////////////////////////////////////////////////////////

TrackingMonitor_impl::~TrackingMonitor_impl() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "tracking monitor servant destroyed");
}

void	TrackingMonitor_impl::update(const ::Astro::TrackingInfo& ti) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "tracking info received");
	_guidermonitordialog.update(ti);
}

void	TrackingMonitor_impl::stop() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "received stop signal");
	_guidermonitordialog.requestStop();
}

//////////////////////////////////////////////////////////////////////
// TrackingImageMonitor implementation
//////////////////////////////////////////////////////////////////////
TrackingImageMonitor_impl::~TrackingImageMonitor_impl() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "tracking image monitor servant destroyed");
}

void	TrackingImageMonitor_impl::update(const ::Astro::ImageSize& size,
		const ::Astro::ShortSequence& imagedata) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "tracking image received");
	_guidermonitordialog.update(size, imagedata);
}


} // namespace guidermonitor
