/*
 * CalibrationProcess.h -- declaration of the CalibrationProcess class
 *
 * This class is not to be exposed to applications, so we don't install
 * this header file
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _CalibrationProcess_h
#define _CalibrationProcess_h

#include <AstroGuiding.h>
#include "GuiderPortProcess.h"

using namespace astro::camera;
using namespace astro::image;

namespace astro {
namespace guiding {

/**
 * \brief Encapsulation of the guiding process
 *
 * This class contains the work function for guider calibration.
 */
class CalibrationProcess : public GuiderPortProcess {
	// parameters for the calibration process
	/**
	 * \brief Pixel size in um
 	 */
	double	grid;
	bool	calibrated;
	int	range;
	double	currentprogress(int ra, int dec) const;
	/**
	 * \brief start time
	 */
	double	starttime;
private:
	double	gridconstant(double focallength, double pixelsize) const;
	Point	starAt(double ra, double dec);
	void	moveto(double ra, double dec);
	void	measure(BasicCalibrator& calibrator,
			int deltara, int deltadec);
	void	callback(const CalibrationPoint& calpoint);
	void	callback(const ProgressInfo& progressinfo);
	void	callback(const GuiderCalibration& calibration);
	void	callback(const ImagePtr& image);
	void	callback(const std::exception& ex);
private:
	CalibrationProcess(const CalibrationProcess& other);
	CalibrationProcess&	operator=(const CalibrationProcess& other);
public:
	CalibrationProcess(GuiderBase *guider, camera::GuiderPortPtr guiderport,
		TrackerPtr tracker, persistence::Database database = NULL);
	~CalibrationProcess();
	void	focallength(double f) { _focallength = f; }
	virtual void	start();
	// the main function of the process
	void	main(astro::thread::Thread<CalibrationProcess>& thread);
private:
	void	main2(astro::thread::Thread<CalibrationProcess>& thread);
};

} // namespace guiding
} // namespace astro

#endif /* _CalibrationProcess_h */
