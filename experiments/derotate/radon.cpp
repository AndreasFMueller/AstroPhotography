/*
 * radon.cpp -- perform the radon transform of an image
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "radon.h"
#include <stdio.h>
#include <stdlib.h>
#include <opencv.hpp>
#include <limits>
#include <stdexcept>
#include <iostream>
#include <math.h>

static inline double	min(double a, double b) {
	return (a < b) ? a : b;
}

double	margin = 0.4;

/**
 * \brief Grid abstraction used to iterate over a grid
 */
class GridPoint;
class Grid {
public:
	unsigned int	width;
	unsigned int	height;
	Grid(unsigned int _width, unsigned int _height) : width(_width), height(_height) { }
	bool	contains(const GridPoint& point) const;
	GridPoint	center() const;
	double	maxS() const;
};

double	Grid::maxS() const {
	return hypot(width, height) / 2;
}

class GridPoint {
public:
	double	x;
	double	y;
	GridPoint(const double x = 0, const double = 0);
	GridPoint	operator+(const GridPoint& other) const;
	GridPoint	operator*(const double lambda) const;
};

GridPoint	Grid::center() const {
	return GridPoint(width / 2, height / 2);
}

GridPoint::GridPoint(const double _x, const double _y) : x(_x), y(_y) {
}

GridPoint	GridPoint::operator+(const GridPoint& other) const {
	return GridPoint(x + other.x, y + other.y);
}

GridPoint	GridPoint::operator*(const double lambda) const {
	return GridPoint(lambda * x, lambda * y);
}

GridPoint	operator*(const double lambda, const GridPoint& point) {
	return GridPoint(lambda * point.x, lambda * point.y);
}

std::ostream&	operator<<(std::ostream& out, const GridPoint& point) {
	out << "[ x=" << point.x << ", y=" << point.y << " ]";
	return out;
}

bool	Grid::contains(const GridPoint& point) const {
	return (0 <= point.x) && (point.x < width) && (0 <= point.y) && (point.y < height);
}

/**
 * \brief A Grid ray is a line through a grid
 */
class GridIterator;
class GridRay {
	const Grid&	grid;
	double	angle;
	double	s;
	GridPoint	direction;
	GridPoint	initial;
public:
	GridRay(const Grid& grid, const double angle, const double s);
	GridPoint	point(double t) const;
	GridIterator	begin();
	GridIterator	end();
	double	paramX(double x) const;
	double	paramY(double y) const;
	friend std::ostream&	operator<<(std::ostream& out, const GridRay& ray);
	friend class GridIterator;
};

std::ostream& 	operator<<(std::ostream& out, const GridRay& ray) {
	std::cout << "direction=" << ray.direction << ", initial=" << ray.initial;
	return out;
}

GridRay::GridRay(const Grid& _grid, const double _angle, const double _s) : grid(_grid), angle(_angle), s(_s) {
	while (angle < 0) {
		angle += 2 * M_PI;
	}
	while (angle > M_PI) {
		angle -= M_PI;
	}
	//std::cout << "angle=" << angle << ", s=" << s << std::endl;
	direction.x = cos(angle);
	direction.y = sin(angle);
	initial.x = grid.width / 2 - s * direction.y;
	initial.y = grid.height / 2 + s * direction.x;
}

GridPoint	GridRay::point(double t) const {
	return initial + t * direction;
}

double	GridRay::paramX(double x) const {
	return (x - initial.x) / direction.x;
}

double	GridRay::paramY(double y) const {
	return (y - initial.y) / direction.y;
}

/**
 * \brief Grid Iterator class
 */
class GridIterator {
	GridRay&	gridray;
	double	t;	// parameter for entry point
	double	nextt;
	void	next();
public:
	double	weight;
	typedef enum gridpos_e { LEFT, BOTTOM, RIGHT } gridpos;
private:
	gridpos	pos;
	gridpos nextpos;
	bool	valid() const;
public:
	int	x, y;
	int	nextx, nexty;
	GridIterator(GridRay& gridray);
	GridIterator&	operator=(const GridIterator& other);
	bool	operator==(const GridIterator& other) const;
	bool	operator!=(const GridIterator& other) const;
	GridIterator&	operator++();
	GridPoint	point() const;
	friend class GridRay;
	friend std::ostream&	operator<<(std::ostream& out, const GridIterator& iterator);
};

