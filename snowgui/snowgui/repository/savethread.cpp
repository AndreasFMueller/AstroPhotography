/*
 * savethread.cpp
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperwil
 */
#include "savethread.h"
#include <AstroDebug.h>
#include <AstroIO.h>
#include <IceConversions.h>
#include <repository.h>

namespace snowgui {

savethread::savethread(QObject *parent) : QThread(parent) {
	_stopProcess = false;
}

savethread::~savethread() {
}

void	savethread::run() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "download thread starts running");
	std::list<std::pair<std::string, int> >::const_iterator	i;
	for (i = _images.begin(); i != _images.end(); i++) {
		std::string	reponame = i->first;
		int	imageid = i->second;
		if (_stopProcess) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "process abort request");
			emit downloadAborted();
			return;
		}
		QString	r(i->first.c_str());
		downloadstatus	s(r, i->second);
		emit sendStatus(s);
		// now download image
		debug(LOG_DEBUG, DEBUG_LOG, 0, "image %d from repo %s", 
			imageid, reponame.c_str());

		// get image
		snowstar::RepositoryPrx repository
			= _repositories->get(reponame);
		snowstar::ImageInfo	info = repository->getInfo(imageid);
		std::string	filename = astro::stringprintf("%s/%s",
			_directory.c_str(), info.filename.c_str());
		snowstar::ImageFile	image = repository->getImage(imageid);
		astro::image::ImagePtr	imageptr = snowstar::convertfile(image);

		// get the file name from the image
	
                debug(LOG_DEBUG, DEBUG_LOG, 0, "filename: %s",
			filename.c_str());

		// write image
                astro::io::FITSout      out(filename);
                if (out.exists()) {
                        out.unlink();
                }
                try {
                        out.write(imageptr);
                } catch (astro::io::FITSexception& x) {
			_errormsg = astro::stringprintf("cannot write image "
				"%d to %s: %s", imageid, filename.c_str(),
				x.what());
			emit downloadAborted();
			return;
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "download complete");
	emit downloadComplete();
}

void	savethread::set(const std::string& directory,
		snowstar::RepositoriesPrx repositories,
		const std::list<std::pair<std::string, int> >& images) {
	_stopProcess = false;
	_directory = directory;
	_repositories = repositories;
	_images = images;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got %d images to process",
		_images.size());
	start();
}

void	savethread::stopProcess() {
	_stopProcess = true;
}

const std::string&	savethread::errormsg() const {
	return _errormsg;
}

} // namespace snowgui

