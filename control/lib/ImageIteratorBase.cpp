/*
 * ImageBaseIterator.cpp -- ImageBaseIterator implementation
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <AstroImage.h>
#include <iostream>

namespace astro {
namespace image {

/**
 * \brief post increment operator
 *
 * increments the iterator and returns the value before increment
 */
ImageIteratorBase	ImageIteratorBase::operator++(int) {
	// return the current state, we are at the end already
	if (offset == -1) {
		return *this;
	}
	int	oldoffset = offset;
	offset += stride;
	if (offset > last) {
		offset = -1;
	}
	return ImageIteratorBase(first, last, oldoffset, stride);
}

/**
 * \brief post increment operator
 *
 * decrements the iterator and returns the value before increment
 */
ImageIteratorBase	ImageIteratorBase::operator--(int) {
	if (offset == -1) {
		return *this;
	}
	int	oldoffset = offset;
	offset -= stride;
	return ImageIteratorBase(first, last, oldoffset, stride);
}

/**
 * \brief pre increment operator
 *
 * increments the iterator and returns the incremented value
 */
ImageIteratorBase&	ImageIteratorBase::operator++() {
	if (offset == -1) {
		return *this;
	} 
	offset += stride;
	if (offset > last) {
		offset = -1;
	}
	return *this;
}

/**
 * \brief pre decrement operator
 *
 * decrement the iterator and return the decremented value
 */
ImageIteratorBase&	ImageIteratorBase::operator--() {
	if (offset == -1) {
		return *this;
	}
	offset -= stride;
	if (offset < first) {
		offset = -1;
	}
	return *this;
}

/**
 * \brief add steps to an iterator
 *
 * This method is equivalent to incrementing the iterator steps times or
 * decrementing the iterator -steps times if steps is negative.
 */
ImageIteratorBase	ImageIteratorBase::operator+(const int steps) const {
	if (offset == -1) {
		return *this;
	}
	int	newoffset = offset + steps * stride;
	if ((newoffset > last) || (newoffset < first)) {
		newoffset = -1;
	}
	return ImageIteratorBase(first, last, newoffset, stride);
}

/**
 * \brief subtract steps from an iterator
 *
 * This method is equivalent to decrementing the iterator steps times or
 * incrementing the iterator -steps times if steps is negative.
 */
ImageIteratorBase	ImageIteratorBase::operator-(const int steps) const {
	if (offset == -1) {
		return *this;
	}
	int	newoffset = offset - steps * stride;
	if ((newoffset < 0) || (newoffset > last)) {
		newoffset = -1;
	}
	return ImageIteratorBase(first, last, offset - steps * stride, stride);
}

/**
 * \brief comparison of iterators: equality
 */
bool    ImageIteratorBase::operator==(const ImageIteratorBase& other) const {
	return (other.offset == offset);
}

/**
 * \brief comparision of iterators: inequality
 */
bool    ImageIteratorBase::operator!=(const ImageIteratorBase& other) const {
	return (other.offset != offset);
}

/**
 * \brief test whether the iterator state is valid
 *
 * The iterator state is considered valid when the offset is inside
 * the interval defined by the first and last members.
 */
bool    ImageIteratorBase::valid() const {
	return ((first <= offset) && (offset <= last));
}

/**
 * \brief test whether iterator state is invalid
 *
 * The iterator state is considered invalid when the offset is outside
 * the interval defined by the first and last members.
 */
bool    ImageIteratorBase::invalid() const {
	return ((offset < first) || (offset > last));
}

/**
 * \brief compute the pixeloffset corresponding to the current iterator state
 *
 * \throws Throws a std::range_error if the iterator no longer points to
 *         a point inside the row or column.
 */
int	ImageIteratorBase::pixeloffset() const throw(std::range_error) {
	if (offset == -1) {
		throw std::range_error("image iterator out of range");
	}
	return offset;
}

} // namespace image
} // namespace astro
