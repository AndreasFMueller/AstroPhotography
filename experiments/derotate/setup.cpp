/*
 * build a pair of images for tests of derotation algorithms
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <stdio.h>
#include <stdlib.h>
#include <opencv.hpp>
#include <unistd.h>
#include <iostream>

double	angle = 0;
cv::Point2d	translation(0, 0);
double	scale = 1;

int	main(int argc, char *argv[]) {
	int	c;
	int	width = 512;
	int	height = 512;
	while (EOF != (c = getopt(argc, argv, "x:y:a:w:h:")))
		switch (c) {
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
			scale = atof(optarg);
			break;
		case 'w':
			width = atoi(optarg);
			break;
		case 'h':
			height = atoi(optarg);
			break;
		}

	if ((argc - optind) != 2) {
		std::cerr << "need exactly two filename arguments" << std::endl;
		exit(EXIT_FAILURE);
	}

	const char	*infile = argv[optind++];
	const char	*outfile = argv[optind];

	// read the infile
	cv::Mat	inimg = cv::imread(std::string(infile));
	cv::Size	size(inimg.cols, inimg.rows);
	cv::Point2d	center(size.width / 2, size.height / 2);

	cv::Rect	rect(center.x - width / 2, center.y - height / 2,
				width, height);

	// the the transform
	cv::Mat	rotmat = cv::getRotationMatrix2D(center, angle, 1);
	rotmat.at<double>(0, 2) += translation.x;
	rotmat.at<double>(1, 2) += translation.y;
	std::cout << rotmat << std::endl;

	// apply the transform
	cv::Mat	outimg(size.width, size.height, CV_32FC1);
	cv::warpAffine(inimg, outimg, rotmat, size);

	// write the outfile
	imwrite(std::string(outfile), outimg(rect));

	exit(EXIT_SUCCESS);
}
