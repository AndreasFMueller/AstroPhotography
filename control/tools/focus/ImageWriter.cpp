/**
 * ImageWriter.cpp -- write the images of a focus element
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "ImageWriter.h"
#include <AstroFormat.h>
#include <AstroImage.h>
#include <AstroIO.h>

namespace astro {
namespace focusing {

/**
 * \brief build the filename for the image to write
 *
 * \param fe
 * \param which		wat type of process this is
 */
std::string	ImageWriter::filename(FocusElementCallbackData& fe,
			const std::string& which) const {
	std::string	extension("fits");
	switch (_format) {
	case JPEG:	extension = std::string("jpg");	break;
	case PNG:	extension = std::string("png");	break;
	default: 	break;
	}
	return stringprintf("%s-%s-%08lu.%s", _prefix.c_str(), which.c_str(),
		fe.position(), extension.c_str());
}

/**
 * \brief Write the image
 *
 * \param image		The image to write
 * \param name		filename to use when writing the file
 */
void	ImageWriter::write(ImagePtr image, const std::string& name) const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "writing %s to %s",
		image->info().c_str(), name.c_str());
	switch (_format) {
	case FITS:
		{
			io::FITSout     out(name);
			out.setPrecious(false);
			out.write(image);
		}
		break;
	case JPEG:
		{
			image::JPEG     jpeg;
			jpeg.writeJPEG(image, name);
		}
		break;
	case PNG:
		{
			image::PNG      png;
			png.writePNG(image, name);
		}
		break;
	}
}

/**
 * \brief Process the FocusElement
 *
 * \param fe		The FocusElementCallbackData to process
 */
void	ImageWriter::handle(FocusElementCallbackData& fe) const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "handle focus element");
	try {
		write(fe.raw_image(), filename(fe, "raw"));
	} catch (const std::exception& x) {
		std::string	msg = stringprintf("cannot write raw image: %s",
			x.what());
	}
	try {
		write(fe.processed_image(), filename(fe, "eval"));
	} catch (const std::exception& x) {
		std::string	msg = stringprintf("cannot write raw image: %s",
			x.what());
	}
}

} // namespace focusing
} // namespace astro
