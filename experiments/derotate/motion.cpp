/*
 * motion.cpp -- extract motion from two radon transforms
 *
 * (c) 2013 Prof Dr Andreas Mueller
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <opencv.hpp>

int	main(int argc, char *argv[]) {
	int	c;
	while (EOF != (c = getopt(argc, argv, "d")))
		switch (c) {
		case 'd':
			break;
		}

	if ((argc - optind) != 2) {
		std::cerr << "need exactly two file arguments" << std::endl;
		exit(EXIT_FAILURE);
	}

	const char	*beforefilename = argv[optind++];
	const char	*afterfilename = argv[optind];

	cv::Mat	beforeimg = cv::imread(std::string(beforefilename));
	cv::Mat	afterimg = cv::imread(std::string(afterfilename));

	exit(EXIT_SUCCESS);
}
