/*
 * submitcommand.cpp -- implementation of submit command
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <submitcommand.h>
#include <AstroDebug.h>
#include <guidecli.h>
#include <AstroTask.h>
#include <algorithm>
#include <tasks.hh>
#include <Conversions.h>
#include <sstream>

namespace astro {
namespace cli {

class TaskParameterParser {
	astro::task::TaskParameters&	_parameters;
public:
	TaskParameterParser(astro::task::TaskParameters& parameters)
		: _parameters(parameters) { }

	const astro::task::TaskParameters&	parameters() const {
		return _parameters;
	}

	void	operator()(const std::string& valuepair);
};

void	TaskParameterParser::operator()(const std::string& valuepair) {
	size_t	pos = valuepair.find('=');
	if (std::string::npos == pos) {
		debug(LOG_ERR, DEBUG_LOG, 0, "not an attribute-value pair: %s",
			valuepair.c_str());
	}
	std::string	attribute = valuepair.substr(0, pos);
	std::string	value = valuepair.substr(pos + 1);
	if ("camera" == attribute) {
		_parameters.camera(value); return;
	}
	if ("ccdid" == attribute) {
		_parameters.ccdid(stoi(value)); return;
	}
	if ("temperature" == attribute) {
		_parameters.ccdtemperature(stod(value)); return;
	}
	if ("filterwheel" == attribute) {
		_parameters.filterwheel(value); return;
	}
	std::istringstream      in(value);
	if ("position" == attribute) {
		int	position;
		in >> position;
		_parameters.filterposition(position);
		return;
	}
	if ("origin" == attribute) {
		astro::image::ImagePoint	origin;
		in >> origin;
		_parameters.exposure().frame.setOrigin(origin);
		return;
	}
	if ("size" == attribute) {
		astro::image::ImageSize size;
		in >> size;
		_parameters.exposure().frame.setSize(size);
		return;
	}
	if ("binning" == attribute) {
		astro::camera::Binning  binning;
		in >> binning;
		_parameters.exposure().mode = binning;
		return;
	}
	if ("exposuretime" == attribute) {
		in >> _parameters.exposure().exposuretime;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "set exposure time: %f",
			_parameters.exposure().exposuretime);
		return;
	}
	if ("gain" == attribute) {
		in >> _parameters.exposure().gain; return;
	}
	if ("limit" == attribute) {
		in >> _parameters.exposure().limit; return;
	}
	if ("shutter" == attribute) {
		if ("closed" == value) {
			_parameters.exposure().shutter
				= astro::camera::Shutter::CLOSED;
			return;
		}
		if ("open" == value) {
			_parameters.exposure().shutter
				= astro::camera::Shutter::OPEN;
			return;
		}
		debug(LOG_ERR, DEBUG_LOG, 0, "bad shutter value: %s",
			value.c_str());
		throw std::runtime_error("bad shutter value");
	}
	debug(LOG_ERR, DEBUG_LOG, 0, "attribute ignored: %s",
		attribute.c_str());
}

void	submitcommand::operator()(const std::string& /* command */,
		const std::vector<std::string>& arguments) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "submit new exposure");
	astro::task::TaskParameters	parameters;
	TaskParameterParser	parser(parameters);
	std::for_each(arguments.begin(), arguments.end(), parser);
	Astro::TaskParameters	taskparameters = convert(parameters);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "exposure time: %f",
		parameters.exposure().exposuretime);
	// get a TaskQueue
	guidesharedcli	gcli;
	long	taskid = gcli->taskqueue->submit(taskparameters);
	std::cout << "task id: " << taskid << std::endl;
}

std::string	submitcommand::summary() const {
	return std::string("submit exposure task");
}

std::string	submitcommand::help() const {
	return std::string(
		"SYNOPSIS\n"
		"\n"
		"\tsubmit <attr=value> ...\n"
		"\n"
		"DESCRIPTION\n"
		"\n"
		"submit a new exposure task. The following attributes\n"
		"are understood:\n"
		"   camera=<camera-name>\n"
		"   ccdid=<ccd-number>\n"
		"   temperature=<ccd-temperature-absolute>\n"
		"   filterwheel=<filterwheel-name>\n"
		"   position=<filterwheel-position>\n"
		"   exposuretime=<exposure-time>\n"
		"   origin=<image-rectangle-origin>\n"
		"   size=<image-rectangle-size>\n"
		"   binning=<image-binning-mode>\n"
		"   shutter=<shutter-mode>\n"
		"   gain=<amplifier-gain>\n"
		"   limit=<limit-value>\n"
	);
}

} // namespace cli
} // namespace astro
