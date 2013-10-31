/*
 * ObjWrapper.h -- wrapper around corba _var types that enable reference
 *                 counting allows us to use them in standard template
 *                 containers
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _ObjWrapper_h
#define _ObjWrapper_h

namespace astro {

template<typename objtype>
class ObjWrapper : public std::shared_ptr<typename objtype::_var_type> {
public:
	typedef	typename objtype::_ptr_type	_ptr_type;
	typedef	typename objtype::_var_type	_var_type;
	ObjWrapper(_ptr_type t)
		: std::shared_ptr<_var_type>(new _var_type(t)) {
		//*this = t;
	}
};


} // namespace astro

#endif /* _ObjWrapper_h */
