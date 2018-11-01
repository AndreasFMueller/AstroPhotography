/* * downloadthread.cpp -- implementation of the download thread
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "downloadthread.h"
#include <image.h>
#include <AstroDebug.h>
#include <AstroImage.h>
#include <AstroIO.h>
#include <repository.h>
#include <IceConversions.h>

namespace snowgui {

/**
 * \brief Create a new download thread
 */
downloadthread::downloadthread(QObject *parent)
	: QThread(parent) {
	_stopProcess = false;
}

/**
 * \brief destroy the thread
 */
downloadthread::~downloadthread() {
}

/**
 * \brief Slot to cancel the download process
 */
void	downloadthread::stopProcess() {
	_stopProcess = true;
}

/**
 * \brief Main function for downloading a bunch of files
 */
void	downloadthread::run() {
	if (_filelist.size() == 0) {
		return;
	}
	std::list<downloaditem>::iterator	fileptr = _filelist.begin();
	int	counter = 0;
	while (fileptr != _filelist.end()) {
		// make thread is still running
		if (_stopProcess) {
			// handle termination
			debug(LOG_DEBUG, DEBUG_LOG, 0, "process abort request");
			_filelist.clear();
			emit downloadAborted();
			return;
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "processing item %d", ++counter);

		// send that current item
		downloaditem	item = *fileptr;
		emit sendStatus(item);

		// get the image
		debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve image %d from repo %s",
			item.imageid(), item.reponame().c_str());
		snowstar::RepositoryPrx	repository
			= _repositories->get(item.reponame());
		snowstar::ImageBuffer	image
			= repository->getImage(item.imageid(),
				snowstar::ImageEncodingFITS);
		astro::image::ImagePtr	imageptr = snowstar::convertimage(image);

		// get the file name
		std::string	filename = astro::stringprintf("%s/%s",
			item.targetdirectory().c_str(),
			item.targetfile().c_str());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "target file path: %s",
			filename.c_str());

		// write the image
		astro::io::FITSout	out(filename);
		if (out.exists()) {
			out.unlink();
		}
		try {
			out.write(imageptr);
		} catch (astro::io::FITSexception& x) {
			_errormsg = astro::stringprintf("cannot write image "
				"%d to %s: %s", item.imageid(),
				filename.c_str(), x.what());
			emit downloadAborted();
			return;
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "file %s completed",
			filename.c_str());

		fileptr++;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "download complete");
	emit downloadComplete();
}

/**
 * \brief set download parameters and start download
 */
void	downloadthread::set(snowstar::RepositoriesPrx repositories,
		const downloadlist& filelist) {
	// don't accept anything if the thread is already running
	if (isRunning()) {
		return;
	}
	_repositories = repositories;
	_filelist = filelist;
	start();
}

} // namespace snowgui
