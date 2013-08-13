/*
 * NameService.h -- Naming and binding of objects 
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _NameService_h
#define _NameService_h

#include <string>
#include <vector>
#include <omniORB4/CORBA.h>

namespace Astro {
namespace Naming {

/**
 * \brief Name class for the composite names used in COS Naming
 */
class Name {
	std::string	_id;
	std::string	_kind;
public:
	Name(const std::string& id, const std::string& kind)
		: _id(id), _kind(kind) { }
	std::string	id() const { return _id; }
	std::string	kind() const { return _kind; }
	std::string	toString() const;
};

/**
 * \brief Names are sequences of simple name components
 */
class Names : public std::vector<Name> {
public:
	std::string	toString() const;
};

/**
 * \brief A Wrapper to simplify access to the CORBA Naming Service
 */
class NameService {
	CosNaming::NamingContext_var	rootContext;
public:
	NameService(CORBA::ORB_var orb);
	CORBA::Object_var	lookup(const Names& names);
	void	bind(const Names& names, CORBA::Object_var obj);
};

} // namespace Naming
} // namespace Astro

#endif /* _NameService_h */
