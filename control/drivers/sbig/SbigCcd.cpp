/*
 * SbigCcd.cpp -- SBIG CCD implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#ifdef HAVE_LPARDRV_H
#include <lpardrv.h>
#else
#ifdef HAVE_SBIGUDRV_LPARDRV_H
#include <SBIGUDrv/lpardrv.h>
#endif /* HAVE_SBIGUDRV_LPARDRV_H */
#endif

#include <SbigLock.h>
#include <SbigLocator.h>
#include <SbigCcd.h>
#include <AstroOperators.h>
#include <AstroDebug.h>
#include <AstroExceptions.h>
#include <utils.h>
#include <includes.h>
#include <SbigCooler.h>

using namespace astro::camera;
using namespace astro::image;
using namespace astro::image::operators;

namespace astro {
namespace camera {
namespace sbig {

/**
 * \brief Create an SBIG CCD object.
 *
 * SBIG Ccd's are essentially holder objects for the CCD info and a reference
 * to the camera.
 */
SbigCcd::SbigCcd(const CcdInfo& info, int _id, SbigCamera& _camera)
	: Ccd(info), SbigDevice(_camera), id(_id) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "ccd %d: %s", id,
		info.toString().c_str());
	cooler = true;
}

SbigCcd::~SbigCcd() {
}

/**
 * \brief Query the exposure status.
 *
 * Since the camera interface is closely modelled on the SBIG driver library,
 * this is essentially a call to the corresponding dirver library function.
 */
CcdState::State	SbigCcd::exposureStatus() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "checking exposure status");
	QueryCommandStatusParams	params;
	params.command = CC_START_EXPOSURE2;
	QueryCommandStatusResults	results;

	query_command_status(&params, &results);

	int	s = 0;
	if (id == 0) {
		s = 0x3 & results.status;
	} else {
		s = 0x3 & (results.status >> 2);
	}
	switch (s) {
	case 0:	state(CcdState::idle);
		break;
	case 1: // this is an undefined state, we treat it as exposing
	case 2: state(CcdState::exposing);
		break;
	case 3: state(CcdState::exposed);
		break;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "exposure status ccd %d: %s:", id,
		CcdState::state2string(state()).c_str());
	return state();
}

/**
 * \brief Start an exposure
 *
 * This is only possible if the camera is in the idle or exposed state.
 */
void	SbigCcd::startExposure(const Exposure& exposure) {
	SbigLock	lock;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "startExposure on ccd %d", id);

	// do common start exposure stuff
	Ccd::startExposure(exposure);

	// we need to get the camera handle for the SBIG Library
	camera.sethandle();

	// prepare the start exposure2 command for the SBIG library
	debug(LOG_DEBUG, DEBUG_LOG, 0, "starting exposure on ccd %d", id);
	StartExposureParams2	params;
	params.ccd = id;
	params.exposureTime = 100 * exposure.exposuretime();
	params.abgState = ABG_LOW7; // XXX should be able to set via property

	// use the shutter info 
	switch (exposure.shutter()) {
	case Shutter::OPEN:
		params.openShutter = ((id == 2)
					? SC_OPEN_EXT_SHUTTER
					: SC_OPEN_SHUTTER);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "shutter open command: %hd",
			params.openShutter);
		break;
	case Shutter::CLOSED:
		params.openShutter = ((id == 2)
					? SC_CLOSE_EXT_SHUTTER
					: SC_CLOSE_SHUTTER);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "shutter close command: %hd",
			params.openShutter);
		break;
	}

	// set the appropriate binning mode
	params.readoutMode = SbigBinning2Mode(exposure.mode());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s binning -> readout mode: %04hx",
		exposure.mode().toString().c_str(), params.readoutMode);

	// get the subframe
	params.top = exposure.y();
	params.left = exposure.x();
	params.width = exposure.width();
	params.height = exposure.height();
	short	e = SBIGUnivDrvCommand(CC_START_EXPOSURE2, &params, NULL);
	if (e != CE_NO_ERROR) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot start exposure: %s",
			sbig_error(e).c_str());
		throw SbigError(e);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "ccd %d exposing", id);

	// now we are exposing
	state(CcdState::exposing);
}