std::ostream&	operator<< (std::ostream& out, const GridIterator::gridpos& pos) {
	switch (pos) {
	case GridIterator::LEFT:
		out << std::string("LEFT");
		break;
	case GridIterator::BOTTOM:
		out << std::string("BOTTOM");
		break;
	case GridIterator::RIGHT:
		out << std::string("RIGHT");
		break;
	}
	return out;
}

bool	GridIterator::valid() const {
	if ((x < 0) || (y < 0)) {
		return false;
	}
	if (x >= gridray.grid.width) {
		return false;
	}
	if (y >= gridray.grid.height) {
		return false;
	}
	return true;
}

GridPoint	GridIterator::point() const {
	return gridray.point(t);
}

std::ostream&	operator<<(std::ostream& out, const GridIterator& iterator) {
	out << "[ t=" << iterator.t;
	out << ", point=" << iterator.point();
	out << ", x=" << iterator.x;
	out << ", y=" << iterator.y;
	out << ", pos=" << iterator.pos;
	out << ", weight=" << iterator.weight;
	out << ", nextx=" << iterator.nextx;
	out << ", nexty=" << iterator.nexty;
	out << ", nextt=" << iterator.nextt;
	out << ", nextpos=" << iterator.nextpos;
	out << " ]";
	return out;
}

GridIterator::GridIterator(GridRay& _gridray) : gridray(_gridray) {
	t = -std::numeric_limits<double>::infinity();
	x = -1;
	y = -1;
	weight = 0;
}

GridIterator&	GridIterator::operator=(const GridIterator& other) {
	t = other.t;
	x = other.x;
	y = other.y;
	pos = other.pos;
	nextx = other.nextx;
	nexty = other.nexty;
	nextt = other.nextt;
	nextpos = other.nextpos;
	return *this;
}

bool	GridIterator::operator==(const GridIterator& other) const {
	return (x == other.x) && (y == other.y);
}

bool	GridIterator::operator!=(const GridIterator& other) const {
	return !(*this == other);
}

const static double epsilon = 0.00001;

void	GridIterator::next() {
	// special cases: horizontal ...
	if (gridray.direction.y == 0) {
		if (gridray.direction.x > 0) {
			nextx = x + 1;
			nexty = y;
		} else {
			nextx = x - 1;
			nexty = y;
		}
		nextt = t + 1;
		weight = 1;
		nextpos = pos;
		return;
	}

	// ... and vertical
	if (gridray.direction.x == 0) {
		nexty = y + 1;
		nextx = x;
		nextt = t + 1;
		nextpos = pos;
		weight = 1;
		return;
	}

	// go to the next pixel
	double	t0 = gridray.paramX(x);
	double	t1 = gridray.paramY(y + 1);
	double	t2 = gridray.paramX(x + 1);

	//std::cout << "t=" << t << " t0=" << t0 << " t1=" << t1 << " t2=" << t2 << std::endl;

	// the next point is the upper left corner of the current pixel
	if ((t0 > t + epsilon) && (t0 == t1)) {
		//std::cout << "upper left corner" << std::endl;
		nextx = x - 1;
		nexty = y + 1;
		weight = sqrt(2);
		nextt = t0;
		nextpos = BOTTOM;
		//std::cout << "completed DIAGONAL up left" << std::endl;
		return;
	}

	// the next point is the upper right corner of the current pixel
	if ((t1 > t + epsilon) && (t1 == t2)) {
		//std::cout << "diagonal up right" << std::endl;
		nextx = x + 1;
		nexty = y + 1;
		weight = sqrt(2);
		nextt = t1;
		nextpos = BOTTOM;
		//std::cout << "completed DIAGONAL up right " << std::endl;
		return;
	}

	// starting from a bottom or right pixel, either to
	// the left border or the top border. The left border
	// is reached if t0 is smaller than t1
	if (pos == RIGHT) {
		if (t0 < t1) {
			//std::cout << "case left" << std::endl;
			nextx = x - 1;
			nexty = y;
			nextt = t0;
			nextpos = RIGHT;
		} else {
			//std::cout << "case top" << std::endl;
			nextx = x;
			nexty = y + 1;
			nextt = t1;
			nextpos = BOTTOM;
		}
		//std::cout << "completed RIGHT " << std::endl;
		weight = nextt - t;
		return;
	}

	if (pos == BOTTOM) {
		if (gridray.direction.x <= 0) {
			//std::cout << "direction left" << std::endl;
			if (t0 < t1) {
				nextx = x;
				nexty = y;
				nextt = t0;
				nextpos = RIGHT;
			} else {
				nextx = x;
				nexty = y + 1;
				nextt = t1;
				nextpos = BOTTOM;
			}
		} else {
			//std::cout << "direction right" << std::endl;
			if (t1 < t2) {
				nextx = x;
				nexty = y + 1;
				nextt = t1;
				nextpos = BOTTOM;
			} else {
				nextx = x + 1;
				nexty = y;
				nextt = t2;
				nextpos = LEFT;
			}
		}
		weight = nextt - t;
		//std::cout << "completed BOTTOM " << std::endl;
		return;
	}

	if (pos == LEFT) {
		if (t2 < t1) {
			//std::cout << "case right" << std::endl;
			nextx = x + 1;
			nexty = y;
			nextt = t2;
			nextpos = LEFT;
		} else {
			nextx = x;
			nexty = y + 1;
			nextt = t1;
			nextpos = BOTTOM;
		}
		weight = nextt - t;
		//std::cout << "completed LEFT " << std::endl;
		return;
	}
	//std::cout << "no next point" << std::endl;
	throw std::runtime_error("cannot get next");
}

