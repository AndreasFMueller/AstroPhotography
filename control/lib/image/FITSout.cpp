/*
 * FITSout.cpp -- output generic images to FITS files
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroIO.h>
#include <includes.h>

using namespace astro::image;

namespace astro {
namespace io {

FITSout::FITSout(const std::string& _filename) : filename(_filename) {
	_precious = true;
}

/**
 * \brief Find out whether a file exists
 */
bool	FITSout::exists() const {
	struct stat	sb;
	int	rc = stat(filename.c_str(), &sb);
	if (rc == 0) {
		return true;
	}
	return false;
}

/**
 * \brief Unlink the file if it exists
 */
void	FITSout::unlink() {
	::unlink(filename.c_str());
}

/**
 * \brief Write a file with a given pixel type.
 *
 * \param filename	Name of the FITS file to write
 * \param image		Image to write
 */
template<typename P>
static bool	do_write(const std::string& filename, const ImagePtr image,
			const bool precious = true)
		throw (FITSexception) {
	Image<P>	*im = dynamic_cast<Image<P> *>(&*image);
	if (NULL == im) {
		return false;
	}
	FITSoutfile<P>	outfile(filename);
	outfile.setPrecious(precious);
	outfile.write(*im);
	return true;
}

/**
 * \brief Write the image to the file
 *
 * \param image		The image to write to the file
 */
void	FITSout::write(const ImagePtr image) throw (FITSexception) {
	// test the various types, and call the do_write template 
#define	do_write_typed(type)						\
	if (do_write<type >(filename, image, precious())) {		\
		return;							\
	}
	do_write_typed(unsigned char)
	do_write_typed(unsigned short)
	do_write_typed(unsigned int)
	do_write_typed(unsigned long)
	do_write_typed(float)
	do_write_typed(double)

	do_write_typed(RGB<unsigned char>)
	do_write_typed(RGB<unsigned short>)
	do_write_typed(RGB<unsigned int>)
	do_write_typed(RGB<unsigned long>)
	do_write_typed(RGB<float>)
	do_write_typed(RGB<double>)

	do_write_typed(YUYV<unsigned char>)
	do_write_typed(YUYV<unsigned short>)
	do_write_typed(YUYV<unsigned int>)
	do_write_typed(YUYV<unsigned long>)
	do_write_typed(YUYV<float>)
	do_write_typed(YUYV<double>)

#define	do_write_multi(type, n)						\
	if (do_write<Multiplane<type, n> >(filename, image, precious())) {\
		return;							\
	}
	do_write_multi(unsigned char,  1)
	do_write_multi(unsigned short, 1)
	do_write_multi(unsigned int,   1)
	do_write_multi(unsigned long,  1)
	do_write_multi(float,          1)
	do_write_multi(double,         1)

	do_write_multi(unsigned char,  2)
	do_write_multi(unsigned short, 2)
	do_write_multi(unsigned int,   2)
	do_write_multi(unsigned long,  2)
	do_write_multi(float,          2)
	do_write_multi(double,         2)

	do_write_multi(unsigned char,  3)
	do_write_multi(unsigned short, 3)
	do_write_multi(unsigned int,   3)
	do_write_multi(unsigned long,  3)
	do_write_multi(float,          3)
	do_write_multi(double,         3)

	do_write_multi(unsigned char,  4)
	do_write_multi(unsigned short, 4)
	do_write_multi(unsigned int,   4)
	do_write_multi(unsigned long,  4)
	do_write_multi(float,          4)
	do_write_multi(double,         4)

	do_write_multi(unsigned char,  5)
	do_write_multi(unsigned short, 5)
	do_write_multi(unsigned int,   5)
	do_write_multi(unsigned long,  5)
	do_write_multi(float,          5)
	do_write_multi(double,         5)

	do_write_multi(unsigned char,  6)
	do_write_multi(unsigned short, 6)
	do_write_multi(unsigned int,   6)
	do_write_multi(unsigned long,  6)
	do_write_multi(float,          6)
	do_write_multi(double,         6)

	do_write_multi(unsigned char,  7)
	do_write_multi(unsigned short, 7)
	do_write_multi(unsigned int,   7)
	do_write_multi(unsigned long,  7)
	do_write_multi(float,          7)
	do_write_multi(double,         7)

	throw FITSexception("cannot locate FITSoutputfile for pixel type");
}

} // namespace io
} // namespace astro
