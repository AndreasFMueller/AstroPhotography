/*
 * PNG.cpp -- reading/writing PNG images
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroImage.h>
#include <png.h>
#include <sys/stat.h>
#include <includes.h>

namespace astro {
namespace image {

/**
 * \brief Auxiliary function to find out whether a filename is JPEG
 *
 * \param filename	filename to investigate
 */
bool	PNG::ispngfilename(const std::string& filename) {
	if (filename.size() > 4) {
		if (filename.substr(filename.size() - 4)
			== std::string(".png")) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "filename %s is PNG",
				filename.c_str());
			return true;
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s is not PNG filename",
		filename.c_str());
	return false;
}

PNG::PNG() {
}

class PngWriteBuffer {
public:
	unsigned char	*_buffer;
	size_t		_buffersize;
	size_t		_written;
	PngWriteBuffer() : _buffer(NULL), _buffersize(0), _written(0) {
	}
	void	write(png_bytep data, png_size_t length) {
		_buffer = (unsigned char *)realloc(_buffer,
			_buffersize + length);
		memcpy(_buffer + _buffersize, data, length);
		_buffersize += length;
	}
};

static void	WriteDataToBuffer(png_structp png, png_bytep data,
	png_size_t length) {
	PngWriteBuffer	*p = (PngWriteBuffer *)png_get_io_ptr(png);
	p->write(data, length);
}

/**
 * \brief Write a color image to a PNG buffer
 *
 * \param colorimage
 * \param buffer
 * \param buffersize
 */
size_t	PNG::writePNG(const ConstImageAdapter<RGB<unsigned char> >& colorimage,
		void **buffer, size_t *buffersize) {
	int	width = colorimage.getSize().width();
	int	height = colorimage.getSize().height();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "writing %dx%d image", width, height);

	png_structp	png = png_create_write_struct(PNG_LIBPNG_VER_STRING,
				NULL, NULL, NULL);
	png_infop	info = png_create_info_struct(png);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "png = %p, info = %p", png, info);

	PngWriteBuffer	writebuffer;

	png_set_write_fn(png, &writebuffer, WriteDataToBuffer, NULL);

	png_set_IHDR(png, info, width, height, 8, PNG_COLOR_TYPE_RGB,
		PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT);
	png_write_info(png, info);

	png_bytep	row_pointers[height];
	for (int y = 0; y < height; y++) {
		int	Y = height - 1 - y;
		row_pointers[y] = (png_bytep)malloc(png_get_rowbytes(png,
					info));
		for (int x = 0; x < width; x++) {
			RGB<unsigned char>	p = colorimage.pixel(x, Y);
			row_pointers[y][3 * x    ] = p.R;
			row_pointers[y][3 * x + 1] = p.G;
			row_pointers[y][3 * x + 2] = p.B;
		}
	}

	png_write_image(png, row_pointers);
	png_write_end(png, NULL);

	for (int y = 0; y < height; y++) {
		free(row_pointers[y]);
	}

	if (png && info) {
		png_destroy_write_struct(&png, &info);
	}

	// write the result
	*buffer = writebuffer._buffer;
	*buffersize = writebuffer._buffersize;

	return *buffersize;
}

/**
 * \brief Write a color image to a PNG file
 *
 * \param colorimage
 * \param filename
 */
size_t	PNG::writePNG(const ConstImageAdapter<RGB<unsigned char> >& colorimage,
		const std::string& filename) {
	int	width = colorimage.getSize().width();
	int	height = colorimage.getSize().height();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "writing %dx%d image", width, height);

	// create a new file
	FILE	*outfile = fopen(filename.c_str(), "wb");
	if (NULL == outfile) {
		std::string	msg = stringprintf("cannot create file %s. %s",
			filename.c_str(), strerror(errno));
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	png_structp	png = png_create_write_struct(PNG_LIBPNG_VER_STRING,
				NULL, NULL, NULL);
	png_infop	info = png_create_info_struct(png);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "png = %p, info = %p", png, info);

	png_init_io(png, outfile);

	png_set_IHDR(png, info, width, height, 8, PNG_COLOR_TYPE_RGB,
		PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT);
	png_write_info(png, info);

	png_bytep	row_pointers[height];
	for (int y = 0; y < height; y++) {
		int	Y = height - 1 - y;
		row_pointers[y] = (png_bytep)malloc(png_get_rowbytes(png,
					info));
		for (int x = 0; x < width; x++) {
			RGB<unsigned char>	p = colorimage.pixel(x, Y);
			row_pointers[y][3 * x    ] = p.R;
			row_pointers[y][3 * x + 1] = p.G;
			row_pointers[y][3 * x + 2] = p.B;
		}
	}

	png_write_image(png, row_pointers);
	png_write_end(png, NULL);

	for (int y = 0; y < height; y++) {
		free(row_pointers[y]);
	}

	if (png && info) {
		png_destroy_write_struct(&png, &info);
	}

	fclose(outfile);
	
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
 * \brief Write a monochrome image to a PNG buffer
 *
 * \param monoimage
 * \param buffer
 * \param buffersize
 */
