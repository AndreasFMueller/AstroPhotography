/*
 * ImageProgramCallback.cpp -- execute a program, but from a separate thread
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCallback.h>
#include <AstroFormat.h>
#include <includes.h>

using namespace astro::image;

namespace astro {
namespace callback {

/**
 * \brief auxiliary class for parameter passing to the callback main function
 */
class callbackargs {
public:
	ImageProgramCallback&	callback;
	CallbackDataPtr	data;
	callbackargs(ImageProgramCallback& _callback, CallbackDataPtr _data)
		: callback(_callback), data(_data) { }
};

/**
 * \brief main function for the separate thread executes the image program
 */
static void	*imageprogramcallback(void *data) {
	// resolve the argument
	callbackargs	*cba = (callbackargs *)data;

	// we create a shared ptr for cba, which will take ownership of the
	// callback arguments and deallocate them when we exit this function
	std::tr1::shared_ptr<callbackargs>	ptr(cba);

	// get the data
	ImageCallbackData	*icb
		= dynamic_cast<ImageCallbackData *>(&*(ptr->data));
	if (NULL == icb) {
		debug(LOG_ERR, DEBUG_LOG, 0, "callback called with bad data");
		return NULL;
	}

	// execute the program
	std::string	cmd = stringprintf("%s %s",
				ptr->callback.progname().c_str(),
				icb->filename().c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "command: %s", cmd.c_str());
	int	rc = system(cmd.c_str());
	if (rc) {
		debug(LOG_ERR, DEBUG_LOG, 0, "return value: %d", rc);
		return NULL;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "command executed successfully");

	// that's it
	return NULL;
}

/**
 * \brief Execute a program on an image file
 */
CallbackDataPtr	ImageProgramCallback::operator()(CallbackDataPtr data) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "callback called");
	// ensure that we have the right type of data
	ImageCallbackData	*icb
		= dynamic_cast<ImageCallbackData *>(&*data);
	if (NULL == icb) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"argument is not ImageCallbackData");
		return CallbackDataPtr();
	}
	// if we should wait for the completion of the program, we just issue
	// the system call
	if (_wait) {
		std::string	cmd = stringprintf("%s %s", _progname.c_str(),
			icb->filename().c_str());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "executing command %s",
			cmd.c_str());
		int	rc = system(cmd.c_str());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "return value: %d", rc);
		return data;
	}


	// if we get to this point, then it is expected that we perform the
	// execution of the program from a separate thread, so that several
	// threads can be running on different images. We have to ensure that
	// the thread function obtains a copy of the argument data, and
	// properly deallocates it when it exits.
	callbackargs	*cba = new callbackargs(*this, data);

	// create the thread
	pthread_t	thread;
	pthread_attr_t	attr;
	pthread_attr_init(&attr);
	int	rc = pthread_create(&thread, &attr, imageprogramcallback, cba);
	if (rc) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot start program: %s",
			strerror(errno));
		// in this case we have to release the callback arguments
		// ourselves
		delete cba;

		// we return an empty argument to indicate the problem
		debug(LOG_DEBUG, DEBUG_LOG, 0, "callback error return");
		return CallbackDataPtr();
	}

	// successfully started callback, return the original data
	debug(LOG_DEBUG, DEBUG_LOG, 0, "callback return");
	return data;
}

} // namespace callback
} // namespace astro
