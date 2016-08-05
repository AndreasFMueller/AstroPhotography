/*
 * calibrationselectiondialog.cpp
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "calibrationselectiondialog.h"
#include "ui_calibrationselectiondialog.h"
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <IceConversions.h>

namespace snowgui {

calibrationselectiondialog::calibrationselectiondialog(QWidget *parent) :
	QDialog(parent), ui(new Ui::calibrationselectiondialog) {
	ui->setupUi(this);

	// create connections
	connect(ui->calibrationlistWidget, SIGNAL(currentRowChanged(int)),
		this, SLOT(currentRowChanged(int)));
	connect(this, SIGNAL(accepted()),
		this, SLOT(calibrationAccepted()));

	// at first, hide the calibration display
	//ui->calibrationdisplayWidget->hide();

	// set the title
	setWindowTitle(QString("Select Calibration"));
}

calibrationselectiondialog::~calibrationselectiondialog() {
	delete ui;
}

static std::string	formatlabel(const snowstar::Calibration& cal) {
	time_t	when = snowstar::converttime(cal.timeago);
	struct tm	*tmp = localtime(&when);
	char	buffer[100];
	strftime(buffer, sizeof(buffer), "%F %T", tmp);
	return astro::stringprintf("%d: %s, %4.1f%%", cal.id, buffer,
		100 * cal.quality);
}

void	calibrationselectiondialog::setGuider(snowstar::ControlType controltype,
		snowstar::GuiderDescriptor guiderdescriptor,
		snowstar::GuiderFactoryPrx guiderfactory) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "setting up the calibration selection");
	// remember the guider parameters
	_controltype = controltype;
	_guiderdescriptor = guiderdescriptor;
	_guiderfactory = guiderfactory;

	// update the title
	std::string	title = astro::stringprintf("Select calibration for %s of instrument %s",
		(_controltype == snowstar::ControlGuiderPort)
			? "Guide Port" : "AO",
		_guiderdescriptor.instrumentname.c_str());
	setWindowTitle(QString(title.c_str()));

	// empty the calibration list
	_calibrations.clear();

	// get all the calibration ids for this guider descriptor
	snowstar::idlist	ids
		= _guiderfactory->getCalibrations(_guiderdescriptor);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d ids", ids.size());

	// now retrieve each calibration and decide whether to display it
	snowstar::idlist::const_iterator	i;
	for (i = ids.begin(); i != ids.end(); i++) {
		snowstar::Calibration	cal
			= _guiderfactory->getCalibration(*i);
		if (cal.type == _controltype) {
			_calibrations.push_back(cal);
			std::string	label = formatlabel(cal);
			ui->calibrationlistWidget->addItem(QString(label.c_str()));
		}
	}
	
	debug(LOG_DEBUG, DEBUG_LOG, 0, "calibration selection initialized");
}

void	calibrationselectiondialog::currentRowChanged(int index) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "row selected: %d", index);
	_calibration = _calibrations[index];
	ui->calibrationdisplayWidget->setCalibration(_calibration);
	ui->calibrationdisplayWidget->setVisible(true);
}

void	calibrationselectiondialog::calibrationAccepted() {
	emit calibrationSelected(_calibration);
}

} // namespace snowgui
