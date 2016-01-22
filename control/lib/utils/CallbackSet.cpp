/*
 * CallbackSet.cpp
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCallback.h>
#include <algorithm>

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
		return callback->operator()(_data);
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
