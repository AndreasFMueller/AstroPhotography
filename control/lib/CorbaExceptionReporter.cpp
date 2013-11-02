/*
 * CorbaExceptionReporter.cpp
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <CorbaExceptionReporter.h>
#include <sstream>

namespace Astro {

std::string	ExceptionReporter::operator()(const CORBA::SystemException& x) const {
	std::ostringstream	out;

	// find the completion status for system exceptions
	const CORBA::SystemException	*se = NULL;

	// now analyze the various system exceptions
	if (NULL != (se = dynamic_cast<const CORBA::SystemException *>(&x))) {
		if (dynamic_cast<const CORBA::BAD_CONTEXT *>(&x)) {
			out << "BAD_CONTEXT: ";
		}
		if (dynamic_cast<const CORBA::BAD_INV_ORDER *>(&x)) {
			out << "BAD_INV_ORDER: ";
		}
		if (dynamic_cast<const CORBA::BAD_OPERATION *>(&x)) {
			out << "BAD_OPERATION: ";
		}
		if (dynamic_cast<const CORBA::BAD_PARAM *>(&x)) {
			out << "BAD_PARAM: ";
		}
		if (dynamic_cast<const CORBA::BAD_TYPECODE *>(&x)) {
			out << "BAD_TYPECODE: ";
		}
		if (dynamic_cast<const CORBA::COMM_FAILURE *>(&x)) {
			out << "COMM_FAILURE: ";
		}
		if (dynamic_cast<const CORBA::DATA_CONVERSION *>(&x)) {
			out << "DATA_CONVERSION: ";
		}
		if (dynamic_cast<const CORBA::FREE_MEM *>(&x)) {
			out << "FREE_MEM: ";
		}
		if (dynamic_cast<const CORBA::IMP_LIMIT *>(&x)) {
			out << "IMP_LIMIT: ";
		}
		if (dynamic_cast<const CORBA::INITIALIZE *>(&x)) {
			out << "INITIALIZE: ";
		}
		if (dynamic_cast<const CORBA::INTERNAL *>(&x)) {
			out << "INTERNAL: ";
		}
		if (dynamic_cast<const CORBA::INTF_REPOS *>(&x)) {
			out << "INTF_REPOS: ";
		}
		if (dynamic_cast<const CORBA::INVALID_TRANSACTION *>(&x)) {
			out << "INVALID_TRANSACTION: ";
		}
		if (dynamic_cast<const CORBA::INV_FLAG *>(&x)) {
			out << "INV_FLAG: ";
		}
		if (dynamic_cast<const CORBA::INV_IDENT *>(&x)) {
			out << "INV_IDENT: ";
		}
		if (dynamic_cast<const CORBA::INV_OBJREF *>(&x)) {
			out << "INV_OBJREF: ";
		}
		if (dynamic_cast<const CORBA::INV_POLICY *>(&x)) {
			out << "INV_POLICY: ";
		}
		if (dynamic_cast<const CORBA::MARSHAL *>(&x)) {
			out << "MARSHAL: ";
		}
		if (dynamic_cast<const CORBA::NO_IMPLEMENT *>(&x)) {
			out << "ON_IMPLEMENT: ";
		}
		if (dynamic_cast<const CORBA::NO_MEMORY *>(&x)) {
			out << "NO_MEMORY: ";
		}
		if (dynamic_cast<const CORBA::NO_PERMISSION *>(&x)) {
			out << "NO_PERMISSION: ";
		}
		if (dynamic_cast<const CORBA::NO_RESOURCES *>(&x)) {
			out << "NO_RESOURCES: ";
		}
		if (dynamic_cast<const CORBA::NO_RESPONSE *>(&x)) {
			out << "NO_RESPONSE: ";
		}
		if (dynamic_cast<const CORBA::OBJECT_NOT_EXIST *>(&x)) {
			out << "OBJECT_NOT_EXISTS: ";
		}
		if (dynamic_cast<const CORBA::OBJ_ADAPTER *>(&x)) {
			out << "OBJ_ADAPTER: ";
		}
		if (dynamic_cast<const CORBA::PERSIST_STORE *>(&x)) {
			out << "PERSIST_STORE: ";
		}
		if (dynamic_cast<const CORBA::TRANSACTION_REQUIRED *>(&x)) {
			out << "TRANSACTION_REQUIRED: ";
		}
		if (dynamic_cast<const CORBA::TRANSACTION_ROLLEDBACK *>(&x)) {
			out << "TRANSACTION_ROLLEDBACK: ";
		}
		if (dynamic_cast<const CORBA::TRANSIENT *>(&x)) {
			out << "TRANSIENT: ";
		}
		if (dynamic_cast<const CORBA::UNKNOWN *>(&x)) {
			out << "BAD_INV_ORDER: ";
		}

		// add the completion status
		out << "completion = ";
		switch (se->completed()) {
		case CORBA::COMPLETED_YES:	out << "YES"; break;
		case CORBA::COMPLETED_NO:	out << "NO"; break;
		case CORBA::COMPLETED_MAYBE:	out << "MAYBE"; break;
		}

		// add the minor
		out << ", minor: " << se->minor();
	}

	return out.str();
}

std::string	ExceptionReporter::operator()(const CORBA::UserException& x) const {
	return "CORBA::UserException";
}

std::string	ExceptionReporter::operator()(const CORBA::Exception& e) const {
	const CORBA::SystemException	*se
		= dynamic_cast<const CORBA::SystemException *>(&e);
	if (NULL != se) {
		return (*this)(*se);
	}
	const CORBA::UserException	*ue
		= dynamic_cast<const CORBA::UserException *>(&e);
	if (NULL != ue) {
		return (*this)(*ue);
	}
	return std::string("unknown exception type");
}

std::string	exception2string(const CORBA::Exception& x) {
	ExceptionReporter	erep;
	return erep(x);
}

} // namespace astro
