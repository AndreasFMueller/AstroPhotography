/*
 * ProcessorFactory.cpp
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroProcess.h>
#include <AstroDebug.h>
#include "ProcessorParser.h"

namespace astro {
namespace process {

/**
 * \brief Construct a new ProcessorFactory
 */
ProcessorFactory::ProcessorFactory() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "processor factory created");
}

/**
 * \brief Build a new processor network
 */
ProcessorNetworkPtr	ProcessorFactory::operator()() {
	ProcessorNetworkPtr	result(new ProcessorNetwork());
	return result;
}

/**
 * \brief Construct a processor network from an XML file
 */
ProcessorNetworkPtr	ProcessorFactory::operator()(
				const std::string& filename) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start parsing file %s",
		filename.c_str());
	ProcessorParser	parser;
	return parser.parse(filename);
}

ProcessorNetworkPtr	ProcessorFactory::operator()(const char *data, int size) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start %d bytes of data @ %p",
		size, data);
	ProcessorParser	parser;
	return parser.parse(data, size);
}

} // namespace process
} // namespace astro