/**
 * \brief Get an Image from the camera.
 *
 * This method waits until the exposure is completed and then downloads the
 * image from the camera.
 */
ImagePtr	SbigCcd::getRawImage() {
	// we should be in state exposing or exposed. If we are in 
	// state idle, we have a problem
	if (state() == CcdState::idle) {
		throw BadState("camera is idle");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieving short image from ccd %d", id);

	// wait until the exposure is complete
	exposureStatus();
	if (CcdState::exposed != state()) {
		throw BadState("no exposed image available");
	}

	// compute the size of the resulting image, if we get one
	ImageSize	resultsize(
		exposure.width() / exposure.mode().x(),
		exposure.height() / exposure.mode().y());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "expecting an %s image",
		resultsize.toString().c_str());

	// this is where we will find the data, we have to declare it
	// here because everything after this point will be protected
	unsigned short	*data = NULL;

	{
		SbigLock	lock;
		// end the exposure
		camera.sethandle();
		EndExposureParams	endexpparams;
		endexpparams.ccd = id;
		short	e = SBIGUnivDrvCommand(CC_END_EXPOSURE, &endexpparams, NULL);
		if (e != CE_NO_ERROR) {
			debug(LOG_ERR, DEBUG_LOG, 0, "cannot end exposure: %s",
				sbig_error(e).c_str());
			throw SbigError(e);
		}

		// start the readout
		StartReadoutParams	readparams;
		readparams.ccd = id;
		readparams.readoutMode = SbigBinning2Mode(exposure.mode());

		readparams.top = exposure.y();
		readparams.left = exposure.x();
		readparams.width = exposure.width();
		readparams.height = exposure.height();

		e = SBIGUnivDrvCommand(CC_START_READOUT, &readparams, NULL);
		if (e != CE_NO_ERROR) {
			debug(LOG_ERR, DEBUG_LOG, 0, "cannot start readout: %s",
				sbig_error(e).c_str());
			throw SbigError(e);
		}

		// read the data lines we really are interested in
		ReadoutLineParams	readlineparams;
		readlineparams.ccd = id;
		readlineparams.pixelStart = exposure.x()
						/ exposure.mode().x();
		readlineparams.pixelLength = exposure.width()
						/ exposure.mode().x();
		readlineparams.readoutMode = readparams.readoutMode;
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"pixelStart = %d, pixelLength = %d", 
			readlineparams.pixelStart,
			readlineparams.pixelLength);
		size_t	arraysize = resultsize.getPixels();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "data allocated: %d", arraysize);
		data = new unsigned short[arraysize];
		memset(data, 0xff, 2 * arraysize);
		unsigned short	*p = data;
		unsigned int	linecounter = 0;
		while (linecounter < resultsize.height()) {
			e = SBIGUnivDrvCommand(CC_READOUT_LINE, &readlineparams, p);
			if (e != CE_NO_ERROR) {
				debug(LOG_ERR, DEBUG_LOG, 0, "error during readout: %s",
					sbig_error(e).c_str());
				delete[] data;
				throw SbigError(e);
			}
			p += resultsize.width();
			linecounter++;
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "read %d lines", linecounter);

		// dump the remaining lines
		DumpLinesParams	dumplines;
		dumplines.ccd = id;
		dumplines.readoutMode = readparams.readoutMode;
		dumplines.lineLength = info.size().height()
			- exposure.height()
			- exposure.y();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "dumping %d remaining lines",
			dumplines.lineLength);
		e = SBIGUnivDrvCommand(CC_DUMP_LINES, &dumplines, NULL);
		if (e != CE_NO_ERROR) {
			debug(LOG_ERR, DEBUG_LOG, 0, "cannot dump remaining lines: %s"
				" (ignored)", sbig_error(e).c_str());
		}

		// end the readout
		EndReadoutParams	endreadparams;
		endreadparams.ccd = id;
		e = SBIGUnivDrvCommand(CC_END_READOUT, &endreadparams, NULL);
		if (e != CE_NO_ERROR) {
			debug(LOG_ERR, DEBUG_LOG, 0, "cannot end readout: %s",
				sbig_error(e).c_str());
			throw SbigError(e);
		}
	}

	// convert the image data into an image
	Image<unsigned short>	*image
		= new Image<unsigned short>(resultsize, data);
	image->setOrigin(exposure.origin());

	// flip image vertically
	FlipOperator<unsigned short>    f;
	f(*image);

	// add the metadata to the image
	addMetadata(*image);

	// convert the data read to a short image
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve image complete");
        state(CcdState::idle);

	return ImagePtr(image);
}

