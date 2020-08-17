/*
 * JPEG.cpp -- class to write FITS images as JPEG
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroImage.h>
#include <cstdio>
#include <jpeglib.h>
#include <setjmp.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <includes.h>

namespace astro {
namespace image {

JPEG::JPEG() : _quality(80) {
}

/**
 * \brief Auxiliary function to find out whether a filename is JPEG
 *
 * \param filename	filename to investigate
 */
bool	JPEG::isjpegfilename(const std::string& filename) {
	if (filename.size() > 4) {
		if (filename.substr(filename.size() - 4)
			== std::string(".jpg")) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "filename %s is JPG",
				filename.c_str());
			return true;
		}
	}
	if (filename.size() > 5) {
		if (filename.substr(filename.size() - 5)
			== std::string(".jpeg")) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "filename %s is JPG",
				filename.c_str());
			return true;
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s is not JPG filename",
		filename.c_str());
	return false;
}

/**
 * \brief Write a color image as JPEG to a buffer
 *
 * \param colorimage	the color image to write
 * \param buffer	the buffer containing the JPEG data
 * \param buffersize	the buffer size
 */
size_t  JPEG::writeJPEG(const ConstImageAdapter<RGB<unsigned char> >& colorimage,
		void **buffer, size_t *buffersize) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "write RGB image to buffer");
	*buffer = NULL;
	*buffersize = 0;

	// prepare a pixel buffer
	int	w = colorimage.getSize().width();
	int	h = colorimage.getSize().height();
	unsigned char	pixelline[3 * w];
	unsigned char	*row_pointer[1];
	row_pointer[0] = pixelline;

	// prepare JPEG stuff
	struct jpeg_compress_struct	cinfo;
	struct jpeg_error_mgr	jerr;
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);

	// set the output buffer
	unsigned char	*jbuffer = NULL;
	unsigned long	jbuffersize = 0;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "setting up buffer");
	jpeg_mem_dest(&cinfo, &jbuffer, &jbuffersize);

	// prepare the defaults
	cinfo.image_width = colorimage.getSize().width();
	cinfo.image_height = colorimage.getSize().height();
	cinfo.input_components = 3;
	cinfo.in_color_space = JCS_RGB;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image size: %d x %d",
		cinfo.image_width, cinfo.image_height);

	// set default parameters
	jpeg_set_defaults(&cinfo);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image size: %d x %d",
		cinfo.image_width, cinfo.image_height);

	// set quality
	jpeg_set_quality(&cinfo, _quality, TRUE);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "quality set to %d", _quality);

	// start the compression
	jpeg_start_compress(&cinfo, TRUE);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "compress started, %d lines",
		cinfo.image_height);
	while (cinfo.next_scanline < cinfo.image_height) {
		int	y = h - 1 - cinfo.next_scanline;
		for (int x = 0; x < w; x++) {
			RGB<unsigned char>	p = colorimage.pixel(x, y);
			pixelline[3 * x    ] = p.R;
			pixelline[3 * x + 1] = p.G;
			pixelline[3 * x + 2] = p.B;
		}
		jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "all data written: %d bytes",
		jbuffersize);

	// close the compression
	jpeg_finish_compress(&cinfo);

	// destroy the data structures that were needed to compress
	jpeg_destroy_compress(&cinfo);

	// copy the buffer data
	*buffer = jbuffer;
	*buffersize = jbuffersize;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "wrote %d bytes", jbuffersize);

	// return the buffersize
	return *buffersize;
}

/**
 * \brief Write a color image as JPEG to a file
 *
 * \param colorimage	Color image to convert to JPEG
 * \param filename	name of the JPEG file
 */
