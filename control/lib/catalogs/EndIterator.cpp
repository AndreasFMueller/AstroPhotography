/*
 * EndIterator.cpp -- generic end iterator implementation
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "CatalogIterator.h"

namespace astro {
namespace catalog {

GenericEndIteratorImplementation::GenericEndIteratorImplementation()
	: IteratorImplementation(false) {
}

Star	GenericEndIteratorImplementation::operator*() {
	throw std::logic_error("cannot dereference end iterator");
}

bool	GenericEndIteratorImplementation::operator==(
		const GenericEndIteratorImplementation& /* other */) const {
	return true;
}

bool    GenericEndIteratorImplementation::operator==(
		const IteratorImplementation& other) const {
	return equal_implementation(this, other);
}

std::string     GenericEndIteratorImplementation::toString() const {
	return std::string("end");
}

void    GenericEndIteratorImplementation::increment() {
	throw std::logic_error("cannot increment end iterator");
}


} // namespace catalog
} // namespace astro
