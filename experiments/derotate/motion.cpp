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
#include <math.h>
#include <fftw3.h>

fftw_complex	*before;

void	removedc(double *a, size_t N) {
	double	s = 0;
	for (int x = 0; x < N; x++) {
		s += a[x];
	}
	s /= N;
	for (int x = 0; x < N; x++) {
		a[x] -= s;
	}
}

void	determine(const cv::Mat& beforeimg, const cv::Mat& afterimg) {
	size_t	N = beforeimg.cols;
	size_t	Nc = 1 + N/2;
	cv::Mat	out(N, N, CV_32FC1);
	//for (int bline = 0; bline < beforeimg.rows; bline++) {
	for (int bline = 247; bline < 248; bline++) {
		std::cout << "bline = " << bline << std::endl;
		// prepare space for fourier transform and plan the FFT
		double	b1[N];
		fftw_complex	*b1f = (fftw_complex *)fftw_malloc(
					sizeof(fftw_complex) * Nc);
		fftw_plan	p = fftw_plan_dft_r2c_1d(N, b1, b1f,
					FFTW_PRESERVE_INPUT);

		// copy the image line into a suitably sized array
		for (int x = 0; x < N; x++) {
			double	value = beforeimg.at<unsigned char>(bline, x);
			b1[N - 1 - x] = value;
		}
		removedc(b1, N);

		// perform the FFT
		fftw_execute(p);

		// plan fourier transform of after line
		double	a[N];
		fftw_complex	*af = (fftw_complex *)fftw_malloc(
					sizeof(fftw_complex) * Nc);
		fftw_plan	q = fftw_plan_dft_r2c_1d(N, a, af,
					FFTW_PRESERVE_INPUT);
		fftw_plan	r = fftw_plan_dft_c2r_1d(N, af, a,
					FFTW_PRESERVE_INPUT);

#if 0
for (int x = 0; x < Nc; x++) {
std::cout << "b1f[" << x << "] = (" << b1f[x][0] << " + " << b1f[x][1] << " I)" << std::endl;
}
#endif

		// now compute cross correlation with all other 
		for (int aline = 0; aline < afterimg.rows; aline++) {
		//for (int aline = 247; aline < 248; aline++) {
			// copy after image line
			for (int x = 0; x < N; x++) {
				a[x] = afterimg.at<unsigned char>(aline, x);
			}
			removedc(a, N);

			// perform fourier transform
			fftw_execute(q);

			// perform convolution
			for (int x = 0; x < Nc; x++) {
				// perform product
				double	re = af[x][0] * b1f[x][0]
						- af[x][1] * b1f[x][1];
				double	im = af[x][0] * b1f[x][1]
						+ af[x][1] * b1f[x][0];
#if 0
std::cout << "(" << af[x][0] << " + " << af[x][1] << " I) * ";
std::cout << "(" << b1f[x][0] << " + " << b1f[x][1] << " I) = ";
std::cout << re << " + " << im << " I" << std::endl;
#endif
				af[x][0] = re;
				af[x][1] = im;
			}
			
			// back transformation
			fftw_execute(r);

			// copy the result into the output array
			for (int x = 0; x < N; x++) {
				double	value = a[x] / (30 * N * N);
				out.at<float>(aline, x) = value;
#if 1
				if (fabs(value) > 0) {
				std::cout << "corr[" << x << "] = " << value << std::endl;
				}
#endif
			}
		}

		fftw_destroy_plan(q);
		fftw_destroy_plan(p);
		fftw_free(af);
		fftw_free(b1f);
		//fftw_free(b2f);
	}
	fftw_cleanup();
	// write the output image
	cv::imwrite(std::string("correlation.jpg"), out);
	
}

void	determine2(cv::Mat& beforegray, cv::Mat& aftergray) {
	size_t	N = beforegray.cols;
	for (int bline = 0; bline < N; bline++) {
		cv::Mat	b(1, N, CV_32FC1);
		for (int x = 0; x < N; x++) {
			float v = beforegray.at<unsigned char>(bline, x);
			b.at<float>(0, x) = v;
		}
		for (int aline = 0; aline < N; aline++) {
			cv::Mat	a(1, N, CV_32FC1);
			for (int x = 0; x < N; x++) {
				float v = aftergray.at<unsigned char>(aline, x);
				a.at<float>(0, x) = v;
			}
			cv::Point2d     v = cv::phaseCorrelate(b, a);
			std::cout << "bline = " << bline;
			std::cout << ", aline = " << aline << ", ";
			std::cout << v << std::endl;
		}
	}
}

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

	// convert both images to grayscale
        cv::Mat beforegray(beforeimg.rows, beforeimg.cols, CV_32FC1);
        cv::cvtColor(beforeimg, beforegray, CV_BGR2GRAY);

        cv::Mat aftergray(afterimg.rows, afterimg.cols, CV_32FC1);
        cv::cvtColor(afterimg, aftergray, CV_BGR2GRAY);

	determine2(beforegray, beforegray);

	exit(EXIT_SUCCESS);
}
