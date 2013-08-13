/*
 * rotate.cpp -- rotate an image
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <opencv.hpp>
#include <Accelerate/Accelerate.h>
#include <math.h>

int	debug = 1;
double	epsilon = 0.0001;
double	angle = 0;

/**
 * \brief
 */

/**
 * \brief Fix 
 */
cv::Point2d	getshift(const cv::Rect& rectangle, cv::Mat& i1, cv::Mat& i2) {
	cv::Mat	sub1;
	i1(rectangle).convertTo(sub1, CV_32FC1);
	cv::Mat	sub2;
	i2(rectangle).convertTo(sub2, CV_32FC1);
	cv::Point2d	v = cv::phaseCorrelate(sub1, sub2);
	return v;
}

/**
 * \brief 
 */
cv::Mat	findTransform(cv::Mat& before, cv::Mat& after, int l = 64) {
	// prepare the result array
	cv::Mat	transform(2, 3, CV_64FC1);
	transform.at<double>(0, 0) = 1;
	transform.at<double>(0, 1) = 0;
	transform.at<double>(0, 2) = 0;
	transform.at<double>(1, 0) = 0;
	transform.at<double>(1, 1) = 1;
	transform.at<double>(1, 2) = 0;

	// compute the center of the image
	int	width = before.cols;
	int	height = after.rows;
	cv::Point	center = cv::Point(width/2, height/2);

	// compute grid parameters
	int	l2 = l / 2;
	int	x0 = center.x - l * trunc(center.x / l) + l2;
	int	y0 = center.y - l * trunc(center.y / l) + l2;
	double	tx = 0, ty = 0;

	// iterate over the grid and compute the local translation in each 
	// grid point. This will give the grid we need for LAPACK to compute
	// the optimal transformation
	typedef std::pair<cv::Point2d, cv::Point2d>	pointpair;
	std::vector<pointpair>	translates;
	for (int x = x0; x + l2 < width; x += l) {
		for (int y = y0; y + l2 < height; y += l) {
			cv::Point2d	tilecenter(x, y);
			cv::Point2d	translation = getshift(cv::Rect(x - l2, y - l2, l, l), before, after);
			translates.push_back(std::make_pair(tilecenter, translation));
			tx += translation.x;
			ty += translation.y;
		}
	}

	// now compute the optimal affine transformation
	std::cout << "size: " << translates.size() << std::endl;
	double	a[12 * translates.size()];
	double	b[2 * translates.size()];
	std::vector<pointpair>::const_iterator	pair;
	int	m = 2 * translates.size();
	int	i = 0;
	for (pair = translates.begin(); pair != translates.end(); pair++, i++) {
		// add coefficients to A array
		a[2 * i            ] = pair->first.x;
		a[2 * i     +     m] = pair->first.y;
		a[2 * i     + 2 * m] = 1;
		a[2 * i     + 3 * m] = 0;
		a[2 * i     + 4 * m] = 0;
		a[2 * i     + 5 * m] = 0;

		a[2 * i + 1        ] = 0;
		a[2 * i + 1 +     m] = 0;
		a[2 * i + 1 + 2 * m] = 0;
		a[2 * i + 1 + 3 * m] = pair->first.x;
		a[2 * i + 1 + 4 * m] = pair->first.y;
		a[2 * i + 1 + 5 * m] = 1;
		// add positions to B array
		b[2 * i    ] = pair->first.x + pair->second.x;
		b[2 * i + 1] = pair->first.y + pair->second.y;
	}
	std::cout << "arrays prepared" << std::endl;

	// now use LAPACK to solve the system of equations
	char	trans = 'N';
	int	n = 6;
	int	nrhs = 1;
	int	lda = m;
	int	ldb = m;
	int	lwork = -1;
	int	info = 0;

	// first perform a call to find out how much data is needed
	std::cout << "calling dgels" << std::endl;
	double	x;
	dgels_(&trans, &m, &n, &nrhs, a, &lda, b, &ldb, &x, &lwork, &info);
	if (info != 0) {
		std::cerr << "dgels lwork determination failed: " << info << std::endl;
		exit(EXIT_FAILURE);
	}
	lwork = x;
	std::cout << "lwork = " << lwork << std::endl;

	// now allocate the work array and perform the real computation
	double	work[lwork];
	dgels_(&trans, &m, &n, &nrhs, a, &lda, b, &ldb, work, &lwork, &info);
	if (info != 0) {
		std::cerr << "dgels solution failed: " << info << std::endl;
	}
	transform.at<double>(0, 0) = b[0];
	transform.at<double>(0, 1) = b[1];
	transform.at<double>(0, 2) = b[2];
	transform.at<double>(1, 0) = b[3];
	transform.at<double>(1, 1) = b[4];
	transform.at<double>(1, 2) = b[5];

	// just for comparison, also compute the 
	return transform;
}

