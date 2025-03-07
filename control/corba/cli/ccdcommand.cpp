/*
 * ccdcommand.h -- ccd command implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <ccdcommand.h>
#include <Ccds.h>
#include <iostream>
#include <unistd.h>
#include <limits>
#include <Images.h>
#include <exposurecommand.h>
#include <Conversions.h>
#include <time.h>
#include <Output.h>

namespace astro {
namespace cli {

std::ostream&	operator<<(std::ostream& out, Astro::BinningSet& binningset) {
	for (unsigned int i = 0; i < binningset.length(); i++) {
		if (i > 0) { out << ", "; }
		out << binningset[i];
	}
	return out;
}

std::ostream&	operator<<(std::ostream& out, Astro::ExposureState state) {
	switch (state) {
	case Astro::EXPOSURE_IDLE:
		std::cout << "idle"; break;
	case Astro::EXPOSURE_EXPOSING:
		std::cout << "exposing"; break;
	case Astro::EXPOSURE_EXPOSED:
		std::cout << "exposed"; break;
	case Astro::EXPOSURE_CANCELLING:
		std::cout << "cancelling"; break;
	}
	return out;
}

std::ostream&	operator<<(std::ostream& out, Astro::CcdInfo_var info) {
	std::cout << "id:         " << info->id << std::endl;
	std::cout << "size:       " << info->size.width << " x "
					<< info->size.height << std::endl;
	std::cout << "binning:    " << info->binningmodes << std::endl;
	std::cout << "shutter:    " << ((info->shutter) ? "YES" : "NO")
		<< std::endl;
	std::cout << "pixelsize:  " << (1000000 * info->pixelwidth) << " x "
				<< (1000000 * info->pixelheight) << std::endl;
	return out;
}

void	ccdcommand::info(CcdWrapper& ccd,
		const std::vector<std::string>& /* arguments */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "ccd %s info", ccd->getName());
	std::cout << "name:       " << ccd->getName() << std::endl;
	Astro::CcdInfo_var	info = ccd->getInfo();
	std::cout << info;
	std::cout << "state:      " << ccd->exposureStatus() << std::endl;
	time_t	last = time(NULL) - ccd->lastExposureStart();
	struct tm	*t = localtime(&last);
	char	buffer[80];
	strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", t);
	std::cout << "last image: " << buffer << std::endl;
}

void	ccdcommand::start(CcdWrapper& ccd,
		const std::vector<std::string>& arguments) {
	// get the default CCD size
	Astro::CcdInfo_var      info = ccd->getInfo();
	astro::image::ImageSize	size(info->size.width, info->size.height);

	// initialize the parser with defaults from the CCD
	ExposureParser	parser;
	parser->frame.setSize(size); // set default size

	// parse the command line arguments and modify the exposure structure
	// accordingly
	parser.parse(arguments, 2);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "exposure parsed: %s",
		parser.exposure().toString().c_str());

	// set up a default exposure structure
	Astro::Exposure	exposure;
	exposure.exposuretime = parser->exposuretime;
	exposure.gain = parser->gain;
	exposure.limit = parser->limit;
	exposure.shutter = Astro::SHUTTER_OPEN;
	exposure.mode.x = parser->mode.getX();
	exposure.mode.y = parser->mode.getY();
	exposure.frame.size.width = parser->frame.size().width();
	exposure.frame.size.height = parser->frame.size().height();
	exposure.frame.origin.x = parser->frame.origin().x();
	exposure.frame.origin.y = parser->frame.origin().y();
	exposure.shutter = astro::convert(parser->shutter);

	// start the exposure
	ccd->startExposure(exposure);
}

void	ccdcommand::cancel(CcdWrapper& ccd,
		const std::vector<std::string>& /* arguments */) {
	if (!((Astro::EXPOSURE_EXPOSING == ccd->exposureStatus()) ||
		(Astro::EXPOSURE_EXPOSED == ccd->exposureStatus()))) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "not exposing/exposed");
		return;
	}
	ccd->cancelExposure();
}

