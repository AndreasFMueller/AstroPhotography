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
		std::string cbdata = demangle_string(&*_data);
		std::string cb = demangle_string(&*callback);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "callback %s on %s",
			cb.c_str(),  cbdata.c_str());
		// check whether the callback actual exists
		if (!callback) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "no callback");
			return CallbackDataPtr(NULL);
		}
		// apply the callback carefully, catching alle exceptions
		try {
			callback->operator()(_data);
		} catch (const std::exception& x) {
			std::string xtype = demangle_string(x);
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
	// make sure we have data
	if (!data) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no data");
		return data;
	}
	CallbackCaller	caller(data);
	try {
		for_each(begin(), end(), caller);
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "failed to call: %s", x.what());
	} catch (...) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "unknown callback failure");
	}
	return data;
}

} // namespace callback
} // namespace astro
