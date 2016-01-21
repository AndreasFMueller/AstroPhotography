/*
 * generalcmd.cpp -- general commands
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "guide.h"
#include <cstdlib>
#include <iostream>
#include <signal.h>
#include <IceConversions.h>
#include <CommunicatorSingleton.h>
#include <CommonClientTasks.h>
#include <ImageCallbackI.h>

namespace snowstar {
namespace app {
namespace snowguide {

/**
 * \brief usage method
 */
void	Guide::usage(const char *progname) {
	astro::Path	path(progname);
	std::string	p = std::string("    ") + path.basename();
	std::cout << "The snowguide program takes the CCD and guiderport defined for for an" << std::endl;
	std::cout << "and builds a guider from them. It understands a number of subcommands" << std::endl;
	std::cout << "to control guding via this guider. A GuiderCCD and GuiderPort must be" << std::endl;
	std::cout << "defined in the instrument, as well as the guiderfocallength property." << std::endl;
	std::cout << std::endl;
	std::cout << "Usage:" << std::endl;
	std::cout << std::endl;
	std::cout << p << " [ options ] help" << std::endl;
	std::cout << p << " [ options ] <service> <INSTRUMENT> help"
		<< std::endl;
	std::cout << p << " [ options ] <service> <INSTRUMENT> state"
		<< std::endl;
	std::cout << p << " [ options ] <service> <INSTRUMENT> repository [ <repo> ]"
		<< std::endl;

	std::cout << std::endl;
	std::cout << "  Calibration:" << std::endl;
	std::cout << std::endl;

	std::cout << p << " [ options ] <service> <INSTRUMENT> calibrate [ <id> | <type> ]"
		<< std::endl;
	std::cout << p << " [ options ] <service> <INSTRUMENT> calibration"
		<< std::endl;
	std::cout << p << " [ options ] <service> <INSTRUMENT> cancel"
		<< std::endl;
	std::cout << p << " [ options ] <service> <INSTRUMENT> list"
		<< std::endl;
	std::cout << p << " [ options ] <service> <INSTRUMENT> trash <calid>"
		<< std::endl;

	std::cout << std::endl;
	std::cout << "  Guiding:" << std::endl;
	std::cout << std::endl;

	std::cout << p << " [ options ] <service> <INSTRUMENT> guide"
		<< std::endl;
	std::cout << p << " [ options ] <service> <INSTRUMENT> stop"
		<< std::endl;
	std::cout << p << " [ options ] <service> <INSTRUMENT> tracks"
		<< std::endl;
	std::cout << p << " [ options ] <service> <INSTRUMENT> history"
		" [ trackid ]" << std::endl;
	std::cout << p << " [ options ] <service> <INSTRUMENT> forget <trackid> ...";
	std::cout << std::endl;

	std::cout << std::endl;
	std::cout << "  Monitoring:" << std::endl;
	std::cout << std::endl;

	std::cout << p << " [ options ] <service> <INSTRUMENT> monitor"
		<< std::endl;
	std::cout << p << " [ options ] <service> <INSTRUMENT> images <path>"
		<< std::endl;

	std::cout << std::endl;

	std::cout << "Options:" << std::endl;
	std::cout << std::endl;
	std::cout << "  -b,--binning=XxY      select XxY binning mode (default "
		"1x1)" << std::endl;
	std::cout << "  -c,--config=<cfg>     use configuration from file <cfg>";
	std::cout << std::endl;
	std::cout << "  -d,--debug            increase debug level" << std::endl;
	std::cout << "  -e,--exposure=<e>     set exposure time to <e>";
	std::cout << std::endl;
	std::cout << "  -h,--help             display this help message and exit";
	std::cout << std::endl;
	std::cout << "  -i,--interval=<i>     perform an update ever i seconds when guiding";
	std::cout << std::endl;
	std::cout << "  -m,--method=<m>       use tracking method <m>. Available methods are 'star'" << std::endl;
	std::cout << "                        (centroid of a star), 'phase' (uses cross correlation" << std::endl;
	std::cout << "                        to find image offsets), 'diff' (uses cross correlation" << std::endl;
	std::cout << "                        on edges in the image to find image offsets)" << std::endl;
	std::cout << "  -r,--rectangle=<rec>  expose only a subrectangle as "
		"specified by <rec>." << std::endl;
	std::cout << "                        <rec> must be of the form";
	std::cout << std::endl;
	std::cout << "                        widthxheight@(xoffset,yoffset)";
	std::cout << std::endl;
	std::cout << "                        if -s and -w are specified, the "
		"subrectangle is";
	std::cout << std::endl;
	std::cout << "                        computed from these." << std::endl;
	std::cout << "  -s,--star=<pos>       position of the star to calibrate "
		"or guide on in the" << std::endl;
	std::cout << "                        syntax (x,y), the parentheses are "
		"optional" << std::endl;
	std::cout << "  -t,--temperature=<t>  cool ccd to temperature <t>, "
		"ignored if the instrument" << std::endl;
	std::cout << "                        has no cooler" << std::endl;
	std::cout << "  -v,--verbose          enable verbose mode" << std::endl;
	std::cout << "  -w,--width=<w>        set the width and height of the area to expose" << std::endl;
	std::cout << std::endl;
}

