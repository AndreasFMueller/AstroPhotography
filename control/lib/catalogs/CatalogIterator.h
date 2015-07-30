/*
 * CatalogIterator.h -- Catalog Iterator internals 
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _CatalogIterator_h
#define _CatalogIterator_h

#include <AstroCatalog.h>
#include <AstroFormat.h>
#include <typeinfo>

namespace astro {
namespace catalog {

/**
 * \brief Protocol class for the iterator implementation
 */
class IteratorImplementation {
public:
	virtual ~IteratorImplementation();
	virtual Star	operator*() = 0;
	virtual bool	operator==(const IteratorImplementation& other) const = 0;
	bool	operator!=(const IteratorImplementation& other) const;
	virtual std::string	toString() const = 0;
private:
	virtual void	increment() = 0;
friend class CatalogIterator;
};

/**
 * \brief Comparison function template for iterator comparisons
 *
 * The main work this comparison function does is to ensure that the other
 * iterator implementation is in fact comparable.
 */
template<class iterimpl>
bool	equal_implementation(const iterimpl *me,
		const IteratorImplementation& other) {
	const iterimpl	*otherp = dynamic_cast<const iterimpl *>(&other);
	if (NULL == otherp) {
		std::string	msg = stringprintf("wrong iterator type %s, "
					"%s expected", typeid(other).name(),
					typeid(*me).name());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::logic_error(msg);
	}
	return (me->operator==(*otherp));
}

} // namespace catalog
} // namespace astro

#endif /* _CatalogIterator_h */
