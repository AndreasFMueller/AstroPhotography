/*
 * coolercommand.h -- cooler command implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <coolercommand.h>
#include <Ccds.h>
#include <iostream>
#include <unistd.h>

namespace astro {
namespace cli {

CoolerWrapper	coolercommand::getCooler(const std::string& ccdid) {
	Ccds	ccds;
	CcdWrapper	ccd = ccds.byname(ccdid);
	if (!ccd->hasCooler()) {
		debug(LOG_ERR, DEBUG_LOG, 0, "ccd %s has no cooler",
			ccd->getName());
		throw command_error("no cooler present");
	}
	Astro::Cooler_ptr	cooler = ccd->getCooler();
	return CoolerWrapper(cooler);
}

std::ostream&	operator<<(std::ostream& out, CoolerWrapper& cooler) {
	out << "actual temperature: "
		<< cooler->getActualTemperature() << std::endl;
	out << "set temperature:    "
		<< cooler->getSetTemperature() << std::endl;
	out << "enabled:            "
		<< ((cooler->isOn()) ? "YES" : "NO") << std::endl;
	return out;
}

void	coolercommand::status(CoolerWrapper& cooler,
		const std::vector<std::string>& /* arguments */) {
	std::cout << cooler;
}

void	coolercommand::set(CoolerWrapper& cooler,
		const std::vector<std::string>& arguments) {
	if (arguments.size() < 3) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"set requires additional temperature argument");
		throw command_error("missing temperature argument");
	}
	std::string	temperaturestring = arguments[2];
	float	temperature = stof(temperaturestring);
	cooler->setTemperature(temperature);
}

void	coolercommand::enable(CoolerWrapper& cooler,
		const std::vector<std::string>& /* arguments */) {
	cooler->setOn(true);
}

void	coolercommand::disable(CoolerWrapper& cooler,
		const std::vector<std::string>& /* arguments */) {
	cooler->setOn(false);
}

void	coolercommand::waitfor(CoolerWrapper& cooler,
		const std::vector<std::string>& arguments) {
	float	settemperature = cooler->getSetTemperature();
	float	target = 0;
	if (arguments.size() > 2) {
		std::string	temperaturestring = arguments[2];
		float requestedtemperature = stof(temperaturestring);
		if (settemperature > requestedtemperature) {
			throw command_error("requested temperature too low");
		}
		target = requestedtemperature;
	} else {
		target = settemperature + 0.5;
	}
	float	actual;
	while ((actual = cooler->getActualTemperature()) > target) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"temperature: target %.2f, actual %.2f",
			actual, target);
		usleep(1000000);
	}
}

void	coolercommand::operator()(const std::string& /* commandname */,
		const std::vector<std::string>& arguments) {
	if (arguments.size() < 2) {
		throw command_error("cooler command requires 2 arguments");
	}
	// get the cooler
	std::string	ccdid = arguments[0];
	CoolerWrapper	cooler = getCooler(ccdid);

	// the the subcommand
	std::string	subcommandname = arguments[1];
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"cooler command for CCD %s, subcommand %s",
		ccdid.c_str(), subcommandname.c_str());
	if (subcommandname == "set") {
		set(cooler, arguments);
		return;
	}
	if (subcommandname == "status") {
		status(cooler, arguments);
		return;
	}
	if (subcommandname == "enable") {
		enable(cooler, arguments);
		return;
	}
	if (subcommandname == "disable") {
		disable(cooler, arguments);
		return;
	}
	if (subcommandname == "waitfor") {
		waitfor(cooler, arguments);
		return;
	}
	throw command_error("unknown command");
}

std::string	coolercommand::summary() const {
	return std::string("access coolers");
}

std::string	coolercommand::help() const {
	return std::string(
	"SYNOPSIS\n"
	"\n"
	"\tcooler <ccdid> status\n"
	"\tcooler <ccdid> set <temperature>\n"
	"\tcooler <ccdid> { enable | disable }\n"
	"\tcooler <ccdid> waitfor [ <temperature> ]\n"
	"\n"
	"DESCRIPTION\n"
	"\n"
	"The cooler command controls the thermoelectric cooler of a ccd.\n"
	"The first synopsis displays the current status of the cooler.\n"
	"The second synopsis sets the temperature the cooler should operate\n"
	"at.\n"
	"The third synopsis turns the cooler on or off\n"
	"The fourth synopsis causes the client to wait for the cooler to\n"
	"reach a temperature below the specified temperature, or the set\n"
	"temperature plus 1 degree of no temperature was specified. If the\n"
	"temperature is not reached within 60 seconds, the command is aborted.\n"
	);
}

} // namespace cli
} // namespace astro
