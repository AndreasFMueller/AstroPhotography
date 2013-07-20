/*
 * transform.cpp -- some image transformations
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroTransform.h>
#include <AstroAdapter.h>
#include <AstroIO.h>
#include <stdlib.h>
#include <stdio.h>
#include <debug.h>
#include <Format.h>

using namespace astro::image;
using namespace astro::image::transform;
using namespace astro::io;

namespace astro {

int	main(int argc, char *argv[]) {
	// parse command line
	int	c;
	Point	translation;
	int	sample = 0;
	double	angle = 0;
	while (EOF != (c = getopt(argc, argv, "dx:y:s:a:")))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'a':
			angle = atof(optarg);
			break;
		case 'x':
			translation.x = atof(optarg);
			break;
		case 'y':
			translation.y = atof(optarg);
			break;
		case 's':
			sample = atoi(optarg);
			break;
		}

	// next two arguments must be filenames
	if ((argc - optind) != 2) {
		std::cerr << "need exactly two file name arguments"
			<< std::endl;
		return EXIT_FAILURE;
	}
	const char	*infilename = argv[optind++];
	const char	*outfilename = argv[optind++];

	// read the image from the file
	FITSin	infile(infilename);
	ImagePtr	image = infile.read();

	// prepare result
	ImagePtr	result;
	if (0 != angle) {
		// perform rotation
		Transform	rotation(angle, translation);
		result = astro::image::transform::transform(image, rotation);
	} else {
		// apply a sampling adapter and a translation adapter
		if (sample > 0) {
			ImageSize	sampling(1 + sample, 1 + sample);
			result = translate(upsample(image, sampling), translation);
		}
		if (sample < 0) {
			ImageSize	sampling(1 - sample, 1 - sample);
			result = translate(downsample(image, sampling), translation);
		}
		if (sample == 0) {
			result = translate(image, translation);
		}
	}
	
	// write the result image
	unlink(outfilename);
	FITSout	outfile(outfilename);
	outfile.write(result);

	// that's it
	return EXIT_SUCCESS;
}

} // namespace astro

int	main(int argc, char *argv[]) {
	try {
		return astro::main(argc, argv);
	} catch (std::exception& x) {
		std::cerr << "transform terminated by exception: " << x.what()
			<< std::endl;
		return EXIT_FAILURE;
	}
}
