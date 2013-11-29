/*
 * imagecommand.cpp -- image command
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <guidecli.h>
#include <imagecommand.h>
#include <AstroDebug.h>
#include <Images.h>

namespace astro {
namespace cli {

void	imagecommand::assign(const std::string& imageid,
		const std::vector<std::string>& arguments) {
	Images	images;
	images.assign(imageid, arguments);
}

void	imagecommand::release(const std::string& imageid) {
	Images	images;
	images.release(imageid);
}

std::ostream&	operator<<(std::ostream& out, const Astro::ImagePoint& point) {
	out << "(" << point.x << "," << point.x << ")";
	return out;
}

std::ostream&	operator<<(std::ostream& out, const Astro::ImageSize& size) {
	out << size.width << "x" << size.height;
	return out;
}

void	imagecommand::info(ImageWrapper& image) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image info");
	std::cout << "size:           " << image->size() << std::endl;
	std::cout << "origin:         " << image->origin() << std::endl;
	std::cout << "bytes/pixel:    " << image->bytesPerPixel() << std::endl;
	std::cout << "bytes/value:    " << image->bytesPerValue() << std::endl;
	std::cout << "planes:         " << image->planes() << std::endl;
}

void	imagecommand::save(ImageWrapper& image, const std::string& filename) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "save image to %s", filename.c_str());
	Astro::Image::ImageFile_var	imagefile = image->file();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got %s bytes", imagefile->length());
}

void	imagecommand::remove(ImageWrapper& image) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "remove the image");
	image->remove();
}

void	imagecommand::operator()(const std::string& command,
		const std::vector<std::string>& arguments) {
	if (arguments.size() < 2) {
		throw command_error("image command requires arguments");
	}
	std::string	imageid = arguments[0];
	std::string	subcommand = arguments[1];

	if ("assign" == subcommand) {
		assign(imageid, arguments);
		return;
	}

	if ("release" == subcommand) {
		release(imageid);
		return;
	}

	Images	images;
	ImageWrapper	image = images.byname(imageid);

	if ("info" == subcommand) {
		info(image);
		return;
	}

	if ("save" == subcommand) {
		if (arguments.size() < 3) {
			throw std::runtime_error("filename argument missing");
		}
		save(image, arguments[2]);
		return;
	}

	if ("remove" == subcommand) {
		remove(image);
		return;
	}

}

std::string	imagecommand::summary() const {
	return std::string("access images");
}

std::string	imagecommand::help() const {
	return std::string(
	"SYNOPSIS\n"
	"\n"
	"\timage <id> assign <filename>\n"
	"\timage <id> info\n"
	"\timage <id> save <localfilename>\n"
	"\timage <id> release\n"
	"\timage <id> remove\n"
	"\n"
	"DESCRIPTION\n"
	"\n"
	"Access image files on the server.\n"
	);
}

} // namespace cli
} // namespace astro
