/*
 * ParseFileimageStep.cpp
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroProcess.h>
#include "ProcessorParser.h"

namespace astro {
namespace process {

/**
 * \brief Create a new File image node
 *
 * \param attrs		XML attributes specified in the start element
 */
void	ProcessorParser::startFileimage(const attr_t& attrs) {
	// get the file name
	attr_t::const_iterator	i = attrs.find(std::string("file"));
	if (i == attrs.end()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no file name");
		throw std::runtime_error("no file name");
	}
	std::string	filename = i->second;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "filename: %s", filename.c_str());
	FileImageStep	*filestep = new FileImageStep(filename);
	ProcessingStepPtr	step(filestep);

	// push the process on the stack
	_stepstack.push(step);

	startCommon(attrs);

	if (filename.size() > 0) {
		if (filename[0] != '/') {
			std::string	f = fullname(filename);
			debug(LOG_DEBUG, DEBUG_LOG, 0, "new filename: %s",
				f.c_str());
			filestep->filename(f);
		}
	}
}

} // namespace process
} // namespace astro
