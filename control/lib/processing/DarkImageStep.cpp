/*
 * DarkImageStep.cpp -- DarkImageStep implementation
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperwil
 */
#include <AstroProcess.h>
#include <AstroCalibration.h>
#include <AstroIO.h>
#include <sstream>

namespace astro {
namespace process {

/**
 * \brief Construct a new dark image
 *
 * \param parent	the parent paths
 * \param purpose	the purpose of the dark step
 */
DarkImageStep::DarkImageStep(NodePaths& parent,
	camera::Exposure::purpose_t purpose)
	: ImageStep(parent), _purpose(purpose) {
	_badpixellimit = 3;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "nodepaths: %s",
		NodePaths::info().c_str());
}

/**
 * \brief Perform the dark image computation
 */
ProcessingStep::state	DarkImageStep::do_work() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get precursor images");
	// construct the sequence of images
	ImageSequence	images = precursorimages();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d images", images.size());
	if (images.size() == 0) {
		return ProcessingStep::failed;
	}

	// actually produce the dark frame
	astro::calibration::DarkFrameFactory	dff(_badpixellimit);
	_image = dff(images);

	// set the purpose
	if (camera::Exposure::bias == purpose()) {
		_image->setMetadata(io::FITSKeywords::meta(
			std::string("PURPOSE"), std::string("bias")));
	}

	// remember that 
	time_t	now;
	time(&now);
	ProcessingStep::when(now);

	// return completion status
	return ProcessingStep::complete;
}

std::string	DarkImageStep::what() const {
	std::ostringstream	out;
	out << "build dark from " << precursors().size() << " images:";
	std::for_each(precursors().begin(), precursors().end(),
		[&out](int pid) mutable {
			out << " " << pid;
		}
	);
	return out.str();
}

} // namespace process
} // namespace astro
