/*
 * FileBackendIterator.cpp -- iterator for the File backend
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <CatalogBackend.h>
#include "CutoverConditions.h"

namespace astro {
namespace catalog {

/**
 * \brief Create the FileBackendIterator
 *
 * Create a FileBackend iterator that points to first star that satisfies
 * the condition.
 */
FileBackendIterator::FileBackendIterator(FileBackend& filebackend) 
	: IteratorImplementation(true), _filebackend(filebackend) {
	// initialize the iterators 
	current_backend = CatalogFactory::BSC;
	current_iterator = current_catalog()->begin();
	condition = CutoverConditionPtr(new BSCCondition());
	// advance to the first valid star
	advance();
}

/**
 * \brief Destroy the iterator
 */
FileBackendIterator::~FileBackendIterator() {
}

/**
 * \brief Get the current catalog based on the current catalog type
 */
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

/**
 * \brief Comparision with iterators of the same type
 */
bool	FileBackendIterator::operator==(const FileBackendIterator& other) const {
	if (isEnd() == other.isEnd()) {
		return true;
	}
	if (current_backend != other.current_backend) {
		return false;
	}
	return current_iterator == other.current_iterator;
}

/**
 * \brief Comparison with other implementations
 */
bool	FileBackendIterator::operator==(const IteratorImplementation& other) const {
	return equal_implementation(this, other);
}

/**
 * \brief Switch to the next catalog
 */
void	FileBackendIterator::nextcatalog() {
	switch (current_backend) {
	case CatalogFactory::BSC:
		debug(LOG_DEBUG, DEBUG_LOG, 0, "switching to Hipparcos");
		current_backend = CatalogFactory::Hipparcos;
		condition = CutoverConditionPtr(new HipparcosCondition());
		break;
	case CatalogFactory::Hipparcos:
		debug(LOG_DEBUG, DEBUG_LOG, 0, "switching to Tycho2");
		current_backend = CatalogFactory::Tycho2;
		condition = CutoverConditionPtr(new Tycho2Condition());
		break;
	case CatalogFactory::Tycho2:
		debug(LOG_DEBUG, DEBUG_LOG, 0, "switching to Ucac4");
		current_backend = CatalogFactory::Ucac4;
		condition = CutoverConditionPtr(new Ucac4Condition());
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
	current_iterator = current_catalog()->begin();
}

/**
 * \brief Increment the iterator
 *
 * Incrementing for the FileBackend means to first increment the current
 * pointer, and then advance until you find a star that satisfies the
 * condition.
 */
void	FileBackendIterator::increment() {
	++current_iterator;
	advance();
}

/**
 * \brief Advance the iterator
 *
 * This method advances the iterator until it finds the next valid star
 */
void	FileBackendIterator::advance() {
	// advance the iterator until we are at the end
	while (!isEnd()) {
		while (!current_iterator.isEnd()) {
			// check whether the current star satisfies the
			// condition
			try {
				Star	star = *current_iterator;
				if ((*condition)(star)) {
					current_star = StarPtr(new Star(star));
					return;
				}
			} catch (const std::exception& x) {
				// ignore any star that causes an exception
			}
			// advance the iterator
			++current_iterator;
		}

		// if we get to this point, then the iterator has ended,
		// so we have to advance to the next catalog
		nextcatalog();
	}
}

/**
 * \brief dereference the iterator
 */
Star	FileBackendIterator::operator*() {
	if (isEnd()) {
		throw std::logic_error("cannot dereference end iterator");
	}
	return *current_star;
}

/**
 * \brief Get a string representation for the iterator
 */
std::string	FileBackendIterator::toString() const {
	if (isEnd()) {
		return std::string("end");
	}
	return current_iterator.toString();
}

} // namespace catalog
} // namespace astro