GridIterator&	GridIterator::operator++() {
	//std::cout << "switch to next item: " << *this << std::endl;
	x = nextx;
	y = nexty;
	t = nextt;
	pos = nextpos;
	try {
		next();
		if (!valid()) {
			//std::cerr << "invalid GridIterator: " << *this << std::endl;
			throw std::runtime_error("no next element");
		}
		return *this;
	} catch (std::exception& x) {
		//std::cerr << "exception: " << x.what() << std::endl;
	}
	x = -1;
	y = -1;
	t = -1;
	return *this;
}

GridIterator	GridRay::begin() {
	GridIterator	result(*this);
	double	t;
	// determine the value of t for which the ray enters the image
	if (direction.x == 0) {
		// special case
		//std::cout << "vertical case" << std::endl;
		if (!grid.contains(point(0))) {
			throw std::runtime_error("outside grid");
		}
		result.t = -(double)(grid.height / 2);
		result.x = trunc(point(0).x);
		result.y = 0;
		result.pos = GridIterator::BOTTOM;
		try {
			result.next();
			if (!result.valid()) {
				goto end;
			}
			return result;
		} catch (std::exception& x) {
		}
		return end();
	}
	if (direction.y == 0) {
		//std::cout << "horizontal case" << std::endl;
		if (!grid.contains(point(0))) {
			throw std::runtime_error("outside grid");
		}
		result.t = -(double)(grid.width / 2);
		result.x = 0;
		result.y = trunc(point(0).y);
		result.pos = (direction.x > 0) ? GridIterator::LEFT : GridIterator::RIGHT;
		try {
			result.next();
			if (!result.valid()) {
				goto end;
			}
			//std::cout << "initialized horizontal iterator: " << *this << std::endl << result << std::endl;
			return result;
		} catch (std::exception& x) {
			//std::cout << "no next point" << std::endl;
		}
		return end();
	}

	// normal case
	t = paramX(0);
	//std::cout << "paramX(0) = " << t << std::endl;
	// left border is only possible if the direction points to the right
	if ((direction.x > 0) && (t < 0)) {
		GridPoint	p = point(t);
		if (p.y >= 0) {
			result.t = t;
			result.x = 0;
			result.y = trunc(p.y);
			result.pos = GridIterator::LEFT;
			result.next();
			if (!result.valid()) {
				goto end;
			}
			return result;
		}
	}
	// right boundary
	t = paramX(grid.width);
	//std::cout << "paramX(grid.width) = " << t << std::endl;
	// right border is only possible if the direction points to the left
	if ((direction.x < 0) && (t < 0)) {
		GridPoint	p = point(t);
		if (p.y >= 0) {
			result.t = t;
			result.x = grid.width - 1;
			result.y = trunc(p.y);
			result.pos = GridIterator::RIGHT;
			result.next();
			if (!result.valid()) {
				goto end;
			}
			return result;
		}
	}
	// if we get to this point, the line we study must intersect
	// the lower border
	t = paramY(0);
	//std::cout << "paramY(0) = " << t << std::endl;
	if (t < 0) {
		GridPoint	p = point(t);
//std::cout << "Boundary point: " << p << std::endl;
		if ((0 <= p.x + epsilon) && (p.x - epsilon< grid.width)) {
			result.t = t;
			result.x = trunc(p.x);
			result.y = 0;
			result.pos = GridIterator::BOTTOM;
//std::cout << "trying to get next from here: " << result << std::endl;
			result.next();
			if (!result.valid()) {
				goto end;
			}
			return result;
		}
	}
	t = paramX(grid.width);
	//std::cout << "paramX(grid.width - 1) = " << t << std::endl;
	if (t < 0) {
		GridPoint	p = point(t);
		if (p.y >= 0) {
			result.t = t;
			result.x = grid.width - 1;
			result.y = round(p.y);
			result.pos = GridIterator::RIGHT;
			result.next();
			if (!result.valid()) {
				goto end;
			}
			return result;
		}
	}
end:
	result.x = -1;
	result.y = -1;
	return result;
}