size_t	PNG::writePNG(const ConstImageAdapter<unsigned char>& monoimage,
		void **buffer, size_t *buffersize) {
	int	width = monoimage.getSize().width();
	int	height = monoimage.getSize().height();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "writing %dx%d image", width, height);

	png_structp	png = png_create_write_struct(PNG_LIBPNG_VER_STRING,
				NULL, NULL, NULL);
	png_infop	info = png_create_info_struct(png);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "png = %p, info = %p", png, info);

	PngWriteBuffer	writebuffer;

	png_set_write_fn(png, &writebuffer, WriteDataToBuffer, NULL);

	png_set_IHDR(png, info, width, height, 8, PNG_COLOR_TYPE_GRAY,
		PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT);
	png_write_info(png, info);

	png_bytep	row_pointers[height];
	for (int y = 0; y < height; y++) {
		int	Y = height - 1 - y;
		row_pointers[y] = (png_bytep)malloc(png_get_rowbytes(png,
					info));
		for (int x = 0; x < width; x++) {
			unsigned char	p = monoimage.pixel(x, Y);
			row_pointers[y][x] = p;
		}
	}

	png_write_image(png, row_pointers);
	png_write_end(png, NULL);

	for (int y = 0; y < height; y++) {
		free(row_pointers[y]);
	}

	if (png && info) {
		png_destroy_write_struct(&png, &info);
	}

	// write the result
	*buffer = writebuffer._buffer;
	*buffersize = writebuffer._buffersize;

	return *buffersize;
}

/**
 * \brief Write a monochrome image to a PNG file
 *
 * \param monoimage
 * \param filename
 */
size_t	PNG::writePNG(const ConstImageAdapter<unsigned char>& monoimage,
		const std::string& filename) {
	int	width = monoimage.getSize().width();
	int	height = monoimage.getSize().height();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "writing %dx%d image", width, height);

	// create a new file
	FILE	*outfile = fopen(filename.c_str(), "wb");
	if (NULL == outfile) {
		std::string	msg = stringprintf("cannot create file %s. %s",
			filename.c_str(), strerror(errno));
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	png_structp	png = png_create_write_struct(PNG_LIBPNG_VER_STRING,
				NULL, NULL, NULL);
	png_infop	info = png_create_info_struct(png);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "png = %p, info = %p", png, info);

	png_init_io(png, outfile);

	png_set_IHDR(png, info, width, height, 8, PNG_COLOR_TYPE_GRAY,
		PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT);
	png_write_info(png, info);

	png_bytep	row_pointers[height];
	for (int y = 0; y < height; y++) {
		int	Y = height - 1 - y;
		row_pointers[y] = (png_bytep)malloc(png_get_rowbytes(png,
					info));
		for (int x = 0; x < width; x++) {
			unsigned char	p = monoimage.pixel(x, Y);
			row_pointers[y][x] = p;
		}
	}

	png_write_image(png, row_pointers);
	png_write_end(png, NULL);

	for (int y = 0; y < height; y++) {
		free(row_pointers[y]);
	}

	if (png && info) {
		png_destroy_write_struct(&png, &info);
	}

	fclose(outfile);
	
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
 * \brief Write an image to a PNG file
 *
 * \param image
 * \param buffer
 * \param buffersize
 */