/**
 * \brief Get a Cooler object, if the CCD has a TEC cooler
 */
CoolerPtr	SbigCcd::getCooler0() {
	// XXX we assume that every SBIG camera has a cooler, which is
	//     obviously incorrect
	DeviceName	devname(name(), DeviceName::Cooler, "cooler");
	return CoolerPtr(new SbigCooler(camera, devname));
}

/**
 * \brief Query the shutter state
 */
Shutter::state	SbigCcd::getShutterState() {
	// get the shutter state from query command status command
	QueryCommandStatusParams	params;
	QueryCommandStatusResults	results;
	params.command = CC_MISCELLANEOUS_CONTROL;
	try {
		query_command_status(&params, &results);
	} catch (const SbigError& x) {
		throw NotImplemented("cannot query command status");
	}

	Shutter::state	state;
	switch ((results.status >> 10) & 0x3) {
	case SS_OPEN:
	case SS_OPENING:
		state = Shutter::OPEN;
		break;
	case SS_CLOSED:
	case SS_CLOSING:
		state = Shutter::CLOSED;
		break;
	}

	return state;
}

/**
 * \brief Set the shutter state.
 */
void	SbigCcd::setShutterState(const Shutter::state& state) {
	SbigLock	lock;
	camera.sethandle();

	// first query the the state of fan and LED so that we can use the
	// right constant in the misc control params
	QueryCommandStatusParams	params;
	params.command = CC_MISCELLANEOUS_CONTROL;
	QueryCommandStatusResults	results;
	try {
		query_command_status(&params, &results);
	} catch (const SbigError& x) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"cannot get status, assuming no shutter");
		throw NotImplemented("apparently there is no shutter");
	}

	// now copy the data
	MiscellaneousControlParams	misc;
	misc.fanEnable = ((results.status >> 8) & 0x1)
		? FS_AUTOCONTROL : FS_OFF;
	switch ((results.status >> 11) & 0x2) {
	case 0:
		misc.ledState = LED_OFF;
		break;
	case 1:
		misc.ledState = LED_ON;
		break;
	case 2:
		misc.ledState = LED_BLINK_LOW;
		break;
	case 3:
		misc.ledState = LED_BLINK_HIGH;
		break;
	}
	switch (state) {
	case Shutter::CLOSED:
		misc.shutterCommand = (id == 2)
					? SC_CLOSE_EXT_SHUTTER
					: SC_CLOSE_SHUTTER;
		break;
	case Shutter::OPEN:
		misc.shutterCommand = (id == 2) 
					? SC_OPEN_EXT_SHUTTER
					: SC_OPEN_SHUTTER;
		break;
	}
	short e = SBIGUnivDrvCommand(CC_MISCELLANEOUS_CONTROL, &misc, NULL);
	if (e != CE_NO_ERROR) {
		throw NotImplemented("shutter command not implemented");
	}
}

} // namespace sbig
} // namespace camera
} // namespace astro
