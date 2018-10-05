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

	setTabOrder(ui->targetRaField, ui->targetDecField);
	setTabOrder(ui->targetDecField, ui->targetRaField);

	_previousstate = snowstar::MountIDLE;

	connect(ui->gotoButton, SIGNAL(clicked()),
		this, SLOT(gotoClicked()));
	connect(ui->viewskyButton, SIGNAL(clicked()),
		this, SLOT(viewskyClicked()));

	_statusTimer.setInterval(1000);

	connect(&_statusTimer, SIGNAL(timeout()),
		this, SLOT(statusUpdate()));

	_skydisplay = NULL;
}

/**
 * \brief Destroy the mount controller widget
 */
mountcontrollerwidget::~mountcontrollerwidget() {
	_statusTimer.stop();

	if (_skydisplay) {
		delete _skydisplay;
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

	// setup the mount
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
		if (_mount->hasParameter("longitude")) {
			_position.longitude().degrees(
				_mount->parameterValueFloat("longitude"));
		} else {
			debug(LOG_ERR, DEBUG_LOG, 0,
				"longitude parameter not set for mount");
		}
		if (_mount->hasParameter("latitude")) {
			_position.latitude().degrees(
				_mount->parameterValueFloat("latitude"));
		} else {
			debug(LOG_ERR, DEBUG_LOG, 0,
				"latitude parameter not set for mount");
		}

		// write the position to the position label
		std::string	pl;
		pl += astro::stringprintf("%.4f",
			fabs(_position.longitude().degrees()));
		pl += (_position.longitude().degrees() < 0) ? "W" : "E";
		pl += " ";
		pl += astro::stringprintf("%.4f",
			fabs(_position.latitude().degrees()));
		pl += (_position.longitude().degrees() < 0) ? "S" : "N";
		ui->observatoryField->setText(QString(pl.c_str()));
		
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
	radec.ra = ui->targetRaField->text().toDouble();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found RA = %.4f", radec.ra);
	if ((radec.ra < 0) || (radec.ra > 24)) {
		QMessageBox	message(this);
		message.setText(QString("Invalid RA"));
		message.setInformativeText(rangemessage);
		message.exec();
		return;
	}
	radec.dec = ui->targetDecField->text().toDouble();
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "Mount status udpate");
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
	snowstar::RaDec	radec = _mount->getRaDec();
	double	deltaRa = (_telescope.ra - radec.ra);
	if (deltaRa > M_PI) { deltaRa -= 2 *M_PI; }
	if (deltaRa < -M_PI) { deltaRa += 2 * M_PI; }
	double	deltaDec = (_telescope.dec - radec.dec);
	double	change = hypot(deltaRa, deltaDec);
	if (change > 0.001) {
		ui->currentRaField->setText(QString(
			astro::stringprintf("%.4f", radec.ra).c_str()));
		ui->currentDecField->setText(QString(
			astro::stringprintf("%.4f", radec.dec).c_str()));
		_telescope = radec;
		emit telescopeChanged(convert(radec));
	}
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
	_target.ra = target.ra().hours();
	while (_target.ra < 0) { _target.ra += 24; }
	while (_target.ra >= 24) { _target.ra -= 24; }
	_target.dec = target.dec().degrees();
	ui->targetRaField->setText(QString(astro::stringprintf("%.4f",
		_target.ra).c_str()));
	ui->targetDecField->setText(QString(astro::stringprintf("%.4f",
		_target.dec).c_str()));

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

/**
 * \brief Slot to accept the new position
 */
void	mountcontrollerwidget::targetChanged(astro::RaDec radec) {
	setTarget(radec);
}

} // namespace snowgui
