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
		result->size().width(), result->size().height());
	return result;
}

/**
 * \brief Read a file.
 */
ImagePtr	FITSin::read() throw (FITSexception) {
	FITSinfileBase	infile(filename);
	ImagePtr	result;

	// if the file has X/YORGSUBF information, apply it
	ImagePoint	origin;
	if (infile.hasHeader(std::string("XORGSUBF")) &&
		infile.hasHeader(std::string("YORGSUBF"))) {
		int	xorgsubf, yorgsubf;
		xorgsubf = std::stoi(infile.getHeader(std::string("XORGSUBF")));
		yorgsubf = std::stoi(infile.getHeader(std::string("YORGSUBF")));
		origin = ImagePoint(xorgsubf, yorgsubf);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "got origin %s from headers",
			origin.toString().c_str());
	}

	/* images with 3 planes have RGB pixels */
	if (infile.getPlanes() == 3) {
		switch (infile.getImgtype()) {
		case BYTE_IMG:
		case SBYTE_IMG:
			result = do_read<RGB<unsigned char> >(filename);
			break;
		case USHORT_IMG:
		case SHORT_IMG:
			result = do_read<RGB<unsigned short> >(filename);
			break;
		case ULONG_IMG:
		case LONG_IMG:
			result = do_read<RGB<unsigned int> >(filename);
			break;
		case FLOAT_IMG:
			result = do_read<RGB<float> >(filename);
			break;
		case DOUBLE_IMG:
			result = do_read<RGB<double> >(filename);
			break;
		}
		result->setOrigin(origin);
		return result;
	}

	/* multiplane pixel images */
#define	multiplane_read(n)						\
	if (infile.getPlanes() == n) {					\
		switch (infile.getImgtype()) {				\
		case BYTE_IMG:						\
		case SBYTE_IMG:						\
			result = do_read<Multiplane<unsigned char, n> >(filename);\
			break;						\
		case USHORT_IMG:					\
		case SHORT_IMG:						\
			result = do_read<Multiplane<unsigned short, n> >(filename);\
			break;						\
		case ULONG_IMG:						\
		case LONG_IMG:						\
			result = do_read<Multiplane<unsigned int, n> >(filename);\
			break;						\
		case FLOAT_IMG:						\
			result = do_read<Multiplane<float, n> >(filename);\
			break;						\
		case DOUBLE_IMG:					\
			result = do_read<Multiplane<double, n> >(filename);\
			break;						\
		}							\
		result->setOrigin(origin);				\
		return result;						\
	}

	multiplane_read(2);
	multiplane_read(4);
	multiplane_read(5);
	multiplane_read(6);
	multiplane_read(7);

	/* images with 1 plane have primitive data types */
	if (infile.getPlanes() == 1) {
		switch (infile.getImgtype()) {
		case BYTE_IMG:
		case SBYTE_IMG:
			result = do_read<unsigned char>(filename);
			break;
		case USHORT_IMG:
		case SHORT_IMG:
			result = do_read<unsigned short>(filename);
			break;
		case ULONG_IMG:
		case LONG_IMG:
			result = do_read<unsigned int>(filename);
			break;
		case FLOAT_IMG:
			result = do_read<float>(filename);
			break;
		case DOUBLE_IMG:
			result = do_read<double>(filename);
			break;
		}
	}

	// resolve mosaic information, check for the BAYER key:
	if (infile.hasHeader(std::string("BAYER"))) {
		std::string	bayervalue
			= infile.getHeader(std::string("BAYER"));
		bayervalue = bayervalue.substr(0, 4);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "bayervalue: '%s'",
			bayervalue.c_str());
                if (bayervalue == std::string("RGGB")) {
			result->setMosaicType(MosaicType::BAYER_RGGB);
                }
                if (bayervalue == std::string("GRBG")) {
			result->setMosaicType(MosaicType::BAYER_GRBG);
                }
                if (bayervalue == std::string("GBRG")) {
			result->setMosaicType(MosaicType::BAYER_GBRG);
                }
                if (bayervalue == std::string("BGGR")) {
			result->setMosaicType(MosaicType::BAYER_BGGR);
                }
        }
	result->setOrigin(origin);
	return result;
}


} // namespace io
} // namespace astro