void	ccdcommand::wait(CcdWrapper& ccd,
		const std::vector<std::string>& /* arguments */) {
	// wait for completion of this exposure
	while (Astro::EXPOSURE_EXPOSING == ccd->exposureStatus()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"waiting for exposure to complete");
		usleep(1000000);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "exposure complete");
}

void	ccdcommand::image(CcdWrapper& ccd,
		const std::vector<std::string>& arguments) {
	if (arguments.size() < 3) {
		throw std::runtime_error("missing imageid argument");
	}
	std::string	imageid = arguments[2];
	if (Astro::EXPOSURE_EXPOSED != ccd->exposureStatus()) {
		debug(LOG_ERR, DEBUG_LOG, 0, "ccd is not in the exposed state");
		throw std::runtime_error("ccd has no image ready");
	}
	Astro::Image_ptr	image = ccd->getImage();

	// assign the image retrieved
	Images	images;
	images.assign(imageid, image);
}

void	ccdcommand::release(const std::string& ccdid,
		const std::vector<std::string>& /* arguments */) {
	Ccds	ccds;
	ccds.release(ccdid);
}

void	ccdcommand::assign(const std::string& ccdid,
		const std::vector<std::string>& arguments) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "assign %s", ccdid.c_str());
	try {
		Ccds	ccds;
		ccds.assign(ccdid, arguments);
	} catch (std::exception& x) {
		throw command_error(x.what());
	}
}

void	ccdcommand::operator()(const std::string& /* commandname */,
		const std::vector<std::string>& arguments) {
	if (arguments.size() < 2) {
		throw command_error("ccd command requires 2 arguments");
	}
	std::string	ccdid = arguments[0];
	std::string	subcommandname = arguments[1];
	debug(LOG_DEBUG, DEBUG_LOG, 0, "ccd command for CCD %s, subcommand %s",
		ccdid.c_str(), subcommandname.c_str());
	if (subcommandname == "release") {
		release(ccdid, arguments);
		return;
	}
	if (subcommandname == "assign") {
		assign(ccdid, arguments);
		return;
	}

	// get the ccd to perform an operation on it
	Ccds	ccds;
	CcdWrapper	ccd = ccds.byname(ccdid);

	// commands that need a ccd to operate on
	if (subcommandname == "info") {
		info(ccd, arguments);
		return;
	}
	if (subcommandname == "start") {
		start(ccd, arguments);
		return;
	}
	if (subcommandname == "cancel") {
		cancel(ccd, arguments);
		return;
	}
	if (subcommandname == "wait") {
		wait(ccd, arguments);
		return;
	}
	if (subcommandname == "image") {
		image(ccd, arguments);
		return;
	}

	throw command_error("unknown command");
}

std::string	ccdcommand::summary() const {
	return std::string("access ccds");
}

std::string	ccdcommand::help() const {
	return std::string(
	"SYNOPSIS\n"
	"\n"
	"\tccd <ccdid> assign <cameraid> <ccdnumber>\n"
	"\tccd <ccdid> info\n"
	"\tccd <ccdid> start ...\n"
	"\tccd <ccdid> cancel\n"
	"\tccd <ccdid> wait\n"
	"\tccd <ccdid> image <imageid>\n"
	"\tccd <ccdid> release\n"
	"\n"
	"DESCRIPTION\n"
	"\n"
	"The ccd command gives access to the CCDs of a camera. The CCDs\n"
	"are numbered from 0 to the number of CCDs - 1. The first synopsis\n"
	"assigns a short name <ccdi> to a ccd. Use the camera command to\n"
	"assign a camera id to a camera.\n"
	"The second synopsis gives info about a CCD.\n"
	"The third synopsis releases a ccd reference, it should no longer be\n"
	"used after this command is issued.\n"
	"The image subcommand retrieves an image from the ccd and makes it\n"
	"available to the image command under the image id specified.\n"
	);
}

} // namespace cli
} // namespace astro