size_t  JPEG::writeJPEG(const ConstImageAdapter<RGB<unsigned char> >& colorimage,
		const std::string& filename) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "write %s image to %s",
		colorimage.getSize().toString().c_str(),
		filename.c_str());

	// prepare a pixel buffer
	int	w = colorimage.getSize().width();
	int	h = colorimage.getSize().height();
	unsigned char	pixelline[3 * w];
	JSAMPROW	row_pointer[1];
	row_pointer[0] = pixelline;

	// prepare JPEG stuff
	struct jpeg_compress_struct	cinfo;
	struct jpeg_error_mgr	jerr;
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);

	// set the output buffer
	FILE	*outfile;
	if ((outfile = fopen(filename.c_str(), "wb")) == NULL) {
		std::string	msg = stringprintf("cannot open file %s: %s",
			filename.c_str(), strerror(errno));
		return 0;
	}
	jpeg_stdio_dest(&cinfo, outfile);

	// prepare the defaults
	cinfo.image_width = colorimage.getSize().width();
	cinfo.image_height = colorimage.getSize().height();
	cinfo.input_components = 3;
	cinfo.in_color_space = JCS_RGB;

	// set default parameters
	jpeg_set_defaults(&cinfo);

	// set quality
	jpeg_set_quality(&cinfo, _quality, TRUE);

	// start the compression
	jpeg_start_compress(&cinfo, TRUE);
	while (cinfo.next_scanline < cinfo.image_height) {
		int	y = h - 1 - cinfo.next_scanline;
		for (int x = 0; x < w; x++) {
			RGB<unsigned char>	p = colorimage.pixel(x, y);
			pixelline[3 * x    ] = p.R;
			pixelline[3 * x + 1] = p.G;
			pixelline[3 * x + 2] = p.B;
		}
		jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}

	// close the compression
	jpeg_finish_compress(&cinfo);

	// destroy the data structures that were needed to compress
	jpeg_destroy_compress(&cinfo);

	// copy the buffer data
	fclose(outfile);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "file %s closed", filename.c_str());

	// get the filesize
	struct stat	sb;
	if (stat(filename.c_str(), &sb) < 0) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot stat %s: %s", 
			filename.c_str(), strerror(errno));
		return 0;
	}

	// return the buffersize
	return sb.st_size;
}

/**
 * \brief Write a mono image as a JPEG data buffer
 *
 * This method allocates the buffer containing the JPEG data and returns
 * it in the buffer/buffersize variables
 *
 * \param monomimage	monochrome image to write to JPEG
 * \param buffer	data pointer for buffer
 * \param buffersize	buffer size
 */
size_t	JPEG::writeJPEG(const ConstImageAdapter<unsigned char>& monoimage,
		void **buffer, size_t *buffersize) {
	*buffer = NULL;
	*buffersize = 0;

	// prepare a pixel buffer
	int	w = monoimage.getSize().width();
	int	h = monoimage.getSize().height();
	unsigned char	pixelline[w];
	unsigned char	*row_pointer[1];
	row_pointer[0] = pixelline;

	// prepare JPEG stuff
	struct jpeg_compress_struct	cinfo;
	struct jpeg_error_mgr	jerr;
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);

	// set the output buffer
	unsigned char	*jbuffer = NULL;
	unsigned long	jbuffersize;
	jpeg_mem_dest(&cinfo, &jbuffer, &jbuffersize);

	// prepare the defaults
	cinfo.image_width = monoimage.getSize().width();
	cinfo.image_height = monoimage.getSize().height();
	cinfo.input_components = 1;
	cinfo.in_color_space = JCS_GRAYSCALE;

	// set default parameters
	jpeg_set_defaults(&cinfo);

	// set quality
	jpeg_set_quality(&cinfo, _quality, TRUE);

	// start the compression
	jpeg_start_compress(&cinfo, TRUE);
	while (cinfo.next_scanline < cinfo.image_height) {
		int	y = h - 1 - cinfo.next_scanline;
		for (int x = 0; x < w; x++) {
			unsigned char	p = monoimage.pixel(x, y);
			pixelline[x] = p;
		}
		jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}

	// close the compression
	jpeg_finish_compress(&cinfo);

	// destroy the data structures that were needed to compress
	jpeg_destroy_compress(&cinfo);

	// copy the buffer data
	*buffer = jbuffer;
	*buffersize = jbuffersize;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "wrote %d bytes", jbuffersize);

	// return the buffersize
	return *buffersize;
}

/**
 * \brief Write a mono image as a JPEG file
 *
 * \param monimage	monochrome image to write to JPEG file
 * \param filename	name of the JPEG file
 */
