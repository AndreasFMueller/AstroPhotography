/*
 * ProcessorParser.h -- auxiliary processor parser class
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _ProcessorParser_h
#define _ProcessorParser_h

#include <AstroProcess.h>
#include <libxml/parser.h>
#include <stack>

namespace astro {
namespace process {

typedef std::map<std::string, std::string>	attr_t;

/**
 * \brief Class to parse a processor network XML file
 */
class ProcessorParser {
	xmlSAXHandler		_handler;
	ProcessorNetworkPtr	_network;
	std::stack<ProcessingStepPtr>	_stepstack;
	ProcessingStepPtr	_parent;
	NodePaths&	parentNodePaths();
	NodePaths&	nodePaths();
	void	push(ProcessingStepPtr step);
	void	pop();
	ProcessingStepPtr	top();
	// private methods for handling different types of processing steps
	void	startCommon(const attr_t& attrs);
	void	endCommon();

	void	setNodePaths(NodePaths& nodepaths, const attr_t& attrs,
			NodePaths *parent = NULL);

	void	startProcess(const attr_t& attrs);
	void	endProcess();

	void	startFileimage(const attr_t& attrs);
	void	startDarkimage(const attr_t& attrs,
			camera::Exposure::purpose_t purpose);
	void	startFlatimage(const attr_t& attrs);
	void	startCalibrate(const attr_t& attrs);
	void	startWritefileimage(const attr_t& attrs);
	void	startStack(const attr_t& attrs);
	void	startTransform(const attr_t& attrs);
	void	startColor(const attr_t& attrs);
	void	startColorclamp(const attr_t& attrs);
	void	startHDR(const attr_t& attrs);
	void	startRescale(const attr_t& attrs);
	void	startDestar(const attr_t& attrs);
	void	startLuminanceStretching(const attr_t& attrs);
public:
	ProcessorParser();
	void	startElement(const std::string& name, const attr_t& attrs);
	void	endElement(const std::string& name);
	void	characters(const std::string& data);

	void	warning(const std::string& msg);
	void	error(const std::string& msg);
	void	fatal(const std::string& msg);

	ProcessorNetworkPtr	parse(const std::string& filename);
	ProcessorNetworkPtr	parse(const char *data, int size);
};

} // namespace process
} // namespace astro

extern "C"
void	startElement(void *user_data, const xmlChar *name,
		const xmlChar **attrs);

extern "C"
void	endElement(void *user_data, const xmlChar *name);

extern "C"
void	characters(void *user_data, const xmlChar *ch, int len);

extern "C"
void	warning(void *user_data, const char *msg, ...);

extern "C"
void	error(void *user_data, const char *msg, ...);

extern "C"
void	fatal(void *user_data, const char *msg, ...);

#endif /* _ProcessorParser_h */
