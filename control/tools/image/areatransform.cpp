/*
 * areatransform.cpp
 *
 * (c) 2026 Prof Dr Andreas Müller 
 */
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <AstroImage.h>
#include <AstroTypes.h>
#include <AstroTransform.h>
#include <getopt.h>

namespace astro {
namespace app {
namespace areatransform {

/**
 * \brief Class to implement matrix multiplication
 */
class Matrix {
	double	_m[4];
public:
	Matrix(double m11, double m12, double m21, double m22) {
		_m[0] = m11;
		_m[1] = m12;
		_m[2] = m21;
		_m[3] = m22;
	}
	Matrix() {
		_m[0] = 1;
		_m[1] = 0;
		_m[2] = 0;
		_m[3] = 1;
	}
	Matrix(const std::string& spec) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "parsing %s", spec.c_str());
		std::string	s = spec;
		for (int i = 0; i < 4; i++) {
			auto	o = s.find_first_not_of("[,;]");
			s = s.substr(o);
			debug(LOG_DEBUG, DEBUG_LOG, 0, "next work string: '%s'",
				s.c_str());
			o = s.find_first_of("[,;]");
			_m[i] = std::stod(s.substr(0,o));
			debug(LOG_DEBUG, DEBUG_LOG, 0, "_m[%d] = %f\n",
				i, _m[i]);
			s = s.substr(o+1);
		}
	}
	Matrix(const Matrix& other) {
		for (int i = 0; i < 4; i++) {
			_m[i] = other._m[i];
		}
	}
	Matrix&	operator=(const Matrix& other) {
		for (int i = 0; i < 4; i++) {
			_m[i] = other._m[i];
		}
		return *this;
	}
	double	operator[](int i) const { return _m[i]; }
	double	det() const {
		return _m[0] * _m[3] - _m[1] * _m[2];
	}
	Matrix	inverse() const {
		double	d = det();
		return Matrix(
			 _m[3] / d,
			-_m[1] / d,
			-_m[2] / d,
			 _m[0] / d
		);
	}
	Point	operator*(const Point& p) const {
		return Point(
			_m[0] * p.x() + _m[1] * p.y(),
			_m[2] * p.x() + _m[3] * p.y()
		);
	}
	Matrix	operator*(const Matrix& other) const {
		return Matrix(
			_m[0] * other[0] + _m[1] * other[2],
			_m[0] * other[3] + _m[1] * other[4],
			_m[2] * other[0] + _m[3] * other[2],
			_m[2] * other[4] + _m[3] * other[4]
		);
	}
};

template<class Pixel>
class TransformAdapter : public ConstImageAdapter<Pixel> {
	const ConstImageAdapter<Pixel>&	_image;
	Matrix	_matrix;
	int	_n;
public:
	TransformAdapter(const ConstImageAdapter<Pixel>& image,
		  const Matrix& matrix, int n)
		: ConstImageAdapter<Pixel>(image.getSize()), _image(image),
		  _matrix(matrix), _n(n) {
	}
	virtual Pixel   pixel(int x, int y) const {
		Point	p(
				x / (double)_image.getSize().width(),
				y / (double)_image.getSize().height()
		);
		int	counter = _n;
		while (counter-- > 0) {
			p = _matrix * p;
		}
		ImagePoint	ip(
			(p.x() - floor(p.x())) * _image.getSize().width(),
			(p.y() - floor(p.y())) * _image.getSize().height()
		);
		return _image.pixel(ip);
	}

};

/**
 * \brief Write image to a file
 *
 * \param imageptr	the image to be writen
 * \param filename	the name of the file
 * \param sequenceno	the sequence number to use to build the file name
 */
static void	writeimage(image::ImagePtr imageptr,
			const std::string& filename, int sequenceno) {
	// create the filename
	std::string	outputfilename = filename;
	if (sequenceno >= 0) {
		auto	o = filename.find_last_of(".");
		outputfilename = stringprintf("%s-%02d%s",
			filename.substr(0, o).c_str(), sequenceno,
			filename.substr(o).c_str());
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "write image to %s",
		outputfilename.c_str());

	// write the image
	if (image::PNG::ispngfilename(outputfilename)) {
		image::PNG	png;
		png.writePNG(imageptr, outputfilename);
	} else if (image::JPEG::isjpegfilename(outputfilename)) {
		image::JPEG	jpeg;
		jpeg.writeJPEG(imageptr, outputfilename);
	} else if (image::FITS::isfitsfilename(outputfilename)) {
		image::FITS	fits;
		fits.writeFITS(imageptr, outputfilename);
	} else {
		std::string	msg = stringprintf("don't know how to write %s",
			outputfilename.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
}

/**
 * \brief Perform the image transformation 
 *
 * \param inimgptr	the image to transform
 * \param matrix	the transformation matrix to use
 * \return		the transformed image
 */
static image::ImagePtr	transform(image::ImagePtr inimgptr,
	const Matrix& matrix, int repeats = 1) {
	// get the image
	image::Image<image::RGB<unsigned char>>	*img
		= dynamic_cast<image::Image<image::RGB<unsigned char>>*>(&*inimgptr);
	if (NULL == img) {
		std::string	msg("not a unsigned char image");
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// setup the transform
	image::transform::PixelInterpolationAdapter<image::RGB<unsigned char>>
		pi(*img);
	TransformAdapter<image::RGB<unsigned char>>	ta(pi, matrix, repeats);
	
	// create output image
	image::Image<image::RGB<unsigned char>>	*outimg
		= new image::Image<image::RGB<unsigned char>>(ta);
	ImagePtr	outimgptr(outimg);
	return outimgptr;
}

/**
 * \brief Help message
 *
 * \param progname	name under which the program was called
 */
static void	usage(char *progname) {
	std::cout << "usage:" << std::endl;
	std::cout << "    " << progname << " [options ]" << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << " -d,--debug          enable debug messages"
		<< std::endl;
	std::cout << " -h,-?--help         display a help message and exit"
		<< std::endl;
	std::cout << " -i,--infile=<i>     read image from <i>" << std::endl;
	std::cout << " -o,--outfile=<o>    write image to <o>" << std::endl;
	std::cout << " -m,--matrix=<m>     use transform matrix <m>"
		<< std::endl;
	std::cout << "                     format: [ m11, m12; m21, m22 ]"
		<< std::endl;
}

/**
 * \brief long options for the areatransform program
 */
static struct option	options[] = {
{ "debug",	no_argument,		NULL,		'd' },
{ "help",	no_argument,		NULL,		'h' },
{ "infile",	required_argument,	NULL,		'i' },
{ "outfile",	required_argument,	NULL,		'o' },
{ "matrix",	required_argument,	NULL,		'm' },
{ "repeats",	required_argument,	NULL,		'n' },
{ "sequence",	no_argument,		NULL,		's' },
{ NULL,		0,			NULL,		 0  }
};

/**
 * \brief Main function for the areatransform program
 *
 * \param argc		number of arguments
 * \param argv		argument vector
 */
static int	main(int argc, char *argv[]) {
	debug_set_ident("areatransform");
	std::string	inputfilename;
	std::string	outputfilename;
	Matrix	matrix;
	int	repeats = 1;
	bool	sequence = false;
	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "dh?i:o:m:s",
		options, &longindex)))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'h':
		case '?':
			usage(argv[0]);
			return EXIT_SUCCESS;
		case 'i':
			inputfilename = std::string(optarg);
			break;
		case 'o':
			outputfilename = std::string(optarg);
			break;
		case 'm':
			debug(LOG_DEBUG, DEBUG_LOG, 0, "matrix: %s", optarg);
			matrix = Matrix(optarg).inverse();
			break;
		case 'n':
			repeats = std::stoi(optarg);
			break;
		case 's':
			sequence = true;
			break;
		}

