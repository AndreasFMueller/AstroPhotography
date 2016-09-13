/*
 * TaskI.cpp -- task servant implementaiton
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <TaskI.h>
#include <AstroTask.h>
#include <Ice/ObjectAdapter.h>
#include <Ice/Communicator.h>
#include <TaskQueueI.h>
#include <TaskTable.h>
#include <ImageI.h>
#include <IceConversions.h>
#include <AstroUtils.h>
#include <AstroConfig.h>
#include <ImageDirectory.h>

namespace snowstar {

TaskI::TaskI(astro::persistence::Database& _database, long _queueid)
	: database(_database), queueid(_queueid) {
}

TaskI::~TaskI() {
}

astro::task::TaskQueueEntry	TaskI::entry() {
	astro::task::TaskTable	tasktable(database);
	if (!tasktable.exists(queueid)) {
		throw NotFound("task does not exist");
	}
	astro::task::TaskQueueEntry	result = tasktable.byid(queueid);
	return result;
}

TaskState	TaskI::state(const Ice::Current& /* current */) {
	return snowstar::convert(entry().state());
}

TaskParameters	TaskI::parameters(const Ice::Current& /* current */) {
	return snowstar::convert(entry().parameters());
}

TaskInfo	TaskI::info(const Ice::Current& /* current */) {
	return snowstar::convert(entry().info());
}

std::string	TaskI::imagename(const Ice::Current& /* current */) {
	return entry().filename();
}

ImagePrx	TaskI::getImage(const Ice::Current& /* current */) {
	// find out whether the file exists
	std::string	filename = entry().filename();
	astro::image::ImageDatabaseDirectory	imagedir;
	if (!imagedir.isFile(filename)) {
		NotFound	exception;
		exception.cause = astro::stringprintf(
			"image %s not found", filename.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", exception.cause.c_str());
		throw exception;
	}
	//return ImageI::createProxy(filename, current);
	return NULL;
}

int	TaskI::imageToRepo(const std::string& reponame,
		const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "imageToRepo(%s)", reponame.c_str());
	// first make sure the file really exist
	std::string	filename = entry().filename();
	astro::image::ImageDatabaseDirectory	imagedir;
	if (!imagedir.isFile(filename)) {
		NotFound	exception;
		exception.cause = astro::stringprintf(
			"image %s not found", filename.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", exception.cause.c_str());
		throw exception;
	}
	astro::image::ImagePtr	image = imagedir.getImagePtr(filename);

	// now get the named image repository configuration
	astro::config::ConfigurationPtr	configuration
		= astro::config::Configuration::get();
	astro::config::ImageRepoConfigurationPtr	imagerepos
		= astro::config::ImageRepoConfiguration::get(configuration);
	if (!imagerepos->exists(reponame)) {
		NotFound	exception;
		exception.cause
			= astro::stringprintf("image repo '%s' not found",
				reponame.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", exception.cause.c_str());
		throw exception;
	}
	astro::project::ImageRepoPtr	repo = imagerepos->repo(reponame);

	// save the image
	return repo->save(image);
}

} // namespace snowstar
