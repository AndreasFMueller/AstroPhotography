/*
 * ImageProgramCallback.cpp -- execute a program, but from a separate thread
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCallback.h>
#include <AstroFormat.h>
#include <includes.h>
#include <thread>

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
 *
 * This method assumes that the image comes with a filename, where the image
 * can also be read from.
 */
static void	imageprogramcallback(callbackargs *cba) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "thread started");
	// we create a shared ptr for cba, which will take ownership of the
	// callback arguments and deallocate them when we exit this function
	std::shared_ptr<callbackargs>	ptr(cba);

	// get the data
	FileImageCallbackData	*icb
		= dynamic_cast<FileImageCallbackData *>(&*(ptr->data));
	if (NULL == icb) {
		debug(LOG_ERR, DEBUG_LOG, 0, "callback called with bad data");
		return;
	}

	// execute the program
	std::string	cmd = stringprintf("%s %s",
				ptr->callback.progname().c_str(),
				icb->filename().c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "command: %s", cmd.c_str());
	int	rc = system(cmd.c_str());
	if (rc) {
		debug(LOG_ERR, DEBUG_LOG, 0, "return value: %d", rc);
		return;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "command executed successfully");
}

/**
 * \brief Execute a program on an image file
 *
 * \param data	callback data of type FileImageCallbackData
 */
CallbackDataPtr	ImageProgramCallback::operator()(CallbackDataPtr data) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "callback called");
	// ensure that we have the right type of data
	FileImageCallbackData	*icb
		= dynamic_cast<FileImageCallbackData *>(&*data);
	if (NULL == icb) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"argument is not ImageCallbackData");
		return CallbackDataPtr();
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got callback data");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "filename: %s", icb->filename().c_str());

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
	try {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "launch thread for program");
		std::thread	thread(imageprogramcallback, cba);
		thread.detach();
	} catch (...) {
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