/**
 * \brief Help command implementation
 */
int	Guide::help_command(const char *progname) {
	usage(progname);
	std::cout <<
"help" << std::endl << 
"    display this help message and exit" << std::endl

<< std::endl << 
"state" << std::endl <<
"    display the current state of the guider. The states are idle (not doing" << std::endl <<
"    anything), calibrating (trying to determine the calibration), calibrated"<< std::endl <<
"    (ready to start guiding), guiding (control the guider port to keep a" << std::endl <<
"    star in the same position on the guider CCD)." << std::endl

<< std::endl << 
"calibrate [ <calibrationid> | <calibrationtype> ]" << std::endl <<
"    use <calibrationid> to calibrate the guider, if <calibrationid> is"
<< std::endl <<
"    is specified. Without an argument, start a new calibration run for the"
<< std::endl <<
"    guider port control device. The <calibrationtype> specifies the control"
<< std::endl <<
"    device to calibrate, it can be 'GP' for guider port or 'AO' for adaptive"
<< std::endl <<
"    optics. If no argument is given, 'GP' is assumed. Depending on the "
<< std::endl <<
"   tracker method, The --star argument may be required for this function."
<< std::endl

<< std::endl << 
"calibration [ <calibrationid> ]" << std::endl <<
"    display the current calibration or the calibration with id "
<< std::endl <<
"    <calibrationid> if specified." << std::endl

<< std::endl << 
"list" << std::endl <<
"    display a short list of all calibrations available in the database"
<< std::endl

<< std::endl << 
"trash <calibrationid> ..." << std::endl <<
"    remove the specified calibration data records from the database"
<< std::endl

<< std::endl <<
"cancel" << std::endl <<
"    cancel the currently active calibration run." << std::endl

<< std::endl << 
"guide" << std::endl <<
"    Start guiding with the current calibration id. The --star option is"
<< std::endl <<
"    required." << std::endl 

<< std::endl << 
"stop" << std::endl <<
"    stop the guiding process" << std::endl

<< std::endl << 
"tracks"  << std::endl <<
"    list all guiding tracks recorded in the database" << std::endl

<< std::endl << 
"history" << std::endl <<
"    Display the tracking history of the current guiding run." << std::endl

<< std::endl << 
"monitor" << std::endl <<
"    Monitor the guiding or calibration process. This subcommand reports all"
<< std::endl <<
"    state changes and all commands sent to the guider port." << std::endl

<< std::endl << 
"images <directory>" << std::endl <<
"    operate as an image callback, and store all images sent from the server"
<< std::endl <<
"    in the directory named <directory>." << std::endl

<< std::endl;
	return EXIT_SUCCESS;
}

/**
 * \brief Get the state of a guider
 *
 * This command retrieves the current state of the guider
 */
int	Guide::state_command(GuiderPrx guider) {
	GuiderState	state = guider->getState();
	std::cout << guiderstate2string(state);
	switch (state) {
	case GuiderCALIBRATING:
		std::cout << ": " << guider->calibrationProgress();
		break;
	case GuiderCALIBRATED:
		std::cout << ": ";
		try {
			Calibration	cal = guider->getCalibration(
						CalibrationTypeGuiderPort);
			std::cout << "GP=" << cal.id;
		} catch (...) { }
		try {
			Calibration	cal = guider->getCalibration(
						CalibrationTypeAdaptiveOptics);
			std::cout << "AO=" << cal.id;
		} catch (...) { }
		break;
	case GuiderGUIDING: {
		std::cout << ": ";
		TrackingSummary	summary = guider->getTrackingSummary();
		std::cout << astro::stringprintf("%d duration=%.0f, ",
			summary.guiderunid, summary.since);
		std::cout << astro::stringprintf("last=(%.2f,%.2f), ",
			summary.lastoffset.x,
			summary.lastoffset.y);
		std::cout << astro::stringprintf("avg=(%.2f,%.2f), ",
			summary.averageoffset.x,
			summary.averageoffset.y);
		std::cout << astro::stringprintf("var=(%.2f,%.2f)",
			sqrt(summary.variance.x),
			sqrt(summary.variance.y));
		}
		break;
	default:
		break;
	}
	std::cout << std::endl;
	return EXIT_SUCCESS;
}

/**
 * \brief get the repository name from the guider
 */
int	Guide::repository_command(GuiderPrx guider) {
	std::string	reponame = guider->getRepositoryName();
	if (0 == reponame.size()) {
		std::cout << "repository name not set" << std::endl;
	} else {
		std::cout << reponame << std::endl;
	}
	return EXIT_SUCCESS;
}

/**
 * \brief set the remote repository name
 */
int	Guide::repository_command(GuiderPrx guider,
		const std::string& repositoryname) {
	guider->setRepositoryName(repositoryname);
	return EXIT_SUCCESS;
}

} // namespace snowguide
} // namespace app
} // namespace snowstar
