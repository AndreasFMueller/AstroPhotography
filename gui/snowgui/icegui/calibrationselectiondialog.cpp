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
#include <QListWidgetItem>
#include <QMessageBox>

namespace snowgui {

/**
 * \brief Construct a calibration selection dialog
 */
calibrationselectiondialog::calibrationselectiondialog(QWidget *parent) :
	QDialog(parent), ui(new Ui::calibrationselectiondialog) {
	ui->setupUi(this);
	_calibration.id = -1;

	// create connections
	connect(ui->calibrationlistWidget, SIGNAL(currentRowChanged(int)),
		this, SLOT(currentRowChanged(int)));
	connect(this, SIGNAL(accepted()),
		this, SLOT(calibrationAccepted()));

	// at first, hide the calibration display
	//ui->calibrationdisplayWidget->hide();

	// set the title
	setWindowTitle(QString("Select Calibration"));

	debug(LOG_DEBUG, DEBUG_LOG, 0, "calibrationselectiondialog created");
}

/**
 * \brief Destroy the calibration selection dialog
 */
calibrationselectiondialog::~calibrationselectiondialog() {
	delete ui;
}

/**
 * \brief Create a label for the calibration
 */
static std::string	formatlabel(const snowstar::Calibration& cal) {
	time_t	when = snowstar::converttime(cal.timeago);
	struct tm	*tmp = localtime(&when);
	char	buffer[100];
	strftime(buffer, sizeof(buffer), "%F %T", tmp);
	return astro::stringprintf("%03d: %s, %5.1f%%, %s", cal.id, buffer,
		100 * cal.quality, (cal.east) ? "east" : "west");
}

/**
 * \brief Set the calibration selection for the guider
 */
void	calibrationselectiondialog::setGuider(snowstar::ControlType controltype,
		snowstar::GuiderDescriptor guiderdescriptor,
		snowstar::GuiderFactoryPrx guiderfactory) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "set the calibration selection %s, %s",
		guiderdescriptor.instrumentname.c_str(),
		(controltype == snowstar::ControlGuidePort) ? "GP" : "AO");
	// remember the guider parameters
	_controltype = controltype;
	_guiderdescriptor = guiderdescriptor;
	_guiderfactory = guiderfactory;

	// update the title
	std::string	title = astro::stringprintf("Select calibration for %s of instrument %s",
		(_controltype == snowstar::ControlGuidePort)
			? "Guide Port" : "AO",
		_guiderdescriptor.instrumentname.c_str());
	setWindowTitle(QString(title.c_str()));

	// empty the calibration list
	_calibrations.clear();

	// get all the calibration ids for this guider descriptor
	debug(LOG_DEBUG, DEBUG_LOG, 0, "getting ids for this guider");
	snowstar::idlist	ids
		= _guiderfactory->getCalibrations(_guiderdescriptor,
			controltype);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "guider found %d ids", ids.size());

	// now retrieve each calibration and decide whether to display it
	QFont	font("Fixed");
	font.setStyleHint(QFont::Monospace);
	snowstar::idlist::const_iterator	i;
	for (i = ids.begin(); i != ids.end(); i++) {
		try {
			snowstar::Calibration	cal
				= _guiderfactory->getCalibration(*i);
			debug(LOG_DEBUG, DEBUG_LOG, 0, "%d: type %d, time %.1f",
				*i, cal.type, cal.timeago);
			if ((cal.type == _controltype) && (cal.complete)) {
				_calibrations.push_back(cal);
				std::string	label = formatlabel(cal);
				QString		ls(label.c_str());
				QListWidgetItem	*item = new QListWidgetItem(ls);
				item->setFont(font);
				ui->calibrationlistWidget->addItem(item);
			}
		} catch (const std::exception& ex) {
			debug(LOG_ERR, DEBUG_LOG, 0, "calibration %d not found",
				*i);
		}
	}

	// if there are no calibrations, display a warning message
	if (0 == ids.size()) {
		QMessageBox	*messagebox = new QMessageBox(this);
		messagebox->setWindowModality(Qt::WindowModal);
		messagebox->setText(QString("no calibrations found"));
		messagebox->setInformativeText(QString(
			astro::stringprintf("searching for calibrations for %s for guider %s returned no calibrations",
				(_controltype == snowstar::ControlGuidePort)
					? "Guide Port" : "Adaptive Optics",
				_guiderdescriptor.instrumentname.c_str()).c_str()));
		messagebox->exec();
		delete messagebox;
	}
	
	debug(LOG_DEBUG, DEBUG_LOG, 0, "calibration selection initialized");
}

/**
 * \brief what to do when the row changes
 */
void	calibrationselectiondialog::currentRowChanged(int index) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "calibration row selected: %d", index);
	_calibration = _calibrations[index];
	debug(LOG_DEBUG, DEBUG_LOG, 0, "index %d -> calibration id %d",
		index, _calibration.id);
	ui->calibrationdisplayWidget->setCalibration(_calibration);
	ui->calibrationdisplayWidget->setVisible(true);
}

/**
 * \brief Accept the selected calibration
 */
void	calibrationselectiondialog::calibrationAccepted() {
	if (_calibration.id > 0) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "accepting calibration %d",
			_calibration.id);
		emit calibrationSelected(_calibration);
	}
}

} // namespace snowgui
