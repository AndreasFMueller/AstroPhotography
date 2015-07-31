/*
 * ConditionIterator.cpp -- iterator restricted to a condition
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "CatalogIterator.h"

namespace astro {
namespace catalog {

/**
 * \brief Construct a condition iterator
 */
ConditionIterator::ConditionIterator(IteratorImplementationPtr iterator,
	IteratorPredicatePtr predicate, bool begin_or_end)
	: IteratorImplementation(begin_or_end),
	  _iterator(iterator), _predicate(predicate) {
	// ensure we have seen the first star
	increment();
}

/**
 * \brief Destroy the condition iterator
 */
ConditionIterator::~ConditionIterator() {
}

/**
 * \brief Retrieve the star the iterator is currently pointing to
 */
Star	ConditionIterator::operator*() {
	return *current_star;
}

/**
 * \brief Compare the iterator to another implementation
 */
bool	ConditionIterator::operator==(const IteratorImplementation& other) const {
	return equal_implementation(this, other);
}

/**
 * \brief Compare the Condiation iterators
 */
bool	ConditionIterator::operator==(const ConditionIterator& other) const {
	if (_iterator->isEnd() != other._iterator->isEnd()) {
		return false;
	}
	return (*_iterator == *other._iterator);
}

/**
 * \brief Convert the current iterator position into a string
 */
std::string	ConditionIterator::toString() const {
	return current_star->toString();
}

/**
 * \brief Increment the condition iterator
 *
 * The condition iterator's increment method increments the embedded iterator
 * until it finds a star that satisfies the condition or it hits the end
 * of the iterator.
 */
void	ConditionIterator::increment() {
	// don't do anything if we are already at the end
	if (isEnd()) {
		return;
	}

	// increment as long as we are not at the end
	while (!_iterator->isEnd()) {
		// increment the iterator
		_iterator->increment();

		// if iteration brought us to the end, we give up
		if (_iterator->isEnd()) {
			_isEnd = true;
			return;
		}

		// inspect the star the iterator now points to
		try {
			Star	s = _iterator->operator*();
			if ((*_predicate)(s)) {
				current_star = StarPtr(new Star(s));
				return;
			}
		} catch (const std::exception& x) {
			// just ignore positions that don't resolv to a
			// star
		}
	}
}

} // namespace catalog
} // namespace astro
