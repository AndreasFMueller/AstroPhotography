/*
 * radon.h -- perform the radon transform of an image
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _radon_h
#define _radon_h

#include <opencv.hpp>

cv::Mat	radon(const char *filename, int width, int height);

#endif /* _radon_h */
