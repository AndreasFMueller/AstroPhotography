/*
 * ImageRetrieverThread.cpp -- retrieves an image from a ccdcontrollerwidget
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <ccdcontrollerwidget.h>
#include <AstroDebug.h>

namespace snowgui {

ImageRetrieverThread::ImageRetrieverThread(ccdcontrollerwidget *c)
	: QThread(NULL), _ccdcontrollerwidget(c) {
}

ImageRetrieverThread::~ImageRetrieverThread() {
}

void	ImageRetrieverThread::run() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start retrieving the image");
	_ccdcontrollerwidget->retrieveImageWork();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image retrieved");
}

} // namespace snowgui
