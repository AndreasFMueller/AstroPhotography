/*
 * Iterator.cpp -- implementation of the CatalogIterator
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCatalog.h>
#include <CatalogIterator.h>

namespace astro {
namespace catalog {

CatalogIterator::CatalogIterator(IteratorImplementationPtr implementation)
	: _implementation(implementation) {
}

CatalogIterator::CatalogIterator() {
}

CatalogIterator::CatalogIterator(const CatalogIterator& other)
	: _implementation(other._implementation) {
}

CatalogIterator&	CatalogIterator::operator=(const CatalogIterator& other) {
	_implementation = other._implementation;
	return *this;
}

void	CatalogIterator::check() const {
	if (!_implementation) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"catalog iterator not initialized");
		throw std::logic_error("catalog iterator not initialized");
	}
}

Star	CatalogIterator::operator*() {
	check();
	return _implementation->operator*();
}

bool	CatalogIterator::operator==(const CatalogIterator& other) const {
	check();
	return (*_implementation) == (*other._implementation);
}

bool	CatalogIterator::operator!=(const CatalogIterator& other) const {
	check();
	return (*_implementation) != (*other._implementation);
}

CatalogIterator	CatalogIterator::operator++() {
	_implementation->increment();
	return *this;
}

std::string	CatalogIterator::toString() const {
	return _implementation->toString();
}

bool	CatalogIterator::isEnd() const {
	return _implementation->isEnd();
}

} // namespace catalog
} // namespace astro