	// read image or create test image
	image::ImagePtr	inimage;
	if (image::PNG::ispngfilename(inputfilename)) {
		image::PNG	png;
		inimage = png.readPNG(inputfilename);
	} else if (image::JPEG::isjpegfilename(inputfilename)) {
		image::JPEG	jpeg;
		inimage = jpeg.readJPEG(inputfilename);
	} else if (image::FITS::isfitsfilename(inputfilename)) {
		image::FITS	fits;
		inimage = fits.readFITS(inputfilename);
	} else {
		// create test image
		image::Image<image::RGB<unsigned char>>	*img
			= new image::Image<image::RGB<unsigned char>>(1500,1500);
		img->fill(image::RGB<unsigned char>((unsigned char)0,
			(unsigned char)0, (unsigned char)255));
		for (int x = 0; x < 500; x++) {
			for (int y = 0; y < 500; y++) {
				img->writablepixel(x,y)
					= image::RGB<unsigned char>(
						(unsigned char)255,
						(unsigned char)0,
						(unsigned char)0
					);
			}
		}
		inimage = image::ImagePtr(img);
	}

	// apply transform
	if (sequence) {
		int	sequenceno = 0;
		writeimage(inimage, outputfilename, sequenceno);
		while (sequenceno++ < repeats) {
			inimage = transform(inimage, matrix);
			writeimage(inimage, outputfilename, sequenceno);
		}
	} else {
		image::ImagePtr	outimgptr = transform(inimage, matrix, repeats);
		writeimage(outimgptr, outputfilename, -1);
	}

	// complete
	return EXIT_SUCCESS;
}

} // namespace areatransform
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	try {
		return astro::app::areatransform::main(argc, argv);
	} catch (const std::exception& x) {
		std::cerr << "areatransform terminated by exception: ";
		std::cerr << x.what() << std::endl;
	} catch (...) {
		std::cerr << "areatransform terminated by unknown exception";
		std::cerr << std::endl;
	}
	return EXIT_FAILURE;
}
