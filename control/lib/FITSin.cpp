/*
 * FITSin.cpp -- read a FITS file into a generic ImagePtr
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroIO.h>
#include <AstroDebug.h>

namespace astro {
namespace io {

/**
 * \brief Construct a generic FITS reader
 *
 * \brief filename 	Name of the file to read
 */
FITSin::FITSin(const std::string& _filename) : filename(_filename) {
}

/**
 * \brief Do the dirty work of the read
 *
 * We already have classes to read images of a certain type, so this
 * function uses such a class, reads the image pointer from it, wrap it
 * in a new ImagePtr and reset the old type specific pointer.
 */
template<typename P>
static ImagePtr	do_read(const std::string& filename) throw (FITSexception) {
	FITSinfile<P>	reader(filename);
	Image<P>	*image = reader.read();
	ImagePtr	result(image);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "result is an %d x %d image",
		result->size.width, result->size.height);
	return result;
}

/**
 * \brief Read a file.
 */
ImagePtr	FITSin::read() throw (FITSexception) {
	FITSinfileBase	infile(filename);

	/* images with 1 plane have primitive data types */
	if (infile.getPlanes() == 1) {
		switch (infile.getImgtype()) {
		case BYTE_IMG:
		case SBYTE_IMG:
			return do_read<unsigned char>(filename);
		case USHORT_IMG:
		case SHORT_IMG:
			return do_read<unsigned short>(filename);
		case ULONG_IMG:
		case LONG_IMG:
			return do_read<unsigned int>(filename);
		case FLOAT_IMG:
			return do_read<float>(filename);
		case DOUBLE_IMG:
			return do_read<double>(filename);
		}
	}

	/* images with 3 planes have RGB pixels */
	if (infile.getPlanes() == 3) {
		switch (infile.getImgtype()) {
		case BYTE_IMG:
		case SBYTE_IMG:
			return do_read<RGB<unsigned char> >(filename);
		case USHORT_IMG:
		case SHORT_IMG:
			return do_read<RGB<unsigned short> >(filename);
		case ULONG_IMG:
		case LONG_IMG:
			return do_read<RGB<unsigned int> >(filename);
		case FLOAT_IMG:
			return do_read<RGB<float> >(filename);
		case DOUBLE_IMG:
			return do_read<RGB<double> >(filename);
		}
	}

	return ImagePtr();
}


} // namespace io
} // namespace astro
