/*
 * SbigCcd.cpp -- SBIG CCD implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SbigCcd.h>
#include <sbigudrv.h>
#include <debug.h>
#include <utils.h>
#include <includes.h>
#include <SbigCooler.h>

using namespace astro::camera;
using namespace astro::image;

namespace astro {
namespace camera {
namespace sbig {

SbigCcd::SbigCcd(const CcdInfo& info, int _id, SbigCamera& _camera)
	: Ccd(info), id(_id), camera(_camera) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "ccd %d: %s", id,
		info.toString().c_str());
}

SbigCcd::~SbigCcd() {
}

Exposure::State	SbigCcd::exposureStatus() throw (not_implemented) {
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

void	SbigCcd::startExposure(const Exposure& exposure)
		throw (not_implemented) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "startExposure on ccd %d", id);
	// check whether we are already exposing
	if (state == Exposure::exposing) {
		throw SbigError("exposure already in progress");
	}
	camera.sethandle();
	this->exposure = exposure;

	// prepare the start exposure2 command for the SBIG library
	debug(LOG_DEBUG, DEBUG_LOG, 0, "starting exposure on ccd %d", id);
	StartExposureParams2	params;
	params.ccd = id;
	params.exposureTime = 100 * exposure.exposuretime;
	params.abgState = ABG_LOW7; // XXX should be able to set via property
	params.openShutter = SC_OPEN_SHUTTER; // XXX need way to set this e.g. for darks
	params.readoutMode = RM_1X1; // XXX should be set from exposure.mode
	params.top = exposure.frame.origin.y;
	params.left = exposure.frame.origin.x;
	params.width = exposure.frame.size.width;
	params.height = exposure.frame.size.height;
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

ShortImagePtr	SbigCcd::shortImage() throw(not_implemented) {
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
	readparams.readoutMode = RM_1X1; // XXX should be set from exposure.mode
	readparams.top = exposure.frame.origin.y;
	readparams.left = exposure.frame.origin.x;
	readparams.width = exposure.frame.size.width;
	readparams.height = exposure.frame.size.height;
	e = SBIGUnivDrvCommand(CC_START_READOUT, &readparams, NULL);
	if (e != CE_NO_ERROR) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot start readout: %s",
			sbig_error(e).c_str());
		throw SbigError(e);
	}

	// read the data lines we really are interested in
	ReadoutLineParams	readlineparams;
	readlineparams.ccd = id;
	readlineparams.pixelStart = exposure.frame.origin.x;
	readlineparams.pixelLength = exposure.frame.size.width;
	readlineparams.readoutMode = readparams.readoutMode;
	size_t	arraysize = exposure.frame.size.width
				* exposure.frame.size.height;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "data allocated: %d", arraysize);
	unsigned short	*data = new unsigned short[arraysize];
	unsigned short	*p = data;
	unsigned int	linecounter = 0;
	while (linecounter < exposure.frame.size.height) {
		e = SBIGUnivDrvCommand(CC_READOUT_LINE, &readlineparams, p);
		if (e != CE_NO_ERROR) {
			debug(LOG_ERR, DEBUG_LOG, 0, "error during readout: %s",
				sbig_error(e).c_str());
			delete[] data;
			throw SbigError(e);
		}
		p += exposure.frame.size.width;
		linecounter++;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "read %d lines", linecounter);

	// dump the remaining lines
	DumpLinesParams	dumplines;
	dumplines.ccd = id;
	dumplines.readoutMode = readparams.readoutMode;
	dumplines.lineLength = info.size.height - exposure.frame.size.height
		- exposure.frame.origin.y;
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

	// convert the image data into an image
	Image<unsigned short>	*image
		= new Image<unsigned short>(exposure.frame.size, data);

	// convert the data read to a short image
	return ShortImagePtr(image);
}

CoolerPtr	SbigCcd::getCooler() throw (not_implemented) {
	return CoolerPtr(new SbigCooler(camera));
}

} // namespace sbig
} // namespace camera
} // namespace astro
