/*
 * guiderfactorycommand.c -- guider factory implementation
 *
 * (c) 2103 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <guiderfactorycommand.h>
#include <AstroDebug.h>

namespace astro {
namespace cli {

void	guiderfactorycommand::assign(const std::string& guiderid,
		const std::vector<std::string>& arguments) {
	Guiders	guiders;
	guiders.assign(guiderid, arguments);
}

void	guiderfactorycommand::release(const std::string& guiderid,
		const std::vector<std::string>& /* arguments * */) {
	Guiders	guiders;
	guiders.release(guiderid);
}

void	guiderfactorycommand::operator()(const std::string& /* command */,
		const std::vector<std::string>& arguments) {
	if (arguments.size() < 2) {
		throw command_error("guiderfactory command requires more "
			"arguments");
	}
	std::string	guiderid = arguments[0];
	std::string	subcommand = arguments[1];
	debug(LOG_DEBUG, DEBUG_LOG, 0, "guiderid: %s", guiderid.c_str());

	if (subcommand == "release") {
		release(guiderid, arguments);
		return;
	}

	if (subcommand == "assign") {
		assign(guiderid, arguments);
		return;
	}
}

std::string	guiderfactorycommand::summary() const {
	return std::string("create and retrieve guiders");
}

std::string	guiderfactorycommand::help() const {
	return std::string(
	"SYNOPSIS\n"
	"\n"
	"\tguiderfactory <guider> assign <cameraname> <ccd-number> <guiderportname>\n"
	"\tguiderfactory <guider> release\n"
	"\n"
	"DESCRIPTION\n"
	"\n"
	"The guiderfactory command builds guiders from cameras, ccd numbers\n"
	"and the guiderport\n"
	);
}

} // namespace cli
} // namespace astro
