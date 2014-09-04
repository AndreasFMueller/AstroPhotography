/*
 * Metavalue.cpp -- value class for Image Metadata
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroImage.h>
#include <fitsio.h>
#include <AstroFormat.h>

namespace astro {
namespace image {

Metavalue::Metavalue(const std::string& _value, const std::string& _comment)
	: datatype(typeid(_value)), value(_value), comment(_comment) {
}

Metavalue::Metavalue(const bool b, const std::string& _comment)
	: datatype(typeid(b)), comment(_comment) {
	value = (b) ? std::string("T") : std::string("F");
}

Metavalue::Metavalue(const char c, const std::string& _comment)
	: datatype(typeid(c)), comment(_comment) {
	value = stringprintf("%d", c);
}

Metavalue::Metavalue(const unsigned char uc, const std::string& _comment)
	: datatype(typeid(uc)), comment(_comment) {
	value = stringprintf("%u", uc);
}

Metavalue::Metavalue(const short s, const std::string& _comment)
	: datatype(typeid(s)), comment(_comment) {
	value = stringprintf("%hd", s);
}

Metavalue::Metavalue(const unsigned short us, const std::string& _comment)
	: datatype(typeid(us)), comment(_comment) {
	value = stringprintf("%hu", us);
}

Metavalue::Metavalue(const int i, const std::string& _comment)
	: datatype(typeid(i)), comment(_comment) {
	value = stringprintf("%d", i);
}

Metavalue::Metavalue(const unsigned int ui, const std::string& _comment)
	: datatype(typeid(ui)), comment(_comment) {
	value = stringprintf("%u", ui);
}

Metavalue::Metavalue(const long l, const std::string& _comment)
	: datatype(typeid(l)), comment(_comment) {
	value = stringprintf("%ld", l);
}

Metavalue::Metavalue(const unsigned long ul, const std::string& _comment)
	: datatype(typeid(ul)), comment(_comment) {
	value = stringprintf("%lu", ul);
}

Metavalue::Metavalue(const float f, const std::string& _comment)
	: datatype(typeid(f)), comment(_comment) {
	value = stringprintf("%f", f);
}

Metavalue::Metavalue(const double f, const std::string& _comment)
	: datatype(typeid(f)), comment(_comment) {
	value = stringprintf("%f", f);
}

Metavalue::Metavalue(std::type_index _datatype, const std::string& _value,
	const std::string& _comment)
		: datatype(_datatype), value(_value), comment(_comment) {
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

} // namespace image
} // namespace astro
