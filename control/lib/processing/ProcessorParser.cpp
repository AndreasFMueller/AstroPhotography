/*
 * ProcessorParser.cpp
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "ProcessorParser.h"
#include <AstroProcess.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <includes.h>

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
	_handler.error = ::error;
	_handler.fatalError = ::fatal;
	_handler.warning = ::warning;
}

NodePaths&	ProcessorParser::parentNodePaths() {
	if (_parent) {
		return *_parent;
	}
	return *_network;
}

NodePaths&	ProcessorParser::nodePaths() {
	if (_stepstack.size() > 0) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "top path: %s",
			_stepstack.top()->NodePaths::info().c_str());
		return *_stepstack.top();
	}
	return *_network;
}

/**
 * \brief Handle a start element
 */
void	ProcessorParser::startElement(const std::string& name,
		const std::map<std::string, std::string>& attrs) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found element '%s'", name.c_str());
	if (name == std::string("process")) {
		startProcess(attrs);
		return;
	}
	if (name == std::string("fileimage")) {
		startFileimage(attrs);
		return;
	}
	if (name == std::string("writefileimage")) {
		startWritefileimage(attrs);
		return;
	}
	if (name == std::string("darkimage")) {
		startDarkimage(attrs, camera::Exposure::dark);
		return;
	}
	if (name == std::string("biasimage")) {
		startDarkimage(attrs, camera::Exposure::bias);
		return;
	}
	if (name == std::string("flatimage")) {
		startFlatimage(attrs);
		return;
	}
	if (name == std::string("calibrate")) {
		startCalibrate(attrs);
		return;
	}
	if (name == std::string("stack")) {
		startStack(attrs);
		return;
	}
	if (name == std::string("sum")) {
		startSum(attrs);
		return;
	}
	if (name == std::string("transform")) {
		startTransform(attrs);
		return;
	}
	if (name == std::string("rescale")) {
		startRescale(attrs);
		return;
	}
	if (name == std::string("color")) {
		startColor(attrs);
		return;
	}
	if (name == std::string("colorclamp")) {
		startColorclamp(attrs);
		return;
	}
	if (name == std::string("destar")) {
		startDestar(attrs);
		return;
	}
	if (name == std::string("hdr")) {
		startHDR(attrs);
		return;
	}
	if (name == std::string("layers")) {
		startLayerImage(attrs);
		return;
	}
	if (name == std::string("rgb")) {
		startRGB(attrs);
		return;
	}
	if (name == std::string("lrgb")) {
		startLRGB(attrs);
		return;
	}
	if (name == std::string("imageplane")) {
		startImagePlane(attrs);
		return;
	}
	if (name == std::string("luminancestretching")) {
		startLuminanceStretching(attrs);
		return;
	}
	if (name == std::string("image")) {
		startImage(attrs);
		return;
	}
	std::string	msg = stringprintf("don't know how to handle <%s>",
		name.c_str());
	throw std::runtime_error(msg);
}

/**
 * \brief Handle an end element
 */
void	ProcessorParser::endElement(const std::string& name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "element '%s' closed", name.c_str());
	if (name == std::string("process")) {
		endProcess();
		return;
	}
	endCommon();
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
	// use the libXML function to actuall run the parser over the file
	int	rc = xmlSAXUserParseFile(&_handler, this, filename.c_str());
	if (rc != 0) {
		std::string	msg = stringprintf("parse error in file %s: %d",
			filename.c_str(), rc);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	return _network;
}

/**
 * \brief Parse the data in a buffer
 *
 * \param data	the dadta block to parse
 * \param size	length of the data block to parse
 */
ProcessorNetworkPtr	ProcessorParser::parse(const char *data, int size) {
	_network.reset();
	// use the libXML function to actuall run the parser over the file
	int	rc = xmlSAXUserParseMemory(&_handler, this, data, size);
	if (rc != 0) {
		std::string	msg = stringprintf("parse error: %d", rc);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	return _network;
}

/**
 * \brief Push a processing stap on the stack
 *
 * \param step	the new proessing step to push on the stack
 */
void	ProcessorParser::push(ProcessingStepPtr step) {
	if (_stepstack.size() > 0) {
		_parent = _stepstack.top();
	}
	_stepstack.push(step);
}

/**
 * \brief Remove the processing step at the top of the stack
 */
void	ProcessorParser::pop() {
	if (_stepstack.size() == 0) {
		throw std::logic_error("stepstack is empty");
	}
	_stepstack.pop();
	if (_stepstack.size() == 0) {
		_parent.reset();
	} else {
		_parent = _stepstack.top();
	}
}

/**
 * \brief The processing step at the top of the stack
 */
ProcessingStepPtr	ProcessorParser::top() {
	if (_stepstack.size() == 0) {
		throw std::logic_error("stepstack is empty");
	}
	return _stepstack.top();
}

} // namespace process
} // namespace astro

/**
 * \brief SAX function for handling start elements
 */
extern "C"
void	startElement(void *user_data, const xmlChar *name,
		const xmlChar **attrs) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "<%s>", name);
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
	try {
		parser->startElement(_name, _attrs);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "start element '%s' error: %s",
			_name.c_str(), x.what());
	}
}

/**
 * \brief SAX function for handling end elements
 */
extern "C"
void	endElement(void *user_data, const xmlChar *name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "</%s>", name);
	astro::process::ProcessorParser	*parser
		= (astro::process::ProcessorParser *)user_data;
	std::string	_name((char *)name);
	try {
		parser->endElement(_name);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "end element '%s' error: %s",
			_name.c_str(), x.what());
	}
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

