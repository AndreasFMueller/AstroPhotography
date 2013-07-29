/*
 * SbigCcd.cpp -- SBIG CCD implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SbigLocator.h>
#include <SbigCcd.h>
#include <sbigudrv.h>
#include <AstroDebug.h>
#include <utils.h>
#include <includes.h>
#include <SbigCooler.h>

using namespace astro::camera;
using namespace astro::image;

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
	: Ccd(info), id(_id), camera(_camera) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "ccd %d: %s", id,
		info.toString().c_str());
}

SbigCcd::~SbigCcd() {
}

/**
 * \brief Query the exposure status.
 *
 * Since the camera interface is closely modelled on the SBIG driver library,
 * this is essentially a call to the corresponding dirver library function.
 */
Exposure::State	SbigCcd::exposureStatus() throw (not_implemented) {
	SbigLock	lock;
	QueryCommandStatusParams	params;
	params.command = CC_START_EXPOSURE2;
	QueryCommandStatusResults	results;
	camera.sethandle();
	short 	e = SBIGUnivDrvCommand(CC_QUERY_COMMAND_STATUS,
		&params, &results);
	if (e != CE_NO_ERROR) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot open command");
	}
	int	s = 0;
	if (id == 0) {
		s = 0x3 & results.status;
	} else {
		s = 0x3 & (results.status >> 2);
	}
	switch (s) {
	case 0:	state = Exposure::idle;
		break;
	case 1: // this is an undefined state, we treat it as exposing
	case 2: state = Exposure::exposing;
		break;
	case 3: state = Exposure::exposed;
		break;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "exposure status ccd %d: %d:", id,
		state);
	return state;
}

/**
 * \brief Start an exposure
 *
 *
 */
void	SbigCcd::startExposure(const Exposure& exposure)
		throw (not_implemented) {
	SbigLock	lock;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "startExposure on ccd %d", id);
	// check whether we are already exposing
	if (state == Exposure::exposing) {
		throw SbigError("exposure already in progress");
	}

	// XXX make sure that the subframe parameters are compatible with the
	//     binning mode

	camera.sethandle();
	this->exposure = exposure;

	// prepare the start exposure2 command for the SBIG library
	debug(LOG_DEBUG, DEBUG_LOG, 0, "starting exposure on ccd %d", id);
	StartExposureParams2	params;
	params.ccd = id;
	params.exposureTime = 100 * exposure.exposuretime;
	params.abgState = ABG_LOW7; // XXX should be able to set via property

	// use the shutter info 
	params.openShutter = (exposure.shutter == SHUTTER_OPEN)
				? ((id == 2)
					? SC_OPEN_EXT_SHUTTER
					: SC_OPEN_SHUTTER)
				: ((id == 2)
					? SC_CLOSE_EXT_SHUTTER
					: SC_CLOSE_SHUTTER);

	// set the appropriate binning mode
	params.readoutMode = SbigBinning2Mode(exposure.mode);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s binning -> readout mode: %04hx",
		exposure.mode.toString().c_str(), params.readoutMode);

	// get the subframe
	params.top = exposure.frame.origin().y();
	params.left = exposure.frame.origin().x();
	params.width = exposure.frame.size().width();
	params.height = exposure.frame.size().height();
	short	e = SBIGUnivDrvCommand(CC_START_EXPOSURE2, &params, NULL);
	if (e != CE_NO_ERROR) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot start exposure: %s",
			sbig_error(e).c_str());
		throw SbigError(e);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "ccd %d exposing", id);

	// now we are exposing
	state = Exposure::exposing;
}

/**
 * \brief Get an Image from the camera.
 *
 * This method waits until the exposure is completed and then downloads the
 * image from the camera.
 */
