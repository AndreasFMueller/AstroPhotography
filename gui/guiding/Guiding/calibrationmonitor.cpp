#include "calibrationmonitor.h"
#include "ui_calibrationmonitor.h"
#include <AstroDebug.h>
#include <cstdio>
#include <cmath>
#include <time.h>
#include <sys/time.h>
#include <cassert>

void	CalibrationMonitor::registerServants() {
	// if the callback is already installed, don' t need to do anything
	if (cm_impl) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "calibration monitor alread up");
		return;
	}

	// create a servant
	cm_impl = new calibrationmonitor::CalibrationMonitor_impl(*this);

        // get the root POA
	CORBA::Object_var	obj
		= ConnectionDialog::orb->resolve_initial_references("RootPOA");
	PortableServer::POA_var	root_poa = PortableServer::POA::_narrow(obj);
	assert(!CORBA::is_nil(root_poa));

        // activate the servant, we also remove a reference from the servant
        // so that the destructor is called when the servant is deactivated
        // in the unregisterServant() method
	PortableServer::ObjectId_var	cmid
		= root_poa->activate_object(cm_impl);
	cm_impl->_remove_ref();

        // get a reference to the object, we need it for the registration
        CORBA::Object_var       cmobj
                = root_poa->id_to_reference(cmid);
        Astro::CalibrationMonitor_var      cmvar
                = Astro::CalibrationMonitor::_narrow(cmobj);

        // now register the servant with the guider
        monitorid = _guider->registerCalibrationMonitor(cmvar);
        debug(LOG_DEBUG, DEBUG_LOG, 0, "monitors registered as %ld",
                monitorid);
}

void	CalibrationMonitor::unregisterServants() {
	if (monitorid < 0) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "nothing to unregister");
	}

	// unregister the servants
	_guider->unregisterCalibrationMonitor(monitorid);
	monitorid = -1;

	// remove the servants from the POA
	debug(LOG_DEBUG, DEBUG_LOG, 0, "destroy CalibrationMonitor servant");
	PortableServer::POA_var	poa = cm_impl->_default_POA();
	PortableServer::ObjectId_var	cmid = poa->servant_to_id(cm_impl);
	poa->deactivate_object(cmid);
	cm_impl = NULL;

	debug(LOG_DEBUG, DEBUG_LOG, 0, "Calibration servants removed");
}

