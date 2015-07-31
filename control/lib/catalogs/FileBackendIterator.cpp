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
		current_iterator = current_catalog()->begin();
	} else {
		current_backend = CatalogFactory::Ucac4;
		current_iterator = current_catalog()->end();
	}
}

FileBackendIterator::~FileBackendIterator() {
}

CatalogPtr	FileBackendIterator::current_catalog() {
	switch (current_backend) {
	case CatalogFactory::BSC:
		return _filebackend.bsc_catalog;
	case CatalogFactory::Hipparcos:
		return _filebackend.hipparcos_catalog;
	case CatalogFactory::Tycho2:
		return _filebackend.tycho2_catalog;
	case CatalogFactory::Ucac4:
		return _filebackend.ucac4_catalog;
	default:
		throw std::logic_error("not a valid backend");
	}
}

bool	FileBackendIterator::operator==(const FileBackendIterator& other) const {
	if (isEnd() == other.isEnd()) {
		return true;
	}
	if (current_backend != other.current_backend) {
		return false;
	}
	return current_iterator == other.current_iterator;
}

bool	FileBackendIterator::operator==(const IteratorImplementation& other) const {
	return equal_implementation(this, other);
}

void	FileBackendIterator::nextcatalog() {
	switch (current_backend) {
	case CatalogFactory::BSC:
		debug(LOG_DEBUG, DEBUG_LOG, 0, "switching to Hipparcos");
		current_backend = CatalogFactory::Hipparcos;
		break;
	case CatalogFactory::Hipparcos:
		debug(LOG_DEBUG, DEBUG_LOG, 0, "switching to Tycho2");
		current_backend = CatalogFactory::Tycho2;
		break;
	case CatalogFactory::Tycho2:
		debug(LOG_DEBUG, DEBUG_LOG, 0, "switching to Ucac4");
		current_backend = CatalogFactory::Ucac4;
		break;
	case CatalogFactory::Ucac4:
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot go beyond Ucac4");
		_isEnd = true;
		return;
	default:
		std::string	msg = stringprintf("nextcatalog from %d not possible", current_backend);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::logic_error(msg);
	}
}

void	FileBackendIterator::increment() {
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
	current_iterator = current_catalog()->begin();
}

Star	FileBackendIterator::operator*() {
	return *current_iterator;
}

std::string	FileBackendIterator::toString() const {
	return current_iterator.toString();
}

} // namespace catalog
} // namespace astro
