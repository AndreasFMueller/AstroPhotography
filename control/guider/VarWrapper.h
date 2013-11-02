/*
 * ObjWrapper.h -- wrapper around corba _var types that enable reference
 *                 counting allows us to use them in standard template
 *                 containers
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _ObjWrapper_h
#define _ObjWrapper_h

#include <AstroDebug.h>

namespace astro {

template<typename objtype>
class ObjWrapper : public std::shared_ptr<typename objtype::_var_type> {
public:
	typedef	typename objtype::_ptr_type	_ptr_type;
	typedef	typename objtype::_var_type	_var_type;
	ObjWrapper(_ptr_type t)
		: std::shared_ptr<_var_type>(new _var_type(t)) {
		_var_type	*var = &*(*this);
		_ptr_type	ptr = var->out();
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"%p: new wrapper %p for ptr %p and var %p",
			t, this,
			ptr,
			var);
	}
	virtual ~ObjWrapper() {
		_var_type	*var = &*(*this);
		_ptr_type	ptr = var->out();
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"%p going out of scope for ptr %p in var %p, %ld",
			this,
			ptr,
			var,
			this->use_count());
	}
};

} // namespace astro

#endif /* _ObjWrapper_h */
