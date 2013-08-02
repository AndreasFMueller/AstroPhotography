/*
 * FITSout.cpp -- output generic images to FITS files
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroIO.h>

using namespace astro::image;

namespace astro {
namespace io {

FITSout::FITSout(const std::string& _filename) : filename(_filename) {
}

/**
 * \brief Write a file with a given pixel type.
 *
 * \param filename	Name of the FITS file to write
 * \param image		Image to write
 */
template<typename P>
static bool	do_write(const std::string& filename, const ImagePtr& image)
		throw (FITSexception) {
	Image<P>	*im = dynamic_cast<Image<P> *>(&*image);
debug(LOG_DEBUG, DEBUG_LOG, 0, "%p", im);
	if (NULL == im) {
		return false;
	}
	FITSoutfile<P>	outfile(filename);
	outfile.write(*im);
	return true;
}

/**
 * \brief Write the image to the file
 *
 * \param image		The image to write to the file
 */
void	FITSout::write(const ImagePtr& image) throw (FITSexception) {
	// test the various types, and call the do_write template 
	if (do_write<unsigned char>         (filename, image)) { return; }
	if (do_write<unsigned short>        (filename, image)) { return; }
	if (do_write<unsigned int>          (filename, image)) { return; }
	if (do_write<unsigned long>         (filename, image)) { return; }
	if (do_write<float>                 (filename, image)) { return; }
	if (do_write<double>                (filename, image)) { return; }
	if (do_write<RGB<unsigned char> >   (filename, image)) { return; }
	if (do_write<RGB<unsigned short> >  (filename, image)) { return; }
	if (do_write<RGB<unsigned int> >    (filename, image)) { return; }
	if (do_write<RGB<unsigned long> >   (filename, image)) { return; }
	if (do_write<RGB<float> >           (filename, image)) { return; }
	if (do_write<RGB<double> >          (filename, image)) { return; }
	if (do_write<YUYV<unsigned char> >  (filename, image)) { return; }
	if (do_write<YUYV<unsigned short> > (filename, image)) { return; }
	if (do_write<YUYV<unsigned int> >   (filename, image)) { return; }
	if (do_write<YUYV<unsigned long> >  (filename, image)) { return; }
	if (do_write<YUYV<float> >          (filename, image)) { return; }
	if (do_write<YUYV<double> >         (filename, image)) { return; }
	throw FITSexception("cannot locate FITSoutputfile for pixel type");
}

} // namespace io
} // namespace astro