ImagePtr	SbigCcd::getImage() throw(not_implemented) {
	// we should be in state exposing or exposed. If we are in 
	// state idle, we have a problem
	if (state == Exposure::idle) {
		throw SbigError("camera is idle");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieving short image ccd %d", id);

	// wait until the exposure is complete
	exposureStatus();
	while (Exposure::exposed != exposureStatus()) {
		if (state == Exposure::idle) {
			throw SbigError("suddenly became idle");
		}
		usleep(100000);
	}

	// compute the size of the resulting image, if we get one
	ImageSize	resultsize(
		exposure.frame.size().width() / exposure.mode.getX(),
		exposure.frame.size().height() / exposure.mode.getY());
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
		readparams.readoutMode = SbigBinning2Mode(exposure.mode);

		readparams.top = exposure.frame.origin().y();
		readparams.left = exposure.frame.origin().x();
		readparams.width = exposure.frame.size().width();
		readparams.height = exposure.frame.size().height();

		e = SBIGUnivDrvCommand(CC_START_READOUT, &readparams, NULL);
		if (e != CE_NO_ERROR) {
			debug(LOG_ERR, DEBUG_LOG, 0, "cannot start readout: %s",
				sbig_error(e).c_str());
			throw SbigError(e);
		}

		// read the data lines we really are interested in
		ReadoutLineParams	readlineparams;
		readlineparams.ccd = id;
		readlineparams.pixelStart = exposure.frame.origin().x()
						/ exposure.mode.getX();
		readlineparams.pixelLength = exposure.frame.size().width()
						/ exposure.mode.getX();
		readlineparams.readoutMode = readparams.readoutMode;
debug(LOG_DEBUG, DEBUG_LOG, 0, "pixelStart = %d, pixelLength = %d", 
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
		dumplines.lineLength = info.size.height()
			- exposure.frame.size().height()
			- exposure.frame.origin().y();
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
	image->setOrigin(exposure.frame.origin());

	// add the metadata to the image
	addMetadata(*image);

	// convert the data read to a short image
	return ImagePtr(image);
}

/**
 * \brief Get a Cooler object, if the CCD has a TEC cooler
 */
CoolerPtr	SbigCcd::getCooler() throw (not_implemented) {
	return CoolerPtr(new SbigCooler(camera));
}

/**
 * \brief Query the shutter state
 */
shutter_state	SbigCcd::getShutterState() throw(not_implemented) {
	SbigLock	lock;
	camera.sethandle();

	// get the shutter state from query command status command
	QueryCommandStatusParams	params;
	QueryCommandStatusResults	results;
	params.command = CC_MISCELLANEOUS_CONTROL;
	unsigned short	e;
	e = SBIGUnivDrvCommand(CC_QUERY_COMMAND_STATUS, &params, &results);
	if (e != CE_NO_ERROR) {
		throw not_implemented("cannot query command status");
	}

	shutter_state	state;
	switch ((results.status >> 10) & 0x3) {
	case SS_OPEN:
	case SS_OPENING:
		state = SHUTTER_OPEN;
		break;
	case SS_CLOSED:
	case SS_CLOSING:
		state = SHUTTER_CLOSED;
		break;
	}

	return state;
}

/**
 * \brief Set the shutter state.
 */
void	SbigCcd::setShutterState(const shutter_state& state) throw(not_implemented) {
	SbigLock	lock;
	camera.sethandle();

	// first query the the state of fan and LED so that we can use the
	// right constant in the misc control params
	QueryCommandStatusParams	params;
	params.command = CC_MISCELLANEOUS_CONTROL;
	QueryCommandStatusResults	results;
	unsigned short	e;
	e = SBIGUnivDrvCommand(CC_QUERY_COMMAND_STATUS, &params, &results);
	if (e != CE_NO_ERROR) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"cannot get status, assuming no shutter");
		throw not_implemented("apparently there is no shutter");
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
	case SHUTTER_CLOSED:
		misc.shutterCommand = (id == 2)
					? SC_CLOSE_EXT_SHUTTER
					: SC_CLOSE_SHUTTER;
		break;
	case SHUTTER_OPEN:
		misc.shutterCommand = (id == 2) 
					? SC_OPEN_EXT_SHUTTER
					: SC_OPEN_SHUTTER;
		break;
	}
	e = SBIGUnivDrvCommand(CC_MISCELLANEOUS_CONTROL, &misc, NULL);
	if (e != CE_NO_ERROR) {
		throw not_implemented("shutter command not implemented");
	}
}

} // namespace sbig
} // namespace camera
} // namespace astro
