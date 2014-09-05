/*
 * ImageMetadata.cpp -- access to metadata
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroImage.h>
#include <AstroFormat.h>

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
	if (hasMetadata(name)) {
		//metadata[name] = mv;
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "inserting '%s' value '%s'",
			name.c_str(), mv.getValue().c_str());
		insert(make_pair(name, mv));
	}
}


} // namespace image
} // namespace astro
