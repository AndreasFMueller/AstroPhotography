/*
 * FileBackendIterator.cpp
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <CatalogBackend.h>

namespace astro {
namespace catalog {

FileBackendIterator::FileBackendIterator(FileBackend& filebackend,
	bool begin_or_end)
		: IteratorImplementation(begin_or_end),
		  _filebackend(filebackend) {
	if (begin_or_end) {
		current_backend = CatalogFactory::BSC;
		current_catalog = filebackend.bsc_catalog;
		current_iterator = current_catalog->begin();
	} else {
		current_backend = CatalogFactory::Ucac4;
		current_catalog = filebackend.ucac4_catalog;
		current_iterator = current_catalog->end();
	}
}

FileBackendIterator::~FileBackendIterator() {
}

bool	FileBackendIterator::operator==(const FileBackendIterator& other) const {
	if (isEnd() == other.isEnd()) {
		return true;
	}
	if (current_backend != other.current_backend) {
		return false;
	}
	if (current_catalog != other.current_catalog) {
		return false;
	}
	return current_iterator == other.current_iterator;
}

bool	FileBackendIterator::operator==(const IteratorImplementation& other) const {
	return equal_implementation(this, other);
}

void	FileBackendIterator::increment() {
	// make sure we are not at the end of the last catalog
	if (isEnd()) {
		return;
	}

	// do the common increment stuff
	++current_iterator;
	if (current_iterator != current_catalog->end()) {
		return;
	}

	// now to the catalog specific stuff
	switch (current_backend) {
	case CatalogFactory::BSC:
		current_backend = CatalogFactory::Hipparcos;
		current_catalog = _filebackend.hipparcos_catalog;
		break;
	case CatalogFactory::Hipparcos:
		current_backend = CatalogFactory::Tycho2;
		current_catalog = _filebackend.tycho2_catalog;
		break;
	case CatalogFactory::Tycho2:
		current_backend = CatalogFactory::Ucac4;
		current_catalog = _filebackend.ucac4_catalog;
		break;
	case CatalogFactory::Ucac4:
		_isEnd = true;
		return;
	}
	current_iterator = current_catalog->begin();
}

Star	FileBackendIterator::operator*() {
	return *current_iterator;
}

std::string	FileBackendIterator::toString() const {
	return current_iterator.toString();
}

} // namespace catalog
} // namespace astro