GridIterator	GridRay::end() {
	GridIterator	result(*this);
	result.x = -1;
	result.y = -1;
	return result;
}

/**
 * \brief Radon Transform
 */
cv::Mat	radon(const char *filename, int width, int height) {
	// read the image
	cv::Mat	inimg = cv::imread(std::string(filename));
	//std::cout << "reading image " << filename << std::endl;

	// convert the image to grayscale
	cv::Mat ingray(inimg.rows, inimg.cols, CV_64FC1);
	cv::cvtColor(inimg, ingray, CV_BGR2GRAY);

	// now apply the windowing function (in place)
	cv::Point2d	center(inimg.cols / 2, inimg.rows / 2);
	double	r = min(center.x, center.y);
	for (int x = 0; x < inimg.cols; x++) {
		for (int y = 0; y < inimg.rows; y++) {
			double	l = hypot(x - center.x, y - center.y) / r;
			if (l < (1 - margin)) {
				continue;
			}
			double	f = 0;
			if (l < 1) {
				f = (1 + cos(M_PI * (l - 1 + margin) / margin)) / 2;
			}
			unsigned char	v = ingray.at<unsigned char>(y, x);
			v = v * f;
			ingray.at<unsigned char>(y, x) = f * v;
		}
	}
	cv::imwrite(std::string("masked.jpg"), ingray);
	Grid	grid(ingray.cols, ingray.rows);

	// create a new image that will take the Radon transform
	cv::Mat	radon(width, height, CV_64FC1);

	// compute the radon transform
	double	anglestep = M_PI / height;
	double	srange = 2 * grid.maxS();
	double	sstep = srange / width;
	int	y = 0, x = 0;
	double	norm = hypot(width, height) / 2;
	for (double angle = 0; y < height; angle += anglestep, y++) {
		//std::cout << "y = " << y << std::endl;
		x = 0;
		for (double s = -srange / 2; x < width; x++, s += sstep) {
			GridRay	ray(grid, angle, s);
			GridIterator	i(ray);
			double	S = 0;
			try {
				//std::cout << "iterator over ray " << ray << std::endl;
#if 1
				for (i = ray.begin(); i != ray.end(); ++i) {
					S += i.weight * ingray.at<unsigned char>(i.y, i.x);
				}
#endif
			} catch (std::exception& x) {
				//std::cerr << "exception: " << x.what() << std::endl;
			}
			unsigned char	value = S / norm;
			radon.at<double>(y, x) = value;
			//std::cout << "[" << x << "," << y << "] = " << S / norm << std::endl;
		}
	}

	// return the result
	return radon;
}