size_t	JPEG::writeJPEG(const ConstImageAdapter<unsigned char>& monoimage,
		const std::string& filename) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "write %s image to %s",
		monoimage.getSize().toString().c_str(),
		filename.c_str());

	// prepare a pixel buffer
	int	w = monoimage.getSize().width();
	int	h = monoimage.getSize().height();
	unsigned char	pixelline[w];
	JSAMPROW	row_pointer[1];
	row_pointer[0] = pixelline;

	// prepare JPEG stuff
	struct jpeg_compress_struct	cinfo;
	struct jpeg_error_mgr	jerr;
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);

	// set the output buffer
	FILE	*outfile;
	if ((outfile = fopen(filename.c_str(), "wb")) == NULL) {
		std::string	msg = stringprintf("cannot open file %s: %s",
			filename.c_str(), strerror(errno));
		return 0;
	}
	jpeg_stdio_dest(&cinfo, outfile);

	// prepare the defaults
	cinfo.image_width = monoimage.getSize().width();
	cinfo.image_height = monoimage.getSize().height();
	cinfo.input_components = 1;
	cinfo.in_color_space = JCS_GRAYSCALE;

	// set default parameters
	jpeg_set_defaults(&cinfo);

	// set quality
	jpeg_set_quality(&cinfo, _quality, TRUE);

	// start the compression
	jpeg_start_compress(&cinfo, TRUE);
	while (cinfo.next_scanline < cinfo.image_height) {
		int	y = h - 1 - cinfo.next_scanline;
		for (int x = 0; x < w; x++) {
			unsigned char	p = monoimage.pixel(x, y);
			pixelline[x] = p;
		}
		jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}

	// close the compression
	jpeg_finish_compress(&cinfo);

	// destroy the data structures that were needed to compress
	jpeg_destroy_compress(&cinfo);

	// copy the buffer data
	fclose(outfile);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "file %s closed", filename.c_str());

	// get the filesize
	struct stat	sb;
	if (stat(filename.c_str(), &sb) < 0) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot stat %s: %s", 
			filename.c_str(), strerror(errno));
		return 0;
	}

	// return the buffersize
	return sb.st_size;
}

/**
 * \brief Write an image as JPEG to a file
 *
 * \param image		the image to write
 * \param filename	the name of the JPEG file
 */
size_t	JPEG::writeJPEG(ImagePtr image, const std::string& filename) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "writing %s image to %s",
		demangle_cstr(*image), filename.c_str());
	{
		Image<unsigned char>	*img
			= dynamic_cast<Image<unsigned char> *>(&*image);
		if (NULL != img) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "mono image jpg");
			return writeJPEG(*img, filename);
		}
	}
	{
		Image<RGB<unsigned char> >	*img
			= dynamic_cast<Image<RGB<unsigned char> >*>(&*image);
		if (NULL != img) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "color image jpg");
			return writeJPEG(*img, filename);
		}
	}
	{
		FormatReduction	*img = FormatReduction::get(image);
		if (NULL != img) {
			size_t	s = writeJPEG(*img, filename);
			delete img;
			return s;
		}
	}
	{
		FormatReductionRGB	*img = FormatReductionRGB::get(image);
		if (NULL != img) {
			size_t	s = writeJPEG(*img, filename);
			delete img;
			return s;
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "no matching pixel type");
	return 0;
}

/**
 * \brief Write an image as JPEG to a buffer
 *
 * \param image
 * \param buffer
 * \param buffersize
 */
