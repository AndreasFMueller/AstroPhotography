/*
 * mountcontrollerwidget.cpp
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "mountcontrollerwidget.h"
#include "ui_mountcontrollerwidget.h"
#include <QMessageBox>
#include <IceConversions.h>

namespace snowgui {

/**
 * \brief Create a new mount controller widget
 *
 * \param parent	the parent widget
 */
mountcontrollerwidget::mountcontrollerwidget(QWidget *parent)
	: InstrumentWidget(parent),
	  ui(new Ui::mountcontrollerwidget) {
	ui->setupUi(this);

	qRegisterMetaType<astro::device::Mount::state_type>(
		"astro::device::Mount::state_type");
	qRegisterMetaType<astro::RaDec>("astro::RaDec");

	setTabOrder(ui->targetRaField, ui->targetDecField);
	setTabOrder(ui->targetDecField, ui->targetRaField);

	_previousstate = snowstar::MountIDLE;
	_previouswest = true;

	connect(ui->gotoButton, SIGNAL(clicked()),
		this, SLOT(gotoClicked()));
	connect(ui->viewskyButton, SIGNAL(clicked()),
		this, SLOT(viewskyClicked()));
	connect(ui->catalogButton, SIGNAL(clicked()),
		this, SLOT(catalogClicked()));

	// create the update thread
	_updatethread = new QThread(NULL);
	connect(_updatethread, SIGNAL(finished()),
		_updatethread, SLOT(deleteLater()));
	_updatethread->start();

	// create the work class
	_updatework = new mountupdatework(this);
	_updatework->moveToThread(_updatethread);

	// initialize the timer
	_statusTimer.setInterval(1000);
	connect(&_statusTimer, SIGNAL(timeout()),
		_updatework, SLOT(statusUpdate()));

	// auxiliary window initialization
	_skydisplay = NULL;
	_catalogdialog = NULL;
}

/**
 * \brief Destroy the mount controller widget
 */
mountcontrollerwidget::~mountcontrollerwidget() {
	_statusTimer.stop();
	//_updatethread->terminate();
	if (_skydisplay) {
		delete _skydisplay;
	}
	if (_catalogdialog) {
		delete _catalogdialog;
	}
	delete ui;
}

/**
 * \brief Setup the instrument
 *
 * \param serviceobject		the service discovery object
 * \param instrument		the remote instrument
 */
void	mountcontrollerwidget::instrumentSetup(
		astro::discover::ServiceObject serviceobject,
		snowstar::RemoteInstrument instrument) {
	// parent setup
	InstrumentWidget::instrumentSetup(serviceobject, instrument);

	// read information about mounts
	int	index = 0;
	while (_instrument.has(snowstar::InstrumentMount, index)) {
		snowstar::MountPrx	mount = _instrument.mount(index);
		if (!mount) {
			debug(LOG_ERR, DEBUG_LOG, 0, "no mount at index %d",
				index);
		}
		if (!_mount) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "found a mount");
			_mount = mount;
		}
		std::string	sn = _instrument.displayname(
					snowstar::InstrumentMount, index,
					serviceobject.name());
		ui->mountSelectionBox->addItem(QString(sn.c_str()));
		index++;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found total of %d mounts", index);
}

/**
 * \brief Main thread initializations for the mount
 */
void	mountcontrollerwidget::setupComplete() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "setup complete");
	setupMount();
}

/**
 * \brief setup the mount
 */
void	mountcontrollerwidget::setupMount() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "setup the mount");
	_statusTimer.stop();
	_previousstate = snowstar::MountIDLE;
	if (_mount) {
		// read longitude and latitude from the mount
		_position = convert(_mount->getLocation());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "mount location: %s",
			_position.toString().c_str());

		// make sure the star chart knows the orientation
		_previouswest = _mount->telescopePositionWest();
		emit orientationChanged(_previouswest);

		// make sure everybody knows our direction
		_telescope = _mount->getRaDec();
		emit telescopeChanged(convert(_telescope));

		// write the position to the position label
		std::string	pl;
#if 0
		pl += astro::stringprintf("%.4f",
			fabs(_position.longitude().degrees()));
#else
		pl += _position.longitude().dms(':', 0).substr(1);
#endif
		pl += (_position.longitude().degrees() < 0) ? "W" : "E";
		pl += " ";
#if 0
		pl += astro::stringprintf("%.4f",
			fabs(_position.latitude().degrees()));
#else
		pl += _position.latitude().dms(':', 0).substr(1);
