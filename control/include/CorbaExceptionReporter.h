/*
 * CorbaExceptionReporter.h
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _CorbaExceptionReporter_h
#define _CorbaExceptionReporter_h

#include <omniORB4/CORBA.h>
#include <string>

namespace Astro {

class ExceptionReporter {
public:
	std::string	operator()(const CORBA::Exception& x) const;
	std::string	operator()(const CORBA::SystemException& x) const;
	std::string	operator()(const CORBA::UserException& x) const;
};

std::string	exception2string(const CORBA::Exception& x);

} // namespace Astro

#endif /* _CorbaExceptionReporter_h */
