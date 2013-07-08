/*
 * Metavalue.cpp -- value class for Image Metadata
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroImage.h>
#include <fitsio.h>
#include <Format.h>

namespace astro {
namespace image {

int	Metavalue::getType() const {
	return datatype;
}

std::string     Metavalue::getValue() const {
	return value;
}

std::string     Metavalue::getComment() const {
	return comment;
}

Metavalue::Metavalue(const std::string& _value, const std::string& _comment)
	: value(_value), comment(_comment) {
	datatype = TSTRING;
}

Metavalue::Metavalue(const bool b, const std::string& _comment)
	: comment(_comment)  {
	value = (b) ? std::string("T") : std::string("F");
	datatype = TLOGICAL;
}

Metavalue::Metavalue(const char c, const std::string& _comment)
	: comment(_comment) {
	value = stringprintf("%d", c);
	datatype = TBYTE;
}

Metavalue::Metavalue(const unsigned char uc, const std::string& _comment)
	: comment(_comment) {
	value = stringprintf("%u", uc);
	datatype = TBYTE;
}

Metavalue::Metavalue(const short s, const std::string& _comment)
	: comment(_comment) {
	value = stringprintf("%hd", s);
	datatype = TSHORT;
}

Metavalue::Metavalue(const unsigned short us, const std::string& _comment)
	: comment(_comment) {
	value = stringprintf("%hu", us);
	datatype = TUSHORT;
}

Metavalue::Metavalue(const int i, const std::string& _comment)
	: comment(_comment) {
	value = stringprintf("%d", i);
	datatype = TINT;
}

Metavalue::Metavalue(const unsigned int ui, const std::string& _comment)
	: comment(_comment) {
	value = stringprintf("%u", ui);
	datatype = TUINT;
}

Metavalue::Metavalue(const long l, const std::string& _comment)
	: comment(_comment) {
	value = stringprintf("%ld", l);
	datatype = TLONG;
}

Metavalue::Metavalue(const unsigned long ul, const std::string& _comment)
	: comment(_comment) {
	value = stringprintf("%lu", ul);
	datatype =TULONG;
}

Metavalue::Metavalue(const float f, const std::string& _comment)
	: comment(_comment) {
	value = stringprintf("%f", f);
	datatype = TFLOAT;
}

Metavalue::Metavalue(const double f, const std::string& _comment)
	: comment(_comment) {
	value = stringprintf("%f", f);
	datatype = TDOUBLE;
}

Metavalue::Metavalue(int _datatype, const std::string& _value,
	const std::string& _comment)
		: datatype(_datatype), value(_value), comment(_comment) {
}

} // namespace image
} // namespace astro