size_t	PNG::writePNG(ImagePtr image,
		void **buffer, size_t *buffersize) {
	{
		Image<unsigned char>    *img
			= dynamic_cast<Image<unsigned char> *>(&*image);
		if (NULL != img) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "mono image png");
			return writePNG(*img, buffer, buffersize);
		}
	}
	{
		Image<RGB<unsigned char> >      *img
			= dynamic_cast<Image<RGB<unsigned char> >*>(&*image);
		if (NULL != img) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "color image png");
			return writePNG(*img, buffer, buffersize);
		}
	}
	{
		FormatReduction	*img = FormatReduction::get(image);
		if (NULL != img) {
			size_t	s = writePNG(*img, buffer, buffersize);
			delete img;
			return s;
		}
	}
	{
		FormatReductionRGB	*img = FormatReductionRGB::get(image);
		if (NULL != img) {
			size_t	s = writePNG(*img, buffer, buffersize);
			delete img;
			return s;
		}
	}
        debug(LOG_DEBUG, DEBUG_LOG, 0, "no matching pixel type");
	return 0;
}

/**
 * \brief Write an image to a PNG file
 *
 * \param image
 * \param filename
 */
size_t  PNG::writePNG(ImagePtr image, const std::string& filename) {
	{
		Image<unsigned char>    *img
			= dynamic_cast<Image<unsigned char> *>(&*image);
		if (NULL != img) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "mono image png");
			return writePNG(*img, filename);
		}
	}
	{
		Image<RGB<unsigned char> >      *img
			= dynamic_cast<Image<RGB<unsigned char> >*>(&*image);
		if (NULL != img) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "color image png");
			return writePNG(*img, filename);
		}
	}
	{
		FormatReduction	*img = FormatReduction::get(image);
		if (NULL != img) {
			size_t	s = writePNG(*img, filename);
			delete img;
			return s;
		}
	}
	{
		FormatReductionRGB	*img = FormatReductionRGB::get(image);
		if (NULL != img) {
			size_t	s = writePNG(*img, filename);
			delete img;
			return s;
		}
	}
        debug(LOG_DEBUG, DEBUG_LOG, 0, "no matching pixel type");
	return 0;
}

// read PNG images
/**
 * \brief Read a PNG image
 *
 * \param filename	name of the PNG file
 */
ImagePtr	PNG::readPNG(const std::string& filename) {
	// open the input file
	FILE	*infile = fopen(filename.c_str(), "rb");
	png_structp	png = png_create_read_struct(PNG_LIBPNG_VER_STRING,
				NULL, NULL, NULL);
	png_infop	info = png_create_info_struct(png);

	png_init_io(png, infile);
	png_read_info(png, info);

	// get information about the image
	int	width = png_get_image_width(png, info);
	int	height = png_get_image_height(png, info);
	ImageSize	size(width, height);
	png_byte	color_type = png_get_color_type(png, info);
	png_byte	bit_depth = png_get_bit_depth(png, info);
	if (bit_depth > 8) {
		std::string	msg = stringprintf("don't know how to handle "
			"%d-bit images", bit_depth);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	if (color_type == PNG_COLOR_TYPE_PALETTE) {
		png_set_palette_to_rgb(png);
	}
	if ((color_type == PNG_COLOR_TYPE_GRAY) && (bit_depth < 8)) {
		png_set_expand_gray_1_2_4_to_8(png);
	}
	if ((color_type == PNG_COLOR_TYPE_RGB) ||
		(color_type = PNG_COLOR_TYPE_GRAY) ||
		(color_type = PNG_COLOR_TYPE_PALETTE)) {
		png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
	}

	if ((color_type == PNG_COLOR_TYPE_GRAY) ||
		(color_type == PNG_COLOR_TYPE_GRAY_ALPHA)) {
		png_set_gray_to_rgb(png);
	}

	png_read_update_info(png, info);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "open %s image %d bits",
		size.toString().c_str(), bit_depth);

	png_bytep row_pointers[height];
	for (int y = 0; y < height; y++) {
		row_pointers[y] = (png_byte *)malloc(png_get_rowbytes(png, info));
	}

	png_read_image(png, row_pointers);

	Image<RGB<unsigned char> >	*image
		= new Image<RGB<unsigned char> >(size);
	for (int y = 0; y < height; y++) {
		int	Y = height - 1 - y;
		for (int x = 0; x < width; x++) {
			unsigned char	R = row_pointers[y][4 * x    ];
			unsigned char	G = row_pointers[y][4 * x + 1];
			unsigned char	B = row_pointers[y][4 * x + 2];
			RGB<unsigned char>	p(R, G, B);
			image->pixel(x, Y) = p;
		}
		free(row_pointers[y]);
	}

	png_destroy_read_struct(&png, &info, NULL);

	fclose(infile);

	return ImagePtr(image);
}

