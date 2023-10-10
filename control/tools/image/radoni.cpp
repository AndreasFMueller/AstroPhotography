/*
 * radoni.cpp -- radon transform of an image
 *
 * (c) 2023 Prof Dr Andreas Müller
 */
#include <includes.h>
#include <AstroDebug.h>
#include <AstroImage.h>
#include <AstroIO.h>
#include <Radon.h>
#include <AstroAdapter.h>
#include <fftw3.h>

using namespace astro;
using namespace astro::io;
using namespace astro::image;
using namespace astro::image::radon;
using namespace astro::adapter;

namespace astro {
namespace app {
namespace radoni {

/**
 * \brief display a message about the radoni program
 */
static void	usage(const char *progname) {
	Path	p(progname);
	std::cout << "usage:" << std::endl;
	std::cout << "    " << p.basename() << " [ options ] infile outfile"
		<< std::endl;
	std::cout << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << "   -d,--debug       increase debug level" << std::endl;
	std::cout << std::endl;
}

static struct option	longopts[] = {
{ "debug",		no_argument,		NULL,		'd' },
{ NULL,			0,			NULL,		 0  }
};

int	main(int argc, char *argv[]) {
	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "d?",
		longopts, &longindex)))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		}

	// next argument must be the file names
	if ((argc - optind) != 4) {
		std::cerr << "wrong number of arguments" << std::endl;
		usage(argv[0]);
		return EXIT_FAILURE;
	}
	std::string	infilename = argv[optind++];
	std::string	f1filename = argv[optind++];
	std::string	f2filename = argv[optind++];
	std::string	ofilename = argv[optind++];

	// read the input image
	FITSin	in(infilename);
	ImagePtr	imageptr = in.read();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got an image of dimension %d x %d",
		imageptr->size().width(), imageptr->size().height());

	// convert the image to double
	Image<double>	*input = new Image<double>(
				ConstPixelValueAdapter<double>(imageptr));
	ImagePtr	inputimage(input);

	// compute horizontal fourier transforms for every line
	int	h = imageptr->size().height();
	int	w = imageptr->size().width();
	fftw_complex	*f1[h];
	fftw_complex	*line = (fftw_complex *)fftw_malloc(
					sizeof(fftw_complex) * w);
	for (int i = 0; i < h; i++) {
		f1[i] = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * w);
		fftw_plan	p = fftw_plan_dft_1d(w, line, f1[i], -1,
			FFTW_ESTIMATE);
		for (int k = 0; k < w; k++) {
			line[k][0] = input->pixel(k, i);
			line[k][1] = 0;
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "transforming row %d", i);
		fftw_execute(p);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "row %d transformed (%f)", i,
			f1[i][0][0]);
		fftw_destroy_plan(p);
	}

	// convert the fourier transforms into an image
	Image<double>	*f1image = new Image<double>(w, h);
	ImagePtr	f1ptr(f1image);
	for (int x = 0; x < w; x++) {
		for (int y = 0; y < h; y++) {
			double	v = hypot(f1[y][x][0], f1[y][x][1]);
			f1image->pixel((x + w/2) % w, y) = v;
		}
	}
	FITSout	f1out(f1filename);
	f1out.write(f1ptr);

	// transform the fourier transforms to polar coordinates
	fftw_complex	*f2 = (fftw_complex *)fftw_malloc(
		sizeof(fftw_complex) * w * w);
	for (int x = 0; x < w; x++) {
		for (int y = 0; y < w; y++) {
			f2[y + w * x][0] = 0;
			f2[y + w * x][1] = 0;
			// find the polar coordinates
			double	r = hypot(x - w/2, y - w/2);
			int	ri = round(r);
			if (ri < w) {
				double	phi = atan2(y - w/2, x - w/2);
				int	phii = h * (phi + 2 * M_PI) / (2 * M_PI);
				phii = phii % h;
				debug(LOG_DEBUG, DEBUG_LOG, 0,
				"(%d,%d): phii = %d, ri = %d", x, y, phii, ri);
				f2[y + w * x][0] = f1[phii][ri][0];
				f2[y + w * x][1] = f1[phii][ri][1];
			}
		}
	}

	// display the polar coordinate version
	Image<double>	*f2image = new Image<double>(w, w);
	ImagePtr	f2imageptr(f2image);
	FITSout	f2out(f2filename);
	for (int x = 0; x < w; x++) {
		for (int y = 0; y < w; y++) {
			double	v = hypot(f2[y + w * x][0], f2[y + w * x][1]);
			f2image->pixel(x,y) = v;
		}
	}
	f2out.write(f2imageptr);

	// perform the inverse transform
	fftw_complex	*o = (fftw_complex *)fftw_malloc(
		sizeof(fftw_complex) * w * w);
	fftw_plan	p = fftw_plan_dft_2d(w, w, f2, o, 1, FFTW_ESTIMATE);
	fftw_execute(p);
	fftw_destroy_plan(p);
	
	// display the image
	Image<double>	*oimage = new Image<double>(w, w);
	ImagePtr	oimageptr(oimage);
	for (int x = 0; x < w; x++) {
		for (int y = 0; y < w; y++) {
			double	v = hypot(o[y + w * x][0], o[y + w * x][1]);
			oimage->pixel(x,y) = v;
		}
	}
	FITSout	oout(ofilename);
	oout.write(oimageptr);

	// cleanup

	return EXIT_SUCCESS;
}

} // namespace radoni
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::app::radoni::main>(argc, argv);
}