#endif
		pl += (_position.longitude().degrees() < 0) ? "S" : "N";
		ui->observatoryField->setText(QString(pl.c_str()));

		// write the position to the LMST widget
		ui->siderealTime->position(_position);
		ui->hourangleWidget->position(_position);

		// try to get the time
		try {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "trying to get time");
			emit updateTime(_mount->getTime());
		} catch (std::exception& x) {
			debug(LOG_ERR, DEBUG_LOG, 0, "cannot update time: %s",
				x.what());
		}
		
		// turn on the buttons
		ui->targetRaField->setEnabled(true);
		ui->targetDecField->setEnabled(true);
		ui->gotoButton->setEnabled(true);
		ui->viewskyButton->setEnabled(true);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "start the mount timer");
		_statusTimer.start();
	} else {
		ui->targetRaField->setEnabled(false);
		ui->targetDecField->setEnabled(false);
		ui->gotoButton->setEnabled(false);
		ui->gotoButton->setText(QString("GOTO"));
		ui->viewskyButton->setEnabled(false);
		ui->currentRaField->setText(QString("(idle)"));
		ui->currentDecField->setText(QString("(idle)"));
		ui->observatoryField->setText(QString("(unknown)"));
	}
}

static QString	rangemessage("The RA value must be between 0 and 24 hours, "
			"and the DEC value must be between -90° and +90°");

/**
 * \brief What to do when the user clicks the goto button
 */
void	mountcontrollerwidget::gotoClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "goto clicked");
	if (!_mount) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no mount present");
		return;
	}
	switch (_previousstate) {
	case snowstar::MountGOTO:
		_mount->cancel();
		return;
	default:
		break;
	}
	snowstar::RaDec	radec;
	bool	ok;
	radec.ra = ui->targetRaField->text().toDouble(&ok);
	if (!ok) {
		std::string	f(ui->targetRaField->text().toLatin1().data());
		try {
			radec.ra = astro::Angle::hms_to_angle(f).hours();
		} catch (const std::exception& x) {
			debug(LOG_ERR, DEBUG_LOG, 0, "cannot parse '%s': %s",
				f.c_str(), x.what());
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found RA = %.4f", radec.ra);
	if ((radec.ra < 0) || (radec.ra > 24)) {
		QMessageBox	message(this);
		message.setText(QString("Invalid RA"));
		message.setInformativeText(rangemessage);
		message.exec();
		return;
	}
	radec.dec = ui->targetDecField->text().toDouble(&ok);
	if (!ok) {
		std::string	f(ui->targetDecField->text().toLatin1().data());
		try {
			radec.dec = astro::Angle::dms_to_angle(f).degrees();
		} catch (const std::exception& x) {
			debug(LOG_ERR, DEBUG_LOG, 0, "cannot parse '%s': %s",
				f.c_str(), x.what());
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found DEC = %.4f", radec.dec);
	if ((radec.dec < -90) || (radec.dec > 90)) {
		QMessageBox	message(this);
		message.setText(QString("Invalid DEC"));
		message.setInformativeText(rangemessage);
		message.exec();
		return;
	}
	_mount->GotoRaDec(radec);
}

/**
 * \brief Slot called when the timer expires
 */
void	mountcontrollerwidget::statusUpdate() {
	if (!_mount) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no active mount");
		return;
	}
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "Mount status udpate");
	snowstar::mountstate state = _mount->state();
	if (state != _previousstate) {
		emit stateChanged(convert(state));
		_previousstate = state;
		switch (state) {
		case snowstar::MountIDLE:
			ui->currentRaField->setText(QString("(idle)"));
			ui->currentDecField->setText(QString("(idle)"));
			ui->gotoButton->setText(QString("GOTO"));
			ui->gotoButton->setEnabled(false);
			return;
		case snowstar::MountALIGNED:
			ui->currentRaField->setText(QString("(aligned)"));
			ui->currentDecField->setText(QString("(aligned)"));
			ui->gotoButton->setText(QString("GOTO"));
			ui->gotoButton->setEnabled(true);
			return;
		case snowstar::MountTRACKING:
			ui->gotoButton->setText(QString("GOTO"));
			ui->gotoButton->setEnabled(true);
			break;
		case snowstar::MountGOTO:
			ui->gotoButton->setText(QString("Cancel"));
			ui->gotoButton->setEnabled(true);
			break;
		}
	}

	// check the side of the telescope on the mount
	bool	west = _mount->telescopePositionWest();
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "telescope orientation: %s",
	//	(west) ? "west" : "east");
	if (west != _previouswest) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "emit orientation change to %s",
			(west) ? "west" : "east");
		emit orientationChanged(west);
		_previouswest = west;
	}

	// read the current position from the mount
	snowstar::RaDec	radec = _mount->getRaDec();
	astro::RaDec	rd = convert(radec);
	if (rd != convert(_telescope)) {
		emit telescopeChanged(rd);
	}
	ui->currentRaField->setText(QString(
		rd.ra().hms(':',1).c_str()));
	ui->currentDecField->setText(QString(
		rd.dec().dms(':',0).c_str()));
	_telescope = radec;
	ui->hourangleWidget->ra(rd.ra());

	// read the current time from the mount
	time_t	now = _mount->getTime();
	emit updateTime(now);
}

