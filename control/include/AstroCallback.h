/*
 * AstroCallback.h -- callback architecture
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroCallback_h
#define _AstroCallback_h

#include <AstroImage.h>

namespace astro {
namespace callback {

/**
 * \brief Argument and return value class for callbacks
 *
 * In oder for all callbacks to have the same signature, we need
 * this common (empty) base class for all data that is handed in
 * to callbacks or returned by callbacks.
 */
class CallbackData {
public:
	CallbackData() { }
	virtual	~CallbackData() { }
};
typedef	std::tr1::shared_ptr<CallbackData>	CallbackDataPtr;

/**
 * \brief A Callback is a functor that processes CallbackData
 *
 * The base class functor just is the identity operator.
 */
class Callback {
public:
	virtual CallbackDataPtr	operator()(CallbackDataPtr data) {
		return CallbackDataPtr();
	}
};

typedef	std::tr1::shared_ptr<Callback>	CallbackPtr;

/**
 * \brief Per image program execution callback argument
 */
class ImageCallbackData : public CallbackData {
	std::string	_filename;
	astro::image::ImagePtr	_image;
public:
	ImageCallbackData(const std::string& filename, astro::image::ImagePtr image)
		: _filename(filename), _image(image) { }
	const std::string&	filename() const { return _filename; }
	astro::image::ImagePtr	image() const { return _image; }
};

/**
 * \brief Program execution callback for images
 */
class ImageProgramCallback : public Callback {
	std::string	_progname;
	bool	_wait;
public:
	ImageProgramCallback(const std::string& progname, bool wait = false)
		: _progname(progname), _wait(wait) { }
	std::string	progname() const { return _progname; }
	bool	wait() const { return _wait; }
	void	wait(bool wait) { _wait = wait; }
	CallbackDataPtr	operator()(CallbackDataPtr data);
};


} // namespace callback
} // namespace astro

#endif /* _AstroCallback_h */
