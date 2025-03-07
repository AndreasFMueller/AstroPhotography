/*
 * UUID.cpp  -- generated universally unique id
 * 
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroUtils.h>
#include <AstroDebug.h>

namespace astro {

UUID::UUID() {
	uuid_t	out;
	uuid_generate_time(out);
	char	buffer[37];
	uuid_unparse_lower(out, buffer);
	_uuid = std::string(buffer);
}

UUID::UUID(const std::string& uuid) : _uuid(uuid) {
}

UUID::UUID(const UUID& other) {
	_uuid = other._uuid;
}

bool	UUID::operator==(const UUID& other) const {
	uuid_t	a, b;
	uuid_parse(_uuid.c_str(), a);
	uuid_parse(other._uuid.c_str(), b);
	return (0 == uuid_compare(a, b));
}

bool	UUID::operator<(const UUID& other) const {
	return _uuid < other._uuid;
}

UUID&	UUID::operator=(const UUID& other) {
	_uuid = other._uuid;
	return *this;
}

UUID::operator	std::string() const {
	return _uuid;
}

std::ostream&	operator<<(std::ostream& out, const UUID& uuid) {
	return out << (std::string)uuid;
}

} // namespace astro