cv::Mat	iteratedTransform(cv::Mat& before, cv::Mat& after, int l = 64) {
	// image dimensions
	int	width = before.cols;
	int	height = before.rows;
	cv::Size	size(width, height);

	// initial transform
	cv::Mat	transform(2, 3, CV_64FC1);
	transform.at<double>(0) =  cos(M_PI * (angle - 1)/ 180);
	transform.at<double>(1) =  sin(M_PI * (angle - 1)/ 180);
	transform.at<double>(3) = -sin(M_PI * (angle - 1)/ 180);;
	transform.at<double>(4) =  cos(M_PI * (angle - 1)/ 180);
	transform.at<double>(2) = 0;
	transform.at<double>(5) = 0;
	std::cout << "start transform: " << transform << std::endl;

	double	s = 0;
	do {
		// apply current transform to a copy of the before image
		cv::Mat	workimg(width, height, CV_32FC1);
		cv::warpAffine(before, workimg, transform, size);

		// determine remaining transform 
		cv::Mat	newtransform = findTransform(workimg, after, l);
		std::cout << "newtransform: " << newtransform << std::endl;

		// add remaining transform to current transform
		double	u[6];
		u[0] = newtransform.at<double>(0) * transform.at<double>(0)
		     + newtransform.at<double>(1) * transform.at<double>(3);
		u[1] = newtransform.at<double>(0) * transform.at<double>(1)
		     + newtransform.at<double>(1) * transform.at<double>(4);
		u[3] = newtransform.at<double>(3) * transform.at<double>(0)
		     + newtransform.at<double>(4) * transform.at<double>(3);
		u[4] = newtransform.at<double>(3) * transform.at<double>(1)
		     + newtransform.at<double>(4) * transform.at<double>(4);

		u[2] = newtransform.at<double>(0) * transform.at<double>(2)
		     + newtransform.at<double>(1) * transform.at<double>(5)
		     + newtransform.at<double>(2);
		u[5] = newtransform.at<double>(3) * transform.at<double>(2)
		     + newtransform.at<double>(4) * transform.at<double>(5)
		     + newtransform.at<double>(5);

		for (int i = 0; i < 6; i++) {
			transform.at<double>(i) = u[i];
		}
		std::cout << "accumulated: " << transform << std::endl;

		// how close is the newtransform to the identity?
		s = 0;
		for (int i = 0; i < 6; i++) {
			double	x = newtransform.at<double>(i);
			if ((i == 0) || (i == 4)) {
				x = 1 - x;
			}
			s += x * x;
		}
		std::cout << "s = " << s << std::endl;
	} while (s > epsilon);

	return transform;
}


int	main(int argc, char *argv[]) {
	int	c;
	int	l = 128;
	double	tx = 0, ty = 0;
	while (EOF != (c = getopt(argc, argv, "a:l:e:x:y:")))
		switch (c) {
		case 'e':
			epsilon = atof(optarg);
			break;
		case 'a':
			angle = atof(optarg);
			break;
		case 'l':
			l = atoi(optarg);
			break;
		case 'x':
			tx = atof(optarg);
			break;
		case 'y':
			ty = atof(optarg);
			break;
		}

	if ((argc - optind) != 3) {
		fprintf(stderr, "need exactly two arguments\n");
		exit(EXIT_FAILURE);
	}

	const char	*infile = argv[optind++];
	const char	*rotatedfile = argv[optind++];
	const char	*recoveredfile = argv[optind];
	printf("rotate %s by angle %f to %s\n", infile, angle, rotatedfile);

	cv::Mat	inimg = cv::imread(std::string(infile));
	int	width = inimg.cols;
	int	height = inimg.rows;
	cv::Mat	ingray(width, height, CV_32FC1);
	cv::cvtColor(inimg, ingray, CV_BGR2GRAY);

	cv::Point	center = cv::Point(width/2, height/2);
	cv::Mat	rotmat = cv::getRotationMatrix2D(center, angle, 1);
	rotmat.at<double>(0, 2) += tx;
	rotmat.at<double>(1, 2) += ty;
	std::cout << rotmat << std::endl;
	cv::Mat	outimg(width, height, CV_32FC1);

	cv::warpAffine(ingray, outimg, rotmat, cv::Size(width, height));
	imwrite(std::string(rotatedfile), outimg);

	cv::Mat	transform = iteratedTransform(ingray, outimg, l);
	std::cout << transform << std::endl;

	inimg = cv::imread(std::string(infile));
	cv::Mat	recoveredimg(width, height, CV_32FC3);
	cv::warpAffine(inimg, recoveredimg, transform, cv::Size(width, height));
	imwrite(std::string(recoveredfile), recoveredimg);

	exit(EXIT_SUCCESS);
}
