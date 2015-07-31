/*
 * FileBackendWindowIterator.cpp
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <CatalogBackend.h>

namespace astro {
namespace catalog {

void	FileBackendWindowIterator::advance() {
	while (!isEnd()) {
		current_iterator
			= current_catalog()->findIter(_window, _magrange);
		if (!current_iterator.isEnd()) {
			return;
		}
		nextcatalog();
		if (isEnd()) {
			return;
		}
	}
}

FileBackendWindowIterator::FileBackendWindowIterator(FileBackend& filebackend,
	const SkyWindow& window, const MagnitudeRange& magrange)
	: FileBackendIterator(filebackend, true), 
	  _window(window), _magrange(magrange) {
	current_backend = CatalogFactory::BSC;
	advance();
}

FileBackendWindowIterator::~FileBackendWindowIterator() {
}

bool	FileBackendWindowIterator::operator==(const FileBackendWindowIterator& other) const {
	if (isEnd() == other.isEnd()) {
		return true;
	}
	return FileBackendIterator::operator==(other);
}

bool	FileBackendWindowIterator::operator==(const IteratorImplementation& other) const {
	return equal_implementation(this, other);
}

void	FileBackendWindowIterator::increment() {
	// make sure we are not at the end of the last catalog
	if (isEnd()) {
		return;
	}

	// do the common increment stuff
	++current_iterator;
	if (!current_iterator.isEnd()) {
		return;
	}

	// now to the catalog specific stuff
	nextcatalog();
	advance();
}

} // namespace catalog
} // namespace astro
