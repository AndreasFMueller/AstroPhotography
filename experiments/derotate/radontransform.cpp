/*
 * radontransform.cpp -- perform the radon transform of an image
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <stdio.h>
#include <stdlib.h>
#include <opencv.hpp>
#include "radon.h"
#include <iostream>

int	main(int argc, char *argv[]) {
	int	c;
	int	width = 512;
	int	height = 512;

	while (EOF != (c = getopt(argc, argv, "w:h:")))
		switch (c) {
		case 'w':
			width = atoi(optarg);
			break;
		case 'h':
			height = atoi(optarg);
			break;
		}	

	if ((argc - optind) != 2) {
		std::cerr << "need exactly two file name arguments" << std::endl;
		exit(EXIT_FAILURE);
	}

	const char	*infile = argv[optind++];
	const char	*outfile = argv[optind];

	// read the image
	cv::Mat	r = radon(infile, width, height);

	// write the result
	imwrite(std::string(outfile), r);
}
