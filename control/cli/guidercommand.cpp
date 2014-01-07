/*
 * guidercommand.cpp -- guider command implementation
 *
 * (c) 2103 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <guidercommand.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <AstroUtils.h>
//#include <guiders.h>
#include <iostream>
#include <Output.h>
#include <includes.h>
#include <iomanip>
#include <Images.h>

namespace astro {
namespace cli {

/**
 * \brief Display exposure structure
 */
std::ostream&	operator<<(std::ostream& out, const Astro::Exposure& exposure) {
	out << "exposure time:   " << exposure.exposuretime << std::endl;
	out << "rectangle:       " << exposure.frame << std::endl;
	out << "gain:            " << exposure.gain << std::endl;
	out << "limit:           " << exposure.limit << std::endl;
	out << "shutter:         "
		<< ((exposure.shutter == Astro::SHUTTER_CLOSED)
			? "close" : "open")
		<< std::endl;
	out << "binning mode:    " << exposure.mode << std::endl;
	return out;
}

/**
 * \brief Display a point.
 */
std::ostream&	operator<<(std::ostream& out, const Astro::Point& star) {
	out << "(";
	out <<  std::fixed << std::setprecision(2) << star.x;
	out << ",";
	out <<  std::fixed << std::setprecision(2) << star.y;
	out << ")";
	return out;
}

/**
 * \brief Display Calibration data.
 */
std::ostream&	operator<<(std::ostream& out,
			const Astro::Guider::Calibration& calibration) {
	out << "calibration:     ";
	out << stringprintf("[ %10.6f, %10.6f, %10.6f;",
		calibration.coefficients[0],
		calibration.coefficients[1],
		calibration.coefficients[2]);
	out << std::endl;
	out << "           :     ";
	out << stringprintf("  %10.6f, %10.6f, %10.6f   ]",
		calibration.coefficients[3],
		calibration.coefficients[4],
		calibration.coefficients[5]);
	out << std::endl;
	return out;
}

std::ostream&	operator<<(std::ostream& out,
			const Astro::TrackingInfo& trackinginfo) {
	out << "last action at:  ";
	double	when = Timer::gettime() - trackinginfo.timeago;
	time_t	t = floor(when);
	when = when - t;
	char	buffer[20];
	struct tm	*tp = localtime(&t);
	strftime(buffer, sizeof(buffer), "%H:%M:%S.", tp);
	out << buffer;
	t = 1000 * when;
	snprintf(buffer, sizeof(buffer), "%03ld", t);
	out << buffer << std::endl;
	out << "last offset:     " << trackinginfo.trackingoffset << std::endl;
	out << "last activation: " << trackinginfo.activation << std::endl;
	return out;
}

/**
 * \brief Display Guider information and status.
 */
std::ostream&	operator<<(std::ostream& out, GuiderWrapper& guider) {
	out << guider->getExposure();
	out << "point:           " << guider->getStar() << std::endl;

	// display calibration data
	Astro::Guider::GuiderState	state = guider->getState();
	switch (state) {
	case Astro::Guider::GUIDER_CALIBRATING:
		out << "cal progress:    ";
		out << std::fixed << std::setprecision(1);
		out << (100 * guider->calibrationProgress()) << "%";
		out << std::endl;
		break;
	case Astro::Guider::GUIDER_GUIDING:
		out << "guiding:         " << std::endl;
		out << guider->mostRecentTrackingInfo();
	case Astro::Guider::GUIDER_CALIBRATED:
		out << guider->getCalibration();
		break;
	default:
		out << "not calibrated" << std::endl;
		break;
	}
	return out;
}

/**
 * \brief Display exposure command
 *
 * This command displays the current exposure setting of the guider
 */
void	guidercommand::exposure(GuiderWrapper& guider,
		const std::vector<std::string>& arguments) {
	std::cout << guider->getExposure();
}

/**
 * \brief Display info command
 *
 * THis command displays complete information about the guider
 */
void	guidercommand::info(GuiderWrapper& guider,
		const std::vector<std::string>& arguments) {
	std::cout << guider;
}

/**
 * \brief Set exposure time command
 *
 * This command allows to change the exposure time
 */
void	guidercommand::exposuretime(GuiderWrapper& guider,
		const std::vector<std::string>& arguments) {
	if (arguments.size() < 3) {
		throw command_error("exposure time missing");
	}
	Astro::Exposure	exposure = guider->getExposure();
	exposure.exposuretime = stof(arguments[2]);
	guider->setExposure(exposure);
}

/**
 * \brief Binning command
 *
 * This command allows to change the binning mode of the guiding exposure.
 * This is only reasonable if the guide camera has rather small pixels.
 */
void	guidercommand::binning(GuiderWrapper& guider,
		const std::vector<std::string>& arguments) {
	if (arguments.size() < 4) {
		throw command_error("binning mode missing");
	}
	Astro::Exposure	exposure = guider->getExposure();
	exposure.mode.x = stoi(arguments[2]);
	exposure.mode.y = stoi(arguments[3]);
	guider->setExposure(exposure);
}

