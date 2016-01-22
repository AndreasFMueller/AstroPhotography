/*
 * CallbackSet.cpp
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCallback.h>
#include <algorithm>
#include <AstroUtils.h>
#include <AstroFormat.h>

namespace astro {
namespace callback {

/**
 * \brief A functor class that applies a functions to a common argument
 */
class CallbackCaller {
	CallbackDataPtr	_data;
public:
	CallbackCaller(CallbackDataPtr data) : _data(data) { }

	CallbackDataPtr	operator()(CallbackPtr callback) {
		// get type names for callback and data
		std::string cbdata = demangle(typeid(&*_data).name());
		std::string cb = demangle(typeid(&*callback).name());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "callback %s on %s",
			cb.c_str(),  cbdata.c_str());
		// apply the callback carefully, catching alle exceptions
		try {
			callback->operator()(_data);
		} catch (const std::exception& x) {
			std::string xtype = demangle(typeid(x).name());
			std::string	cause = stringprintf(
				"callback %s failed on %s: %s, cause: %s",
				cb.c_str(), cbdata.c_str(), xtype.c_str(),
				x.what());
		} catch (...) {
			debug(LOG_ERR, DEBUG_LOG, 0,
				"callback %s failed on %s: unknown error",
				cb.c_str(), cbdata.c_str());
		}
		// return the data
		return _data;
	}
};

/**
 * \brief let all callbacks operate on data
 */
CallbackDataPtr	CallbackSet::operator()(CallbackDataPtr data) {
	CallbackCaller	caller(data);
	for_each(begin(), end(), caller);
	return data;
}

} // namespace callback
} // namespace astro
