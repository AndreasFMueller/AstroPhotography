/*
 * MeasureFocusWork.cpp -- implement the work finding the focus using a measure
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <FocusWork.h>
#include <AstroDebug.h>
#include <limits>
#include <AstroFormat.h>
#include <FocusCompute.h>
#include <AstroFilterfunc.h>
#include <stdexcept>
#include <AstroFilter.h>
#include <AstroAdapter.h>

using namespace astro::adapter;
using namespace astro::image::filter;
using namespace astro::image;

namespace astro {
namespace focusing {

std::string	FocusValue::toString() const {
	return stringprintf("pos=%hu, val=%g", position, value);
}

bool	FocusValue::operator==(const FocusValue& other) const {
	return (position == other.position) && (value && other.value);
}

FocusInterval::FocusInterval(const FocusValue& left, const FocusValue& right)
	: std::pair<FocusValue, FocusValue>(left, right) {
	if (left.position >= right.position) {
		throw std::runtime_error("");
	}
}

unsigned short	FocusInterval::length() const {
	return second.position - first.position;
}

unsigned short	FocusInterval::center() const {
	return ((long)second.position + (long)first.position) / 2;
}

std::string	FocusInterval::toString() const {
	return stringprintf("[%s, %s]", left().toString().c_str(),
		right().toString().c_str());
}

FocusInterval	FocusInterval::operator-(const FocusInterval& other) const {
	if (left() == other.left()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"difference is right subinterval: %s %s",
			other.right().toString().c_str(),
			right().toString().c_str());
		return FocusInterval(other.right(), right());
	}
	if (right() == other.right()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"difference is left subinterval");
		return FocusInterval(left(), other.left());
	}
	throw std::runtime_error("difference of intervals that cannot be subtracted");
}

class wronginterval : public std::runtime_error {
public:
	wronginterval(const std::string& cause) : std::runtime_error(cause) { }
};

/**
 * \brief Combine image and edge detection in one color image
 */
ImagePtr	MeasureFocusWork::combine(ImagePtr image, FocusInfo& focusinfo) {
	// find the maximum value of the edges
	Image<double>	*im = dynamic_cast<Image<double> *>(&*focusinfo.edges);
	if (NULL == im) {
		throw std::runtime_error("edges image has incorrect pixel type");
	}
	double	maxvalue = Max<double, double>().filter(*im);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "maximum edge value: %f", maxvalue);
	

	// create an adapter that rescales to a reasonable value
	RescaleAdapter<double>	rescale(*im, maxvalue);
	TypeReductionAdapter<unsigned char, double>	tc(rescale);
	Image<unsigned char>	*red = new Image<unsigned char>(tc);
	ImagePtr	redptr(red);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "maximum red: %f",
		astro::image::filter::max(redptr));

	// Rescale the image to produce the green channel
	Image<unsigned char>	*green = FocusWork::green(image);
	ImagePtr	greenptr(green);

	// create a constant blue channel
	ConstantValueAdapter<unsigned char>	blue(image->size(), 0);

	// combine the channels to a color image
	CombinationAdapter<unsigned char>	combinator(*red, *green, blue);
	Image<RGB<unsigned char> >	*result
		= new Image<RGB<unsigned char> >(combinator);

	return ImagePtr(result);
}

/**
 * \brief Subdivide a focus interval
 */
FocusInterval	MeasureFocusWork::subdivide(const FocusInterval& interval) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "interval subdivision %d", counter);
	if (counter > steps()) {
		throw std::runtime_error("number of steps exceeded");
	}
	FocusValue	v = measureat(interval.center());
	counter++;
	if ((v.value < interval.left().value) && (v.value < interval.right().value))  {
		throw wronginterval("new value is smaller than boundaries");
	}
	if (interval.left().value > interval.right().value) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "using left subdiv interval");
		return FocusInterval(interval.left(), v);
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "using right subdiv interval");
		return FocusInterval(v, interval.right());
	}
}

/**
 * \brief Perform a measurement at a certain focus position
 */
FocusValue	MeasureFocusWork::measureat(unsigned short pos) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "measurement at pos = %hu", pos);
	moveto(pos);
	focusingstatus(Focusing::MEASURING);
	ccd()->startExposure(exposure());
	ccd()->wait();
	ImagePtr	image = ccd()->getImage();
	FocusInfo	fi = astro::image::filter::focus_squaredgradient_extended(image);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "pos = %hu, value = %g(%f)",
		pos, fi.value, log10(fi.value));
	// call the callback
	callback(combine(image, fi), pos, fi.value);
	return FocusValue(pos, fi.value);
}

/**
 * \brief Main function of the Focusing process
 */
void	MeasureFocusWork::main(astro::thread::Thread<FocusWork>& /* thread */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start focusing work");
	if (!complete()) {
		focusingstatus(Focusing::FAILED);
		throw std::runtime_error("focuser not completely specified");
	}
	counter = 0;

	// move to the minimum
	debug(LOG_DEBUG, DEBUG_LOG, 0, "measure left end of interval: %hu",
		min());
	FocusValue	left = measureat(min());
	FocusValue	right = measureat(max());
	
	// perform measurements at both ends of the interval
	FocusInterval	interval(left, right);

	// perform subdivisions
	double	resolution = (max() - min()) / pow(2, steps());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "target resolution: %f", resolution);
	std::list<FocusInterval>	intervals;
	intervals.push_back(interval);
	try {
		while (interval.length() > resolution) {
			try {
				interval = subdivide(interval);
				intervals.push_back(interval);
			} catch (wronginterval) {
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"retrying other interval");
				intervals.pop_back();
				interval = *intervals.rbegin() - interval;
				intervals.push_back(interval);
			}
			debug(LOG_DEBUG, DEBUG_LOG, 0, "new interval: %s",
				interval.toString().c_str());
		}
		focusingstatus(Focusing::FOCUSED);
	} catch (std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "focus failed: %s", x.what());
		focusingstatus(Focusing::FAILED);
	} catch (...) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "unknown exception during focus");
		focusingstatus(Focusing::FAILED);
	}
}

} // namespace focusing
} // namespace astro
