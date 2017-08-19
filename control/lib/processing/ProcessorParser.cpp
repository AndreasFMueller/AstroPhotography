/*
 * ProcessorParser.cpp
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "ProcessorParser.h"
#include <AstroProcess.h>
#include <AstroDebug.h>
#include <AstroFormat.h>

namespace astro {
namespace process {

/**
 * \brief Construct a parser
 */
ProcessorParser::ProcessorParser() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "parser constructed");
	memset(&_handler, 0, sizeof(_handler));
	_handler.startElement = ::startElement;
	_handler.endElement = ::endElement;
	_handler.characters = ::characters;
	_handler.error = ::error;
	_handler.fatalError = ::fatal;
	_handler.warning = ::warning;
}

/**
 * \brief Handle a start element
 */
void	ProcessorParser::startElement(const std::string& name,
		const std::map<std::string, std::string>& attrs) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found element '%s'", name.c_str());
	if (name == std::string("fileimage")) {
		startFileimage(attrs);
		return;
	}
}

/**
 * \brief Handle an end element
 */
void	ProcessorParser::endElement(const std::string& name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "element '%s' closed", name.c_str());
	if (name == std::string("fileimage")) {
		endFileimage();
		return;
	}
}

/**
 * \brief handle new characters
 */
void	ProcessorParser::characters(const std::string& data) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "data: %s", data.c_str());
}

/**
 * \brief error warning
 */
void	ProcessorParser::warning(const std::string& msg) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "warning: %s", msg.c_str());
}

/**
 * \brief error message
 */
void	ProcessorParser::error(const std::string& msg) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "error: %s", msg.c_str());
}

/**
 * \brief fatal error message
 */
void	ProcessorParser::fatal(const std::string& msg) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "fatal: %s", msg.c_str());
}

/**
 * \brief Parse a file
 */
ProcessorNetworkPtr	ProcessorParser::parse(const std::string& filename) {
	_network = ProcessorNetworkPtr(new ProcessorNetwork());
	// use the libXML function to actuall run the parser over the file
	int	rc = xmlSAXUserParseFile(&_handler, this, filename.c_str());
	if (rc < 0) {
		throw std::runtime_error("parse error");
	}
	return _network;
}

/**
 * \brief Parse the data in a buffer
 */
ProcessorNetworkPtr	ProcessorParser::parse(const char *data, int size) {
	_network = ProcessorNetworkPtr(new ProcessorNetwork());
	// use the libXML function to actuall run the parser over the file
	int	rc = xmlSAXUserParseMemory(&_handler, this, data, size);
	if (rc < 0) {
		throw std::runtime_error("parse error");
	}
	return _network;
}

} // namespace process
} // namespace astro

/**
 * \brief SAX function for handling start elements
 */
extern "C"
void	startElement(void *user_data, const xmlChar *name,
		const xmlChar **attrs) {
	astro::process::ProcessorParser	*parser
		= (astro::process::ProcessorParser *)user_data;
	std::string	_name((char *)name);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start: '%s'", _name.c_str());
	astro::process::attr_t	_attrs;
	if (attrs) {
		const xmlChar **p = attrs;
		while (*p) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "attribute %s", *p);
			std::string	attr((char *)*p++);
			if (*p) {
				std::string	value((char *)*p++);
				_attrs.insert(std::make_pair(attr, value));
			}
		}
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "element has no attributes");
	}
	parser->startElement(_name, _attrs);
}

/**
 * \brief SAX function for handling end elements
 */
extern "C"
void	endElement(void *user_data, const xmlChar *name) {
	astro::process::ProcessorParser	*parser
		= (astro::process::ProcessorParser *)user_data;
	std::string	_name((char *)name);
	parser->endElement(_name);
}

/**
 * \brief SAX function for handling character data
 */
extern "C"
void	characters(void *user_data, const xmlChar *name, int len) {
	astro::process::ProcessorParser	*parser
		= (astro::process::ProcessorParser *)user_data;
	std::string	_name((char *)name, len);
	parser->characters(_name);
}

/**
 * \brief SAX function for handling warnings
 */
extern "C"
void	warning(void *user_data, const char *msg, ...) {
	astro::process::ProcessorParser	*parser
		= (astro::process::ProcessorParser *)user_data;
	va_list	args;
	va_start(args, msg);
	std::string	s = astro::vstringprintf(msg, args);
	parser->warning(s);
	va_end(args);
}

/**
 * \brief SAX function for handling error
 */
extern "C"
void	error(void *user_data, const char *msg, ...) {
	astro::process::ProcessorParser	*parser
		= (astro::process::ProcessorParser *)user_data;
	va_list	args;
	va_start(args, msg);
	std::string	s = astro::vstringprintf(msg, args);
	parser->error(s);
	va_end(args);
}

/**
 * \brief SAX function for handling fatal errors
 */
extern "C"
void	fatal(void *user_data, const char *msg, ...) {
	astro::process::ProcessorParser	*parser
		= (astro::process::ProcessorParser *)user_data;
	va_list	args;
	va_start(args, msg);
	std::string	s = astro::vstringprintf(msg, args);
	parser->fatal(s);
	va_end(args);
}

