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
	uuid_unparse(out, buffer);
	_uuid = std::string(buffer);
}

UUID::UUID(const std::string& uuid) : _uuid(uuid) {
}

bool	UUID::operator==(const UUID& other) const {
	uuid_t	a, b;
	uuid_parse(_uuid.c_str(), a);
	uuid_parse(other._uuid.c_str(), b);
	return (0 == uuid_compare(a, b));
}

UUID::operator	std::string() const {
	return _uuid;
}

} // namespace astro
