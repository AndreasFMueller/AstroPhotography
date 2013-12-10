/*
 * guidercommand.cpp -- guider command implementation
 *
 * (c) 2103 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <guidercommand.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
//#include <guiders.h>
#include <iostream>
#include <Output.h>

namespace astro {
namespace cli {

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

std::ostream&	operator<<(std::ostream& out, const Astro::Point& star) {
	out << "point:           (" << star.x << "," << star.y << ")" << std::endl;
	return out;
}

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

std::ostream&	operator<<(std::ostream& out, GuiderWrapper& guider) {
	out << guider->getExposure();
	out << guider->getStar();

	// display calibration data
	Astro::Guider::GuiderState	state = guider->getState();
	if ((Astro::Guider::GUIDER_CALIBRATED == state) ||
		(Astro::Guider::GUIDER_GUIDING == state)) {
		out << guider->getCalibration();
	} else {
		out << "not calibrated" << std::endl;
	}
	return out;
}

void	guidercommand::exposure(GuiderWrapper& guider,
		const std::vector<std::string>& arguments) {
	std::cout << guider->getExposure();
}

void	guidercommand::info(GuiderWrapper& guider,
		const std::vector<std::string>& arguments) {
	std::cout << guider;
}

void	guidercommand::exposuretime(GuiderWrapper& guider,
		const std::vector<std::string>& arguments) {
	if (arguments.size() < 3) {
		throw command_error("exposure time missing");
	}
	Astro::Exposure	exposure = guider->getExposure();
	exposure.exposuretime = stof(arguments[2]);
	guider->setExposure(exposure);
}

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

void	guidercommand::calibrate(GuiderWrapper& guider,
		const std::vector<std::string>& arguments) {
	if (arguments.size() < 3) {
		throw command_error("calibrate command requires 2 arguments");
	}
	float	focallength = stod(arguments[2]);
	guider->startCalibration(focallength);
}

void	guidercommand::wait(GuiderWrapper& guider,
		const std::vector<std::string>& arguments) {
	double	timeout = 60;
	if (arguments.size() >= 3) {
		timeout = stod(arguments[2]);
	}
	guider->waitCalibration(timeout);
}

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

	if (subcommand == "calibrate") {
		calibrate(guider, arguments);
		return;
	}

	if (subcommand == "wait") {
		wait(guider, arguments);
	}
}

std::string	guidercommand::summary() const {
	return std::string("create and retrieve guiders");
}

std::string	guidercommand::help() const {
	return std::string(
	"SYNOPSIS\n"
	"\n"
	"\tguider <guider> exposure\n"
	"\tguider <guider> info\n"
	"\tguider <guider> binning <bin_x> <bin_y>\n"
	"\tguider <guider> size <x> <y>\n"
	"\tguider <guider> offset <width> <height>\n"
	"\tguider <guider> star <x> <y>\n"
	"\tguider <guider> calibration <a0> <a1> <a2> <a3> <a4> <a5>\n"
	"\n"
	"DESCRIPTION\n"
	"\n"
	);
}

} // namespace cli
} // namespace astro
