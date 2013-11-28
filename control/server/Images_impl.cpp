/*
 * Images_impl.cpp -- servant for images in a directory
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Images_impl.h>
#include <sys/stat.h>
#include <time.h>
#include <AstroDebug.h>
#include <dirent.h>
#include <list>
#include <OrbSingleton.h>

namespace Astro {

Images_impl::Images_impl() {
}

CORBA::Long	Images_impl::imageSize(const char *name) {
	return fileSize(name);
}

CORBA::Long	Images_impl::imageAge(const char *name) {
	return fileAge(name);
}

Images::ImageList*	Images_impl::listImages() {
	// read all file names
	std::list<std::string>	names = fileList();

	// now convert this into a corba list
	Astro::Images::ImageList	*list
		= new Astro::Images::ImageList();
	list->length(names.size());
	std::list<std::string>::const_iterator	i;
	int	j = 0;
	for (i = names.begin(); i != names.end(); i++, j++) {
		(*list)[j++] = ::CORBA::string_dup(i->c_str());
	}
	return list;
}

Image_ptr	Images_impl::getImage(const char *_name) {
	return ImageDirectory::getImage(std::string(_name));
}

} // namespace Astro