/**
 * \brief Auxiliary class to wrap around buffer for reading
 */
class PngReadBuffer {
	unsigned char	*_buffer;
	size_t		_buffersize;
	size_t		_offset;
public:
	PngReadBuffer(unsigned char *buffer, size_t buffersize)
		: _buffer(buffer), _buffersize(buffersize) {
		_offset = 0;
	}
	void	read(png_bytep outBytes, png_size_t byteCountToRead) {
		memcpy(outBytes, _buffer + _offset, byteCountToRead);
		_offset += byteCountToRead;
	}
};

static void	ReadDataFromInputStream(png_structp png, png_bytep outBytes,
			png_size_t byteCountToRead) {
	PngReadBuffer	*p = (PngReadBuffer*)png_get_io_ptr(png);
	p->read(outBytes, byteCountToRead);
}

/**
 * \brief Read a PNG image from a buffer
 *
 * This is based on
 * http://pulsarengine.com/2009/01/reading-png-images-from-memory/
 */
ImagePtr	PNG::readPNG(void *buffer, size_t buffersize) {
	// open the input file
	PngReadBuffer	readbuffer((unsigned char *)buffer, buffersize);

	// create the PNG structure
	png_structp	png = png_create_read_struct(PNG_LIBPNG_VER_STRING,
				NULL, NULL, NULL);
	png_infop	info = png_create_info_struct(png);

	png_set_read_fn(png, &readbuffer, ReadDataFromInputStream);

	png_read_info(png, info);

	// get information about the image
	int	width = png_get_image_width(png, info);
	int	height = png_get_image_height(png, info);
	ImageSize	size(width, height);
	png_byte	color_type = png_get_color_type(png, info);
	png_byte	bit_depth = png_get_bit_depth(png, info);
	if (bit_depth > 8) {
		std::string	msg = stringprintf("don't know how to handle "
			"%d-bit images", bit_depth);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	if (color_type == PNG_COLOR_TYPE_PALETTE) {
		png_set_palette_to_rgb(png);
	}
	if ((color_type == PNG_COLOR_TYPE_GRAY) && (bit_depth < 8)) {
		png_set_expand_gray_1_2_4_to_8(png);
	}
	if ((color_type == PNG_COLOR_TYPE_RGB) ||
		(color_type = PNG_COLOR_TYPE_GRAY) ||
		(color_type = PNG_COLOR_TYPE_PALETTE)) {
		png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
	}

	if ((color_type == PNG_COLOR_TYPE_GRAY) ||
		(color_type == PNG_COLOR_TYPE_GRAY_ALPHA)) {
		png_set_gray_to_rgb(png);
	}

	png_read_update_info(png, info);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "open %s image %d bits",
		size.toString().c_str(), bit_depth);

	png_bytep row_pointers[height];
	for (int y = 0; y < height; y++) {
		row_pointers[y] = (png_byte *)malloc(png_get_rowbytes(png,
					info));
	}

	png_read_image(png, row_pointers);

	Image<RGB<unsigned char> >	*image
		= new Image<RGB<unsigned char> >(size);
	for (int y = 0; y < height; y++) {
		int	Y = height - 1 - y;
		for (int x = 0; x < width; x++) {
			unsigned char	R = row_pointers[y][4 * x    ];
			unsigned char	G = row_pointers[y][4 * x + 1];
			unsigned char	B = row_pointers[y][4 * x + 2];
			RGB<unsigned char>	p(R, G, B);
			image->pixel(x, Y) = p;
		}
		free(row_pointers[y]);
	}

	png_destroy_read_struct(&png, &info, NULL);

	return ImagePtr(image);
}

} // namespace image
} // namespace astro
