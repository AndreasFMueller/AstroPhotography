/*
 * FocusingI.cpp -- focusing servant implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <FocusingI.h>
#include <IceConversions.h>
#include <Ice/Connection.h>
#include <ProxyCreator.h>
#include <AstroDebug.h>

namespace snowstar {

/**
 * \brief Create a Focusing object
 *
 * \param focusingptr	the Focusing process to base this servant on
 */
FocusingI::FocusingI(astro::focusing::FocusingPtr focusingptr) {
	_focusingptr = focusingptr;
	astro::callback::CallbackPtr	cb
		= astro::callback::CallbackPtr(new FocusingCallback(*this));
	_focusingptr->callback(cb);
}

/**
 * \brief Destroy the Focusing
 */
FocusingI::~FocusingI() {
}

/**
 * \brief Get the current status
 */
FocusState	FocusingI::status(const Ice::Current& /* current */) {
	return convert(_focusingptr->status());
}

/**
 * \brief provide the Method
 */
std::string	FocusingI::method(const Ice::Current& /* current */) {
	return _focusingptr->method();
}

/**
 * \brief Set the method
 */
void	FocusingI::setMethod(const std::string& method,
		const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "set the method to %s",
		method.c_str());
	_focusingptr->method(method);
}

/**
 * \brief provide the Solver
 */
std::string	FocusingI::solver(const Ice::Current& /* current */) {
	return _focusingptr->solver();
}

/**
 * \brief Set the solver
 */
void	FocusingI::setSolver(const std::string& solver,
		const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "set the solver to %s",
		solver.c_str());
	_focusingptr->solver(solver);
}

/**
 * \brief provide the information about the exposure
 */
Exposure	FocusingI::getExposure(const Ice::Current& /* current */) {
	return convert(_focusingptr->exposure());
}

/**
 * \brief Set the exposure for the focusing process
 */
void	FocusingI::setExposure(const Exposure& exposure,
		const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "set exposure");
	_focusingptr->exposure(convert(exposure));
}

/**
 * \brief provide the number of steps
 */
int	FocusingI::steps(const Ice::Current& /* current */) {
	return _focusingptr->steps();
}

/**
 * \brief Set the number of steps
 */
void	FocusingI::setSteps(int steps, const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "set steps to %d", steps);
	_focusingptr->steps(steps);
}

/**
 * \brief Start the focusing process
 */
void	FocusingI::start(int min, int max, const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start focusing in interval [%d,%d]",
		min, max);
	// ensure we are in the right state
	switch (_focusingptr->status()) {
	case astro::focusing::Focus::MOVING:
	case astro::focusing::Focus::MEASURING:
	case astro::focusing::Focus::MEASURED:
		throw BadState(std::string("currently focusing"));
	default:
		break;
	}
	try {
		// clear the history
		_history.clear();
		// start the focusing
		_focusingptr->start(min, max);
	} catch (const std::exception& x) {
		throw BadState(std::string("cannot start focusing: ")
			+ std::string(x.what()));
	} catch (...) {
		throw BadState(std::string("cannot start focusing"));
	}
}

/**
 * \brief Cancel the focusing process in progress
 */
void	FocusingI::cancel(const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "cancelling the focusing");
	_focusingptr->cancel();
}

/**
 * \brief Provide a CCD proxy
 */
CcdPrx	FocusingI::getCcd(const Ice::Current& current) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating the CCD proxy");
	std::string	name = _focusingptr->ccd()->name();
	return createProxy<CcdPrx>(name, current);
}

/**
 * \brief Provide a Focuser proxy
 */
FocuserPrx	FocusingI::getFocuser(const Ice::Current& current) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating the focuser proxy");
	std::string	name = _focusingptr->focuser()->name();
	return createProxy<FocuserPrx>(name, current);
}

/**
 * \brief retrieve the focus history
 */
FocusHistory	FocusingI::history(const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve the history");
	return _history;
}

/**
 * \brief send an add point callback to all registered callbacks
 */
void	FocusingI::addPoint(const FocusPoint& point) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "adding a point %d: %f",
		point.position, point.value);
	_history.push_back(point);
}

/**
 * \brief register a callback 
 */
void	FocusingI::registerCallback(const Ice::Identity& callbackidentity,
		const Ice::Current& current) {
	callbacks.registerCallback(callbackidentity, current);
}

/**
 * \brief unregister a callback
 */
void	FocusingI::unregisterCallback(const Ice::Identity& callbackidentity,
		const Ice::Current& current) {
	callbacks.unregisterCallback(callbackidentity, current);
}

/**
 * \brief Update fromthe callback
 */
void	FocusingI::updateFocusing(astro::callback::CallbackDataPtr data) {
	// FocusCallbackData
	astro::focusing::FocusCallbackData	*focusdata
		= dynamic_cast<astro::focusing::FocusCallbackData *>(&*data);
	if (NULL != focusdata) {
		FocusPoint	p;
		p.position = focusdata->position();
		p.value = focusdata->value();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "callback position=%d value=%f",
			p.position, p.value);
		addPoint(p);
		if (focusdata->image() && imagerepo()) {
			try {
				imagerepo()->save(focusdata->image());
			} catch (const std::exception& ex) {
				debug(LOG_ERR, DEBUG_LOG, 0, "cannot save "
					"processed image to repo: %s",
					ex.what());
			}
		}
	}

	// ImageCallback
	astro::callback::ImageCallbackData	*imagedata
		= dynamic_cast<astro::callback::ImageCallbackData *>(&*data);
	if (NULL != imagedata) {
		if ((imagedata->image()) && (imagerepo())) {
			try {
				debug(LOG_DEBUG, DEBUG_LOG, 0, "got %s",
					imagedata->image()->info().c_str());
				imagerepo()->save(imagedata->image());
			} catch (const std::exception& ex) {
				debug(LOG_ERR, DEBUG_LOG, 0, "cannot save"
					" raw image to repo: %s", ex.what());
			}
		}
	}

	// other callbacks
	callbacks(data);
}

/**
 * \brief set the repository name
 */
void    FocusingI::setRepositoryName(const std::string& reponame,
                                const Ice::Current& current) {
	RepositoryUser::setRepositoryName(reponame, current);
}

/**
 * \brief get the repository name
 */
std::string     FocusingI::getRepositoryName(const Ice::Current& current) {
	return RepositoryUser::getRepositoryName(current);
}

} // namespace snowstar
