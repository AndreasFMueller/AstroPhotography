/*
 * InstrumentSetupThread.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <InstrumentWidget.h>

namespace snowgui {

/**
 * \brief Construct an instrument setup thread
 */
InstrumentSetupThread::InstrumentSetupThread(InstrumentWidget *instrumentwidget,
	snowstar::RemoteInstrument remoteinstrument,
	astro::discover::ServiceObject serviceobject)
	: QThread(NULL), _instrumentwidget(instrumentwidget),
	  _remoteinstrument(remoteinstrument),
	  _serviceobject(serviceobject) {
	// make sure the thread object is destroyed when it terminates
	connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
	connect(this, &InstrumentSetupThread::setupCompletion,
		instrumentwidget, &InstrumentWidget::setupCompletion);
}

/**
 * \brief Destroy the instrument setup
 */
InstrumentSetupThread::~InstrumentSetupThread() {
	std::string	t = astro::demangle(typeid(*_instrumentwidget).name());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s setup thread destroyed", t.c_str());
}

/**
 * \brief Work method for the thread
 */
void	InstrumentSetupThread::run() {
	std::string	t = astro::demangle(typeid(*_instrumentwidget).name());
	try {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "start the work on %s setup", t.c_str());
		_instrumentwidget->instrumentSetup(_serviceobject, _remoteinstrument);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "emit signal for %s", t.c_str());
		emit setupCompletion();
	} catch (const std::exception& x) {
		std::string	msg("cannot setup instrument");
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "work on %s setup complete", t.c_str());
}

} // namespace snowgui
