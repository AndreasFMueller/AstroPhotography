/*
 * ImageInfo.h -- auxiliary class to simplify image list processing
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _ImageInfo_h
#define _ImageInfo_h

#include <time.h>
#include <string>

namespace snowgui {

class ImageInfo {
	std::string	_name;
public:
	const std::string&	name() const { return _name; }
private:
	time_t	_when;
	std::string	_whenString;
	std::string	_dateString;
	std::string	_timeString;
public:
	time_t	when() const { return _when; }
	const std::string	whenString() const { return _whenString; }
	void	when(time_t t);
	const std::string	dateString() const { return _dateString; }
	const std::string	timeString() const { return _timeString; }
	int	age() const;
	void	age(int a);
private:
	size_t	_size;
	std::string	_sizeString;
public:
	size_t	size() const { return _size; }
	const std::string& 	sizeString() const { return _sizeString; }
	void	size(size_t s);
	
public:
	ImageInfo(const std::string& name);
	bool	operator<(const ImageInfo& other) const;
};

} // namespace snowgui

#endif /* _ImageInfo_h */
