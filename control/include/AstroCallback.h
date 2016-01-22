/*
 * AstroCallback.h -- callback architecture
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroCallback_h
#define _AstroCallback_h

#include <AstroImage.h>
#include <ImageDirectory.h>
#include <AstroUtils.h>
#include <set>

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
typedef	std::shared_ptr<CallbackData>	CallbackDataPtr;

/**
 * \brief Template envelope to turn any object into a callback data
 */
template<typename payload>
class CallbackDataEnvelope : public CallbackData {
	payload	_data;
public:
	CallbackDataEnvelope(const payload& data) : _data(data) { }
	const payload&	data() const { return _data; }
	operator	payload() const { return _data; }
};

/**
 * \brief A Callback is a functor that processes CallbackData
 *
 * The base class functor just is the identity operator.
 */
class Callback {
public:
	virtual CallbackDataPtr	operator()(CallbackDataPtr data) = 0;
};

typedef	std::shared_ptr<Callback>	CallbackPtr;

/**
 * \brief a class to fan out callbacks to many receivers
 */
class CallbackSet : public std::set<CallbackPtr> {
public:
	CallbackDataPtr	operator()(CallbackDataPtr data);
};

/**
 * \brief Image callback argument
 */
class ImageCallbackData : public CallbackData {
	astro::image::ImagePtr	_image;
public:
	ImageCallbackData(astro::image::ImagePtr image) : _image(image) { }
	astro::image::ImagePtr	image() const { return _image; }
};

/**
 * \brief Image callback including filename
 *
 * This is used in places where a recommended file name is also needed,
 * like when an external program must be called. The image contained in
 * the callback data is expected to be stored in the file.
 */
class FileImageCallbackData : public ImageCallbackData {
	std::string	_filename;
	astro::image::ImagePtr	_image;
public:
	FileImageCallbackData(const std::string& filename,
		astro::image::ImagePtr image)
		: ImageCallbackData(image), _filename(filename) { }
	const std::string&	filename() const { return _filename; }
};

/**
 * \brief Program execution callback for images
 *
 * This callback executes an external program, the first argument given to
 * the program is the name of a temporary image file.
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

/**
 * \brief Callback to save an image in an ImageDirectory
 *
 * This callback expects callback data of type ImageCallbackData, which 
 * includes a filename together with the image, and adds the image to
 * the ImageDirectory.
 */
class SaveImageCallback : public Callback {
public:
	SaveImageCallback(const std::string& basedir) {
		image::ImageDirectory::basedir(basedir);
	}
	CallbackDataPtr	operator()(CallbackDataPtr data);
};

} // namespace callback
} // namespace astro

#endif /* _AstroCallback_h */
