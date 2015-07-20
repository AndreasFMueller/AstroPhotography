/*
 * ImageMetadata.cpp -- access to metadata
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroImage.h>
#include <AstroFormat.h>
#include <AstroIO.h>

using namespace astro::io;

namespace astro {
namespace image {

/**
 * \brief access the meta data
 */
const Metavalue& ImageMetadata::getMetadata(const std::string& keyword) const {
	ImageMetadata::const_iterator	m = find(keyword);
	if (m == end()) {
		std::string	msg = stringprintf("no metadata for keyword '%s'",
			keyword.c_str());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	return m->second;
}

/**
 * \brief Check whether a metadata keyword is present
 */
bool	ImageMetadata::hasMetadata(const std::string& keyword) const {
	return (find(keyword) != end());
}

/**
 * \brief set metadata
 */
void    ImageMetadata::setMetadata(const Metavalue& mv) {
	const std::string&	name = mv.getKeyword();
	bool	unique = false;
	if (FITSKeywords::known(name)) {
		unique = FITSKeywords::unique(name);
	}
	if (hasMetadata(name) && unique) {
		find(name)->second = mv;
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "inserting %s",
			mv.toString().c_str());
		push_back(make_pair(name, mv));
	}
}

/**
 * \brief helper class to to look for a particular key
 */
class keyword_comparator {
	std::string	_keyword;
public:
	keyword_comparator(const std::string keyword) : _keyword(keyword) { }
	bool	operator()(const ImageMetadata::value_type& v) {
		return v.first == _keyword;
	}
};

/** 
 * \brief Find a metadata entry based on the name of the keyword
 */
ImageMetadata::const_iterator	ImageMetadata::find(const std::string& keyword) const {
	return std::find_if(begin(), end(), keyword_comparator(keyword));
}

/** 
 * \brief Find a metadata entry based on the name of the keyword
 */
ImageMetadata::iterator	ImageMetadata::find(const std::string& keyword) {
	return std::find_if(begin(), end(), keyword_comparator(keyword));
}

/**
 * \brief Delete all entries for a given keyword
 */
void	ImageMetadata::remove(const std::string& keyword) {
	iterator	i;
	while (end() != (i = find(keyword))) {
		erase(i);
	}
}

void	ImageMetadata::dump() const {
	int	counter = 0;
	for (auto ptr = begin(); ptr != end(); ptr++, counter++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "meta[%d] %s", counter,
			ptr->second.toString().c_str());
	}
}

} // namespace image
} // namespace astro