/**
 * \brief offset command
 *
 * This command sets the origin of the rectangle to use to track the guide
 * star.
 */
void	guidercommand::offset(GuiderWrapper& guider,
		const std::vector<std::string>& arguments) {
	if (arguments.size() < 4) {
		throw command_error("viewport window missing");
	}
	Astro::Exposure	exposure = guider->getExposure();
	exposure.frame.origin.x = stoi(arguments[2]);
	exposure.frame.origin.y = stoi(arguments[3]);
	guider->setExposure(exposure);
}

/**
 * \brief size command
 *
 * This command sets the size of the rectangle to track the guide star
 */
void	guidercommand::size(GuiderWrapper& guider,
		const std::vector<std::string>& arguments) {
	if (arguments.size() < 4) {
		throw command_error("viewport window missing");
	}
	Astro::Exposure	exposure = guider->getExposure();
	exposure.frame.size.width = stoi(arguments[2]);
	exposure.frame.size.height = stoi(arguments[3]);
	guider->setExposure(exposure);
}

/**
 * \brief star command
 *
 * This command sets the star point. The star point is used as a reference
 * during calibration, the calibration computes offsets from this point.
 * The guiding process then guides the telescope in such a way that the
 * guide star is at this position as precisely as possible.
 */
void	guidercommand::star(GuiderWrapper& guider,
		const std::vector<std::string>& arguments) {
	if (arguments.size() < 4) {
		throw command_error("star coordinates missing");
	}
	Astro::Point	point;
	point.x = stoi(arguments[2]);
	point.y = stoi(arguments[3]);
	guider->setStar(point);
}

/**
 * \brief calibration command
 *
 * This command can be used to force a calibration, without actually going
 * through the calibration process on the server side. This allows to
 * reuse calibration settings when the next guide process is based on a
 * guide star close to the one previously used.
 */
void	guidercommand::calibration(GuiderWrapper& guider,
		const std::vector<std::string>& arguments) {
	if (arguments.size() < 8) {
		throw command_error("calibration command requires 6 arguments");
	}
	std::vector<std::string>::const_iterator	i = arguments.begin();
	i += 2;
	int	j = 0;
	Astro::Guider::Calibration	cal;
	while (j < 6) {
		cal.coefficients[j++] = stod(*i++);
	}
	guider->useCalibration(cal);
}

/**
 * \brief start command
 *
 * This command initiates a calibration or guiding process. The command
 * takes an optional argument for the focallength or the gain
 */
void	guidercommand::start(GuiderWrapper& guider,
		const std::vector<std::string>& arguments) {
	if (arguments.size() < 3) {
		throw command_error("start command requires 2 arguments");
	}
	std::string	what = arguments[2];
	if (what == "calibration") {
		float	focallength = 0.600;
		if (arguments.size() >= 4) {
			focallength = stod(arguments[3]);
			debug(LOG_DEBUG, DEBUG_LOG, 0, "using focal length %f",
				focallength);
		}
		guider->startCalibration(focallength);
		return;
	}
	if (what == "guiding") {
		if (guider->getState() == Astro::Guider::GUIDER_GUIDING) {
			std::cout << "already guiding, ignored" << std::endl;
			return;
		}
		double	interval = 10;
		if (arguments.size() >= 4) {
			interval = stod(arguments[3]);
		}
		interval = std::max(1., interval);
		guider->startGuiding(interval);
		return;
	}
}

/**
 * \brief stop command
 *
 * The stop command can be used to stop a calibraiton or guiding process.
 */
void	guidercommand::stop(GuiderWrapper& guider,
		const std::vector<std::string>& arguments) {
	if (arguments.size() < 3) {
		throw command_error("start command requires 2 arguments");
	}
	std::string	what = arguments[2];
	if (what == "calibration") {
		if (guider->getState() != Astro::Guider::GUIDER_CALIBRATING) {
			std::cout << "not currently calibrating" << std::endl;
			return;
		}
		guider->cancelCalibration();
		return;
	}
	if (what == "guiding") {
		if (guider->getState() != Astro::Guider::GUIDER_GUIDING) {
			std::cout << "not currently guiding" << std::endl;
			return;
		}
		guider->stopGuiding();
		return;
	}
}

/**
 * \brief wait command
 *
 * This command waits for the completion of the calibration process on the
 * server. It does not use the waitCalibration method, because this would
 * block a thread on the server side.
 */