/**
 * \brief Slot called when the selection of the mount changes
 */
void	mountcontrollerwidget::mountChanged(int index) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "mount changed to %d", index);
	_mount = _instrument.mount(index);
	setupMount();
	emit mountSelected(index);
}

/**
 * \brief get the RA and DEC from the mount
 */
astro::RaDec	mountcontrollerwidget::current() {
	if (_mount) {
		snowstar::RaDec	radec = _mount->getRaDec();
		return convert(radec);
	} else {
		throw std::runtime_error("cannot get current position without "
			"a mount");
	}
}

/**
 * \brief set the target
 *
 * This sets the position where the telescope is supposed to move next
 */
void	mountcontrollerwidget::setTarget(const astro::RaDec& target) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "setting new target: %s",
		target.toString().c_str());
	astro::Angle	ra = target.ra();
	while (ra > astro::Angle(24, astro::Angle::Hours)) {
		ra = ra - astro::Angle(24, astro::Angle::Hours);
	}
	while (ra < astro::Angle(0, astro::Angle::Hours)) {
		ra = ra + astro::Angle(24, astro::Angle::Hours);
	}
	astro::Angle	dec = target.dec();
	ui->targetRaField->setText(QString(ra.hms(':', 1).c_str()));
	ui->targetDecField->setText(QString(dec.dms(':', 0).c_str()));
	_target = snowstar::convert(astro::RaDec(ra, dec));

	// if the _skyview is open also change the target there
	if (_skydisplay) {
		_skydisplay->targetChanged(convert(_target));
	}
}

/**
 * \brief Method called when the view Sky button is clicked
 */
void	mountcontrollerwidget::viewskyClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "viewskyClicked()");
	if (_skydisplay) {
		_skydisplay->raise();
		return;
	}

	// create a new SkyDisplayWidget
	_skydisplay = new SkyDisplayDialog(NULL);
	_skydisplay->position(_position);
	astro::RaDec	radec = current();
	_skydisplay->telescope(radec);

	// connect the widget
	connect(this, SIGNAL(telescopeChanged(astro::RaDec)),
		_skydisplay, SLOT(telescopeChanged(astro::RaDec)));
	connect(_skydisplay, SIGNAL(pointSelected(astro::RaDec)),
		this, SLOT(targetChanged(astro::RaDec)));
	connect(_skydisplay, SIGNAL(destroyed()),
		this, SLOT(skyviewDestroyed()));

	// show the window
	_skydisplay->show();
}

/*
 * \brief Slot to call when the skyview widget is destroyed
 */
void	mountcontrollerwidget::skyviewDestroyed() {
	_skydisplay = NULL;
}

void	mountcontrollerwidget::catalogDestroyed() {
	_catalogdialog = NULL;
}

/**
 * \brief Slot to accept the new position
 */
void	mountcontrollerwidget::targetChanged(astro::RaDec newtarget) {
	// compute the difference to the current target
	astro::RaDec	correction = newtarget - convert(_telescope);

	// emit the movement in RA/DEC
	emit radecCorrection(correction,_previouswest);

	// set the new target
	setTarget(newtarget);

}

/**
 * \brief Slot called to bring up the catalog
 */
void	mountcontrollerwidget::catalogClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "catalogClicked()");
	if (_catalogdialog) {
		_catalogdialog->raise();
		return;
	}
	_catalogdialog = new CatalogDialog(NULL);
	connect(_catalogdialog, SIGNAL(objectSelected(astro::RaDec)),
		this, SLOT(targetChanged(astro::RaDec)));
	connect(_catalogdialog, SIGNAL(destroyed()),
		this, SLOT(catalogDestroyed()));
	_catalogdialog->show();
}

} // namespace snowgui
