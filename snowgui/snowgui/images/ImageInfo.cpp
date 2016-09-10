/*
 * ImageInfo.cpp
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "ImageInfo.h"
#include <AstroFormat.h>

namespace snowgui {

ImageInfo::ImageInfo(const std::string& name) : _name(name) {
}

void	ImageInfo::size(size_t s) {
	_size = s;
	_sizeString = astro::stringprintf("%lu", _size);
}

void	ImageInfo::when(time_t t) {
	_when = t;
	struct tm	*tmp = localtime(&_when);
        char    buffer[100];
        strftime(buffer, sizeof(buffer), "%F %T", tmp);
	_whenString = std::string(buffer);

        strftime(buffer, sizeof(buffer), "%F", tmp);
	_dateString = std::string(buffer);

        strftime(buffer, sizeof(buffer), "%T", tmp);
	_timeString = std::string(buffer);
}

int	ImageInfo::age() const {
	time_t	now;
	time(&now);
	int	a = now - _when;
	return a;
}

void	ImageInfo::age(int a) {
	time_t	now;
	time(&now);
	now -= a;
	when(now);
}

bool	ImageInfo::operator<(const ImageInfo& other) const {
	return _when < other._when;
}

} // namespace snowgui