size_t	JPEG::writeJPEG(ImagePtr image, void **buffer, size_t *buffersize) {
	{
		Image<unsigned char>	*img
			= dynamic_cast<Image<unsigned char >*>(&*image);
		if (NULL != img) {
			return writeJPEG(*img, buffer, buffersize);
		}
	}
	{
		Image<RGB<unsigned char> >	*img
			= dynamic_cast<Image<RGB<unsigned char > >*>(&*image);
		if (NULL != img) {
			return writeJPEG(*img, buffer, buffersize);
		}
	}
	{
		FormatReduction	*img = FormatReduction::get(image);
		if (NULL != img) {
			size_t	s = writeJPEG(*img, buffer, buffersize);
			delete img;
			return s;
		}
	}
	{
		FormatReductionRGB	*img = FormatReductionRGB::get(image);
		if (NULL != img) {
			size_t	s = writeJPEG(*img, buffer, buffersize);
			delete img;
			return s;
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "no matching pixel type");
	return 0;
}

/**
 * \brief Read an Image from a JPEG file
 */
ImagePtr      JPEG::readJPEG(const std::string& filename) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "reading %s", filename.c_str());
	// decompression structure
	jpeg_decompress_struct	cinfo;
	struct jpeg_error_mgr	jerr;
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	// open the input file
	FILE *infile = fopen(filename.c_str(), "rb");;
	if (NULL == infile) {
		std::string	msg = stringprintf("cannot open %s: %s", 
			filename.c_str(), strerror(errno));
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "file %s opened", filename.c_str());

	// attach the input file as the JPG source
	jpeg_stdio_src(&cinfo, infile);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "reading header");

	// read the header
	int	rc = jpeg_read_header(&cinfo, TRUE);
	if (rc != 1) {
		std::string	msg = stringprintf("cannot read header");
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// start decompression
	jpeg_start_decompress(&cinfo);
	ImageSize	size(cinfo.output_width, cinfo.output_height);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "read %s image from %s",
		size.toString().c_str(), filename.c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "decompression started");

	// construct the output image
	Image<RGB<unsigned char> >	*colorimage = NULL;
	Image<unsigned char>		*monoimage = NULL;
	switch (cinfo.output_components) {
	case 1:	monoimage = new Image<unsigned char>(size);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "mono image %s",
			size.toString().c_str());
		break;
	case 3:	colorimage = new Image<RGB<unsigned char> >(size);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "color image %s",
			size.toString().c_str());
		break;
	default:
		std::string	msg = stringprintf("don't know how to deal "
			"with %d components", cinfo.output_components);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// prepare a pixel row
	int	row_stride = cinfo.output_width * cinfo.output_components;
	JSAMPARRAY	buffer = (*cinfo.mem->alloc_sarray)
			((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

	// scan the image
	int	h = size.height();
	int	w = size.width();
	while (cinfo.output_scanline < cinfo.output_height) {
		int	Y = h - 1 - cinfo.output_scanline;
		jpeg_read_scanlines(&cinfo, buffer, 1);
		if (colorimage) {
			for (int x = 0; x < w; x++) {
				unsigned char	R = buffer[0][3 * x    ];
				unsigned char	G = buffer[0][3 * x + 1];
				unsigned char	B = buffer[0][3 * x + 2];
				RGB<unsigned char>	p(R, G, B);
				colorimage->pixel(x, Y) = p;
			}
		}
		if (monoimage) {
			for (int x = 0; x < w; x++) {
				monoimage->pixel(x, Y) = buffer[0][x];
			}
		}
	}

	// complete the decompression
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	// close the file
	fclose(infile);

	// return the image file
	if (colorimage) { return ImagePtr(colorimage); }
	if (monoimage) { return ImagePtr(monoimage); }
	return ImagePtr();
}

/**
 * \brief Read an Image from a JPEG buffer
 *
 * \param buffer	buffer containing the JPEG data
 * \param buffersize	size of the buffer
 */
ImagePtr	JPEG::readJPEG(void *buffer, size_t buffersize) {
	// decompression structure
	jpeg_decompress_struct	cinfo;
	struct jpeg_error_mgr	jerr;
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	// attach the input file as the JPG source
	jpeg_mem_src(&cinfo, (unsigned char *)buffer, buffersize);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "reading header");

	// read the header
	int	rc = jpeg_read_header(&cinfo, TRUE);
	if (rc != 1) {
		std::string	msg = stringprintf("cannot read header");
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// start decompression
	jpeg_start_decompress(&cinfo);
	ImageSize	size(cinfo.output_width, cinfo.output_height);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "decompression started");

	// construct the output image
	Image<RGB<unsigned char> >	*colorimage = NULL;
	Image<unsigned char>		*monoimage = NULL;
	switch (cinfo.output_components) {
	case 1:	monoimage = new Image<unsigned char>(size);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "mono image %s",
			size.toString().c_str());
		break;
	case 3:	colorimage = new Image<RGB<unsigned char> >(size);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "color image %s",
			size.toString().c_str());
		break;
	default:
		std::string	msg = stringprintf("don't know how to deal "
			"with %d components", cinfo.output_components);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// prepare a pixel row
	int	row_stride = cinfo.output_width * cinfo.output_components;
	JSAMPARRAY	linebuffer = (*cinfo.mem->alloc_sarray)
			((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

	// scan the image
	int	h = size.height();
	int	w = size.width();
	while (cinfo.output_scanline < cinfo.output_height) {
		int	Y = h - 1 - cinfo.output_scanline;
		jpeg_read_scanlines(&cinfo, linebuffer, 1);
		if (colorimage) {
			for (int x = 0; x < w; x++) {
				unsigned char	R = linebuffer[0][3 * x    ];
				unsigned char	G = linebuffer[0][3 * x + 1];
				unsigned char	B = linebuffer[0][3 * x + 2];
				RGB<unsigned char>	p(R, G, B);
				colorimage->pixel(x, Y) = p;
			}
		}
		if (monoimage) {
			for (int x = 0; x < w; x++) {
				monoimage->pixel(x, Y) = linebuffer[0][x];
			}
		}
	}

	// complete the decompression
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	// return the image file
	if (colorimage) { return ImagePtr(colorimage); }
	if (monoimage) { return ImagePtr(monoimage); }
	return ImagePtr();
	return ImagePtr();
}

} // namespace image
} // namespace astro