CalibrationMonitor::CalibrationMonitor(Astro::Guider_var guider,
	QWidget *parent)
	: QWidget(parent), _guider(guider), ui(new Ui::CalibrationMonitor)
{
	ui->setupUi(this);

	// set some fields in display widgets
	ui->calibrationpointsWidget->grid = true;
	ui->calibrationpointsWidget->circle = false;
	ui->calibrationpointsWidget->color = QColor(0., 0., 255.);
	ui->xerrorWidget->color = QColor(0., 0., 255.);
	ui->xerrorWidget->label = QString("X");
	ui->errorsWidget->grid = false;
	ui->errorsWidget->circle = true;
	ui->errorsWidget->color = QColor(0., 128., 0.);
	ui->yerrorWidget->color = QColor(0., 128., 0.);
	ui->yerrorWidget->label = QString("Y");

	ui->calibrationpointsWidget->setToolTip(
					"Star positions on CCD relative\n"
					"to the first measured point");
	ui->calibrationpointsWidget->setToolTipDuration(10000);
	ui->errorsWidget->setToolTip(	"Residual error when calibration\n"
					"correction is applied to\n"
					"calibration points");
	ui->errorsWidget->setToolTipDuration(10000);
	ui->xerrorWidget->setToolTip(	"Residual error in CCD x-axis\n"
					"after calibration correction");
	ui->xerrorWidget->setToolTipDuration(10000);
	ui->yerrorWidget->setToolTip(	"Residual error in CCD y-axis\n"
					"after calibration correction");
	ui->yerrorWidget->setToolTipDuration(10000);
	
	// get the current calibration
	Astro::Calibration_var	cal;
	try {
		cal = _guider->getCalibration();
		addCalibration(cal);
	} catch (const std::exception&) {
		debug(LOG_ERR, DEBUG_LOG, 0, "no calibration available");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got calibration %d", cal->id);

	// make sure the calibration monitor pointer is zero
	cm_impl = NULL;
	monitorid = -1;

	// create a callback and register it
	registerServants();

	// connect signals
	connect(this, SIGNAL(pointUpdated()), this,
		SLOT(display()), Qt::QueuedConnection);
	connect(this, SIGNAL(stopSignal()), this,
		SLOT(rereadCalibration()), Qt::QueuedConnection);
}

CalibrationMonitor::~CalibrationMonitor()
{
	unregisterServants();

	delete ui;
}

void	CalibrationMonitor::updateWidgets(int i, const Astro::CalibrationPoint& point) {
	// get details of the calibration point
	Astro::Point	off = point.offset;
	Astro::Point	star = point.star;
	double	t = point.t;

	// get an easily accessible array for the coefficients
	double	coef[6];
	for (int i = 0; i < 6; i++) {
		coef[i] = _calibration->coefficients[i];
	}

	// compute errors
	double	ex = coef[0] * off.x + coef[1] * off.y + coef[2] * t;
	ex = star.x - ex;
	double	ey = coef[3] * off.x + coef[4] * off.y + coef[5] * t;
	ey = star.y - ey;

	// point also to the point display widget
	ui->calibrationpointsWidget->addPoint(star);
	
	// compute the correction that the calibraiton will produce
	// and compute the error of the calibration point
	ui->errorsWidget->addPoint(std::make_pair(ex, ey));

	// add errors to the error widgets
	ui->xerrorWidget->addPoint(std::make_pair(t, ex));
	ui->yerrorWidget->addPoint(std::make_pair(t, ey));

	// display details int raw data list
	char	buffer[1924];
	snprintf(buffer, sizeof(buffer),
		"%02d%8.3f%9.3f%8.3f%11.4f%8.3f", i,
		t, off.x, off.y, star.x, star.y);
	ui->calibrationpointList->addItem(buffer);
}

void	CalibrationMonitor::addPoint(const Astro::CalibrationPoint& point) {
	// add the point to the calibration
	int	l = _calibration->points.length();
	_calibration->points.length(l + 1);
	_calibration->points[l] = point;

	// udpate the other objects
	updateWidgets(l, point);

	// signal a redraw
	emit pointUpdated();
}

void	CalibrationMonitor::addCalibration(Astro::Calibration_var calibration) {
	// remember the calibration object
	_calibration = calibration;

	ui->xerrorWidget->clear();
	ui->yerrorWidget->clear();
	ui->calibrationpointsWidget->clear();
	ui->errorsWidget->clear();
	ui->calibrationpointList->clear();

	// add the header
	char	buffer[128];
	snprintf(buffer, sizeof(buffer),
		"No    time       RA     DEC          X       Y");
	ui->calibrationpointList->addItem(buffer);

	// display the raw points in the list
	int	l = calibration->points.length();
	for (int i = 0; i < l; i++) {
		updateWidgets(i, _calibration->points[i]);
	}

	// write the calibraiton id in the title
	snprintf(buffer, sizeof(buffer), "Calibration[%d]", _calibration->id);
	this->setWindowTitle(buffer);

	// display start time
	time_t	t = time(NULL) - _calibration->timeago;;
	struct tm	*tmp = localtime(&t);
	strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tmp);
	ui->startField->setText(buffer);

	// get an easily accessible array for the coefficients
	double	coef[6];
	double	s = 0;
	for (int i = 0; i < 6; i++) {
		coef[i] = _calibration->coefficients[i];
		s += fabs(coef[i]);
	}

	// display the values of the calibration coefficients
	if (s < 0.01) {
		ui->raxField->setText("");
		ui->rayField->setText("");
		ui->decxField->setText("");
		ui->decyField->setText("");
		ui->driftxField->setText("");
		ui->driftyField->setText("");
		ui->errorsWidget->setToolTip(
					"Star position offsets relative to\n"
					"the first star. Will be replaced\n"
					"by residual position errors once\n"
					"the calibration becomes available");
		ui->errorsWidget->setToolTipDuration(10000);
		ui->xerrorWidget->setToolTip(
					"X position offset relative to\n"
					"the first star. Will be replaced\n"
					"by residual position errors once\n"
					"the calibration becomes available");
		ui->xerrorWidget->setToolTipDuration(10000);
		ui->yerrorWidget->setToolTip(
					"Y position offset relative to\n"
					"the first star. Will be replaced\n"
					"by residual position errors once\n"
					"the calibration becomes available");
		ui->yerrorWidget->setToolTipDuration(10000);
	} else {
		ui->raxField->setText(tr("%1").arg(coef[0]));
		ui->rayField->setText(tr("%1").arg(coef[3]));
		ui->decxField->setText(tr("%1").arg(coef[1]));
		ui->decyField->setText(tr("%1").arg(coef[4]));
		ui->driftxField->setText(tr("%1").arg(coef[2]));
		ui->driftyField->setText(tr("%1").arg(coef[5]));
		ui->errorsWidget->setToolTip(
					"Residual error when calibration\n"
					"correction is applied to\n"
					"calibration points");
		ui->errorsWidget->setToolTipDuration(10000);
		ui->xerrorWidget->setToolTip(
					"Residual error in CCD x-axis\n"
					"after calibration correction");
		ui->xerrorWidget->setToolTipDuration(10000);
		ui->yerrorWidget->setToolTip(
					"Residual error in CCD y-axis\n"
					"after calibration correction");
		ui->yerrorWidget->setToolTipDuration(10000);
	}

	// add the information to the to the calibration 
	ui->calibrationWidget->addCalibration(calibration);
}

void	CalibrationMonitor::stopCalibration() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stop received -> emit stopSignal()");
	emit stopSignal();
}

void	CalibrationMonitor::display() {
	repaint();
}

void	CalibrationMonitor::rereadCalibration() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "rereadCalibration reqursted");
	// reread the calibration
	try {
		Astro::Calibration_var	newcal = _guider->getCalibration();
		addCalibration(newcal);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "calibration reread");
	} catch (const std::exception&) {
		debug(LOG_ERR, DEBUG_LOG, 0, "no calibration available");
	}
	repaint();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got calibration %d", _calibration->id);
}

namespace calibrationmonitor {

/**
 * \brief
 */
CalibrationMonitor_impl::CalibrationMonitor_impl(
	::CalibrationMonitor& calibrationmonitor)
	: _calibrationmonitor(calibrationmonitor) {
}

/**
 * \brief Destroy the calibration monitor
 */
CalibrationMonitor_impl::~CalibrationMonitor_impl() {
}

/**
 * \brief update for a new calibration point
 */
void    CalibrationMonitor_impl::update(const ::Astro::CalibrationPoint& cp) {
	_calibrationmonitor.addPoint(cp);
}

/**
 * \brief stop message, calibration process terminates
 */
void    CalibrationMonitor_impl::stop() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got stop notification from server");
	_calibrationmonitor.stopCalibration();
}

}
