/*
 * FocusCallback.cpp -- 
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <stdexcept>
#include <iostream>
#include <focusing.h>
#include <AstroUtils.h>
#include <AstroConfig.h>
#include <AstroDebug.h>
#include <IceConversions.h>
#include <AstroFormat.h>
#include <FocusCallbackI.h>
#include <includes.h>

using namespace astro::config;
using namespace astro;
using namespace snowstar;

namespace snowstar {
namespace app {
namespace snowfocus {

void	FocusCallbackI::raw_prefix(const std::string raw_prefix) {
	_raw_prefix = raw_prefix;
}

void	FocusCallbackI::evaluated_prefix(const std::string& evaluated_prefix) {
	_evaluated_prefix = evaluated_prefix;
}

/**
 * \brief Create a callback object
 */
FocusCallbackI::FocusCallbackI() {
}

/**
 * \brief add a point
 */
void	FocusCallbackI::addPoint(const FocusPoint& point,
		const Ice::Current& /* current */) {
	std::cout << timeformat("%H:%M:%S ", time(NULL));
	std::cout << point.position << ": " << point.value;
	std::cout << std::endl;
}

/**
 * \brief change the state
 */
void	FocusCallbackI::changeState(FocusState state,
		const Ice::Current& /* current */) {
	std::cout << timeformat("%H:%M:%S ", time(NULL));
	std::cout << "new state: ";
	std::cout << focusingstate2string(state);
	std::cout << std::endl;
}

static int focus_counter = 0;

/**
 * \brief write the focus element
 */
void	FocusCallbackI::addFocusElement(const FocusElement& element,
		const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "raw size=%d, evaluated size=%d",
		element.raw.data.size(), element.evaluated.data.size());

{
	int	fd = open(stringprintf("d-%d.png", focus_counter++).c_str(),
			O_CREAT | O_TRUNC | O_WRONLY);
	::write(fd, element.raw.data.data(), element.raw.data.size());
	close(fd);
}
	std::cout << timeformat("%H:%M:%S ", time(NULL));
	astro::focusing::FocusElementPtr	fe = convert(element);

	std::cout << "raw: " << fe->raw_image->info();
	std::cout << ", ";
	std::cout << "evaluated: " << fe->processed_image->info();

	if (_raw_prefix.size() > 0) {
		std::string	filename = stringprintf("%s-%d.jpg",
			_raw_prefix.c_str(), fe->pos());
		try {
			FITS	fits;
			fits.writeFITS(fe->raw_image, stringprintf("%s-%d.fits",
				_raw_prefix.c_str(), fe->pos()));
			JPEG	jpeg;
			jpeg.writeJPEG(fe->raw_image, filename);
		} catch (const std::exception& x) {
			debug(LOG_ERR, DEBUG_LOG, 0, "cannot write %s: %s",
				filename.c_str(), x.what());
		}
	}
	
	if (_evaluated_prefix.size() > 0) {
		std::string	filename = stringprintf("%s-%d.png",
			_evaluated_prefix.c_str(), fe->pos());
		try {
			PNG	png;
			png.writePNG(fe->processed_image, filename);
		} catch (const std::exception& x) {
			debug(LOG_ERR, DEBUG_LOG, 0, "cannot write %s: %s",
				filename.c_str(), x.what());
		}
	}
	
	std::cout << std::endl;
}


} // namespace snowfocus
} // namespace app
} // namespace snowstar
