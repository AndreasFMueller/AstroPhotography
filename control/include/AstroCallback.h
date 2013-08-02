/*
 * AstroCallback.h -- callback architecture
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroCallback_h
#define _AstroCallback_h

namespace astro {
namespace callback {

class CallbackData {
public:
	/**
	 * \brief constructor takes ownership of the data
	 */
	CallbackData() { }
	virtual	~CallbackData() { }
};
typedef	std::tr1::shared_ptr<CallbackData>	CallbackDataPtr;

class Callback {
public:
	virtual CallbackDataPtr	operator()(CallbackDataPtr data) {
		return CallbackDataPtr();
	}
};

typedef	std::tr1::shared_ptr<Callback>	CallbackPtr;

} // namespace callback
} // namespace astro

#endif /* _AstroCallback_h */
