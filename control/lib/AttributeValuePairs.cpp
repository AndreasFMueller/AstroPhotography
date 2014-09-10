/*
 * AttributeValuePairs.cpp -- attriute value parser class
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUtils.h>
#include <AstroFormat.h>
#include <AstroDebug.h>

namespace astro {

/**
 * \brief parse arguments
 */
AttributeValuePairs::pair_t	AttributeValuePairs::parse(const std::string& argument) const {
	std::string::size_type	s;
	if ((s = argument.find('=')) == std::string::npos) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "not a pair: %s",
			argument.c_str());
		throw std::runtime_error("not an attribute-value pair");
	}
	std::string	attribute = argument.substr(0, s);
	std::string	value = argument.substr(s + 1);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "adding pair %s -> %s",
		attribute.c_str(), value.c_str());
	return std::make_pair(attribute, value);
}

/**
 * \brief Default constructor creates an empty container
 */
AttributeValuePairs::AttributeValuePairs() {
}

/**
 * \brief Constructor from a set of argument strings
 *
 * \param arguments	argument strings to be parsed as attribute value pairs
 * \param skip		number of parameters to skip before starting to parse
 */
AttributeValuePairs::AttributeValuePairs(const std::vector<std::string>& arguments, int skip) {
	std::vector<std::string>::const_iterator	ai = arguments.begin();
	std::advance(ai, skip);
	while (ai != arguments.end()) {
		try {
			pair_t	p = parse(*ai);
			data.insert(p);
		} catch (...) { }
		ai++;
	}
}

/**
 * \brief Find out whether an attribute of a given name exists
 */
bool	AttributeValuePairs::has(const std::string& attribute) const {
	return data.find(attribute) != data.end();
}

/**
 * \brief Retreive the first value for a given attribute
 */
std::string	AttributeValuePairs::operator()(const std::string& attribute) const {
	if (!has(attribute)) {
		std::string	msg = stringprintf("attribute '%s' not found",
			attribute.c_str());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	std::string	value = data.find(attribute)->second;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %s -> %s", attribute.c_str(),
		value.c_str());
	return value;
}

/**
 * \brief Get all values for an attribute
 */
std::set<std::string>	AttributeValuePairs::get(const std::string& attribute) const {
	std::set<std::string>	result;
	throw std::runtime_error("XXX get not implemented");
	return result;
}

} // namespace astro