void	guidercommand::wait(GuiderWrapper& guider,
		const std::vector<std::string>& arguments) {
	if (arguments.size() < 3) {
		throw command_error("stop command requires 2 arguments");
	}
	std::string	what = arguments[2];
	double	timeout = 60;
	if (arguments.size() >= 4) {
		timeout = stod(arguments[3]);
	}
	double	timeouttime = astro::Timer::gettime() + timeout;

	if (what == "calibration") {
		while ((guider->getState() == Astro::Guider::GUIDER_CALIBRATING)
			&& (astro::Timer::gettime() < timeouttime)) {
			usleep(1000000);
		}
		switch (guider->getState()) {
		case Astro::Guider::GUIDER_CALIBRATED:
			std::cout << "calibration complete" << std::endl;
			break;
		default:
			std::cout << "calibration failed" << std::endl;
			break;
		}
		return;
	}
	if (what == "guiding") {
		while ((guider->getState() == Astro::Guider::GUIDER_GUIDING)
			&& (astro::Timer::gettime() < timeouttime)) {
			usleep(1000000);
		}
		if (guider->getState() == Astro::Guider::GUIDER_GUIDING) {
			std::cout << "still guiding" << std::endl;
		}
		return;
	}
}

/**
 * \brief retrieve the most recent image from the guider
 */
void	guidercommand::image(GuiderWrapper& guider,
		const std::vector<std::string>& arguments) {
	if (arguments.size() < 3) {
		throw std::runtime_error("missing imageid argument");
	}
	std::string	imageid = arguments[2];

	Astro::Image_ptr	image = guider->mostRecentImage();
        Images  images;
        images.assign(imageid, image);
}


/**
 * \brief Main command interpreter function
 *
 * This operator analyzes the command arguments and calls the appropriate
 * subcommand method.
 */
void	guidercommand::operator()(const std::string& command,
		const std::vector<std::string>& arguments) {
	if (arguments.size() < 2) {
		throw command_error("guider command requires more "
			"arguments");
	}
	std::string	guiderid = arguments[0];
	std::string	subcommand = arguments[1];
	debug(LOG_DEBUG, DEBUG_LOG, 0, "guiderid: %s", guiderid.c_str());

	Guiders	guiders;
	GuiderWrapper	guider = guiders.byname(guiderid);

	if (subcommand == "info") {
		info(guider, arguments);
		return;
	}

	if (subcommand == "exposure") {
		exposure(guider, arguments);
		return;
	}

	if (subcommand == "exposuretime") {
		exposuretime(guider, arguments);
		return;
	}

	if (subcommand == "binning") {
		binning(guider, arguments);
		return;
	}

	if (subcommand == "size") {
		size(guider, arguments);
		return;
	}

	if (subcommand == "offset") {
		offset(guider, arguments);
		return;
	}

	if (subcommand == "star") {
		star(guider, arguments);
		return;
	}

	if (subcommand == "calibration") {
		calibration(guider, arguments);
		return;
	}

	if (subcommand == "start") {
		start(guider, arguments);
		return;
	}

	if (subcommand == "stop") {
		stop(guider, arguments);
		return;
	}

	if (subcommand == "wait") {
		wait(guider, arguments);
		return;
	}

	if (subcommand == "image") {
		image(guider, arguments);
		return;
	}
}

/**
 * \brief Summary of guider command
 */
std::string	guidercommand::summary() const {
	return std::string("create and retrieve guiders");
}

/**
 * \brief Help on guider command
 */
std::string	guidercommand::help() const {
	return std::string(
	"SYNOPSIS\n"
	"\n"
	"\tguider <guider> exposure\n"
	"\tguider <guider> info\n"
	"\tguider <guider> exposuretime <time>\n"
	"\tguider <guider> binning <bin_x> <bin_y>\n"
	"\tguider <guider> size <x> <y>\n"
	"\tguider <guider> offset <width> <height>\n"
	"\tguider <guider> star <x> <y>\n"
	"\tguider <guider> calibration <a0> <a1> <a2> <a3> <a4> <a5>\n"
	"\tguider <gudier> start { calibration | guiding } [ args ... ]\n"
	"\tguider <gudier> stop { calibration | guiding }\n"
	"\tguider <guider> wait { calibration | guiding } [ <timeout> ]\n"
	"\tguider <guider> image <name>\n"
	"\n"
	"DESCRIPTION\n"
	"\n"
	"The subcommands \"exposure\" and \"info\" display information about\n"
	"the current guider configuration\n"
	"\n"
	"The \"exposuretime\", \"binning\", \"size\" and \"offset\" commands\n"
	"control the exposure taken using the camera\n"
	"\n"
	"The \"star\" command sets the star coordinates to track.\n"
	"\n"
	"The \"calibration\" command sets calibration settings without the\n"
	"need for a calibration process. This can save time when using a\n"
	"guide star close to one previously calibrated.\n"
	"\n"
	"The \"start\" command starts a new calibration or guiding process.\n"
	"If the calibration is successful, it sets the new calibration. The\n"
	"guiding process does not terminate until it is cancelled with the\n"
	"stop command\n"
	"\n"
	"The \"stop\" command can be used to stop a calibration or guiding\n"
	"process. A wait command should be used to wait until the process\n"
	"has terminated.\n"
	"\n"
	"The \"wait\" command waits for the calibration to complete, the\n"
	"timeout argument is in seconds.\n"
	"\n"
	"The \"image\" command makes the most recent image available via\n"
	"image commands\n"
	);
}

} // namespace cli
} // namespace astro
