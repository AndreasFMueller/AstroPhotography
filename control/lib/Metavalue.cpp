/*
 * Metavalue.cpp -- value class for Image Metadata
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroImage.h>
#include <fitsio.h>
#include <AstroFormat.h>
#include <AstroIO.h>
#include <AstroUtils.h>

using namespace astro::io;

namespace astro {
namespace image {

void	Metavalue::standardize() {
	if (FITSKeywords::known(keyword)) {
		datatype = FITSKeywords::index(keyword);
		comment = FITSKeywords::comment(keyword);
	}
	if (keyword == "PURPOSE") {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "check value for PURPOSE: [%s]",
			value.c_str());
		value = trim(value);
			debug(LOG_DEBUG, DEBUG_LOG, 0, "illegal purpose: [%s]",
				value.c_str());
		if ((value != "dark") && (value != "flat")
			&& (value != "light")) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "illegal purpose: [%s]",
				value.c_str());
			throw std::runtime_error("illegal value for PURPOSE");
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "purpose: %s", value.c_str());
	}
	if ((keyword == "INSTRUME") || (keyword == "PURPOSE")
		|| (keyword == "PROJECT") || (keyword == "BAYER")) {
		value = rtrim(value);
	}
}

Metavalue::Metavalue(const std::string& _keyword, const std::string& _value,
	const std::string& _comment)
	: keyword(_keyword), datatype(typeid(_value)), value(_value),
	  comment(_comment) {
	standardize();
}

Metavalue::Metavalue(const std::string& _keyword, const bool b,
	const std::string& _comment)
	: keyword(_keyword), datatype(typeid(b)), comment(_comment) {
	value = (b) ? std::string("T") : std::string("F");
	standardize();
}

Metavalue::Metavalue(const std::string& _keyword, const char c,
	const std::string& _comment)
	: keyword(_keyword), datatype(typeid(c)), comment(_comment) {
	value = stringprintf("%d", c);
	standardize();
}

Metavalue::Metavalue(const std::string& _keyword, const unsigned char uc,
	const std::string& _comment)
	: keyword(_keyword), datatype(typeid(uc)), comment(_comment) {
	value = stringprintf("%u", uc);
	standardize();
}

Metavalue::Metavalue(const std::string& _keyword, const short s,
	const std::string& _comment)
	: keyword(_keyword), datatype(typeid(s)), comment(_comment) {
	value = stringprintf("%hd", s);
	standardize();
}

Metavalue::Metavalue(const std::string& _keyword, const unsigned short us,
	const std::string& _comment)
	: keyword(_keyword), datatype(typeid(us)), comment(_comment) {
	value = stringprintf("%hu", us);
	standardize();
}

Metavalue::Metavalue(const std::string& _keyword, const int i,
	const std::string& _comment)
	: keyword(_keyword), datatype(typeid(i)), comment(_comment) {
	value = stringprintf("%d", i);
	standardize();
}

Metavalue::Metavalue(const std::string& _keyword, const unsigned int ui,
	const std::string& _comment)
	: keyword(_keyword), datatype(typeid(ui)), comment(_comment) {
	value = stringprintf("%u", ui);
	standardize();
}

Metavalue::Metavalue(const std::string& _keyword, const long l,
	const std::string& _comment)
	: keyword(_keyword), datatype(typeid(l)), comment(_comment) {
	value = stringprintf("%ld", l);
	standardize();
}

Metavalue::Metavalue(const std::string& _keyword, const unsigned long ul,
	const std::string& _comment)
	: keyword(_keyword), datatype(typeid(ul)), comment(_comment) {
	value = stringprintf("%lu", ul);
	standardize();
}

Metavalue::Metavalue(const std::string& _keyword, const float f,
	const std::string& _comment)
	: keyword(_keyword), datatype(typeid(f)), comment(_comment) {
	value = stringprintf("%f", f);
	standardize();
}

Metavalue::Metavalue(const std::string& _keyword, const double f,
	const std::string& _comment)
	: keyword(_keyword), datatype(typeid(f)), comment(_comment) {
	value = stringprintf("%f", f);
	standardize();
}

Metavalue::Metavalue(const std::string& _keyword, const FITSdate& d,
	const std::string& _comment)
	: keyword(_keyword), datatype(typeid(FITSdate)), comment(_comment) {
	value = d.showVeryLong();
	standardize();
}

Metavalue::Metavalue(const std::string& _keyword, std::type_index _datatype,
	const std::string& _value, const std::string& _comment)
		: keyword(_keyword), datatype(_datatype), value(_value),
		  comment(_comment) {
	if (datatype != std::type_index(typeid(void))) {
		standardize();
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "don't standardize comments");
	}
}

Metavalue::operator bool() const {
	if ((value != "T") && (value != "F")) {
		throw std::runtime_error("not a boolean value");
	}
	return bool(value == "T");
}

Metavalue::operator char() const {
	if (value.size() != 1) {
		throw std::runtime_error("not a character");
	}
	return char(value[0]);
}

Metavalue::operator	unsigned char() const {
	if (value.size() != 1) {
		throw std::runtime_error("not a character");
	}
	unsigned char	result(value[0]);
	return result;
}

Metavalue::operator	short() const {
	long	l = std::stol(value);
	if (std::numeric_limits<short>::max() < l) {
		throw std::invalid_argument("not a short");
	}
	if (std::numeric_limits<short>::min() > l) {
		throw std::invalid_argument("not a short");
	}
	return (short)l;
}

Metavalue::operator	unsigned short() const {
	unsigned long	ul = std::stoul(value);
	if (ul > std::numeric_limits<unsigned short>::max()) {
		throw std::invalid_argument("not an unsigned short");
	}
	return (unsigned short)ul;
}

Metavalue::operator	int() const {
	return std::stoi(value);
}

Metavalue::operator	unsigned int() const {
	unsigned long	ul = std::stoul(value);
	if (ul > std::numeric_limits<unsigned int>::max()) {
		throw std::invalid_argument("not an unsigned int");
	}
	return (unsigned int)(ul);
}

Metavalue::operator	long() const {
	return std::stol(value);
}

Metavalue::operator	unsigned long() const {
	return std::stoul(value);
}

Metavalue::operator	float() const {
	return std::stof(value);
}

Metavalue::operator	double() const {
	return std::stod(value);
}

Metavalue::operator	std::string() const {
	return value;
}

Metavalue::operator	FITSdate() const {
	return FITSdate(value);
}

std::string	Metavalue::toString() const {
	return stringprintf("%s[%s]: %s / %s", 
		keyword.c_str(), datatype.name(),
		value.c_str(), comment.c_str());
}

bool	Metavalue::operator==(const Metavalue& other) const {
	return (keyword == other.keyword) && (value == other.value);
}

} // namespace image
} // namespace astro
