/*
 * Testtable.h -- Testtable implementation to support Table class development
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _Testtable_h
#define _Testtable_h

#include <AstroPersistence.h>
#include <iostream>

namespace astro {
namespace persistence {

/**
 * \brief Entry in the test table
 */
class	TestEntry {
	int	_id;
public:
	int	id() const { return _id; }

public:
	int		_intfield;
public:
	int	intfield() const { return _intfield; }
	void	intfield(int i) { _intfield = i; }

private:
	double		_doublefield;
public:
	double	doublefield() const { return _doublefield; }
	void	doublefield(double d) { _doublefield = d; }

private:
	std::string	_stringfield;
public:
	const std::string&	stringfield() const { return _stringfield; }
	void	stringfield(const std::string s) { _stringfield = s; }

	TestEntry(int id) : _id(id) { }
};

std::ostream&	operator<<(std::ostream& out, const TestEntry& entry);

/**
 * \brief Table adapter 
 */
class	TesttableAdapter {
public:
static std::string	tablename();
static TestEntry	row_to_object(int objectid, const Row& row);
static UpdateSpec	object_to_updatespec(const TestEntry& entry);
};

} // namespace persistence
} // namespace astro

#endif /* _Testtable_h */
