/*
 * ParseWriteFileimageStep.cpp
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroProcess.h>
#include <ProcessorParser.h>

namespace astro {
namespace process {

/**
 * \brief start the writefileimage element
 */
void	ProcessorParser::startWritefileimage(const attr_t& attrs) {
	// we need a file attribute
	attr_t::const_iterator	i = attrs.find(std::string("file"));
	if (i == attrs.end()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "missing file attribute");
		throw std::runtime_error("missing file attribute");
	}
	std::string	filename = i->second;

	// create a new dark process
	WritableFileImageStep	*writable
		= new WritableFileImageStep(nodePaths(), filename);
	ProcessingStepPtr	step(writable);

	// remember the step everywhere
	push(step);

	startCommon(attrs);

	if (filename.size() > 0) {
		writable->filename(filename);
	}
}

} // namespace process
} // namespace astro
