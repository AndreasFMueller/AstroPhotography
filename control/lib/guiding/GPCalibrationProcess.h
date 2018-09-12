/*
 * GPCalibrationProcess.h -- declaration of the GPCalibrationProcess class
 *
 * This class is not to be exposed to applications, so we don't install
 * this header file
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _GPCalibrationProcess_h
#define _GPCalibrationProcess_h

#include <AstroGuiding.h>
#include "GuidePortProcess.h"

using namespace astro::camera;
using namespace astro::image;

namespace astro {
namespace guiding {

/**
 * \brief Encapsulation of the guiding process for guide ports
 *
 * This class contains the work function for guider calibration.
 * Note that adaptive optics devices have their own calibration process.
 */
class GPCalibrationProcess : public GuidePortProcess {
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
	Point	starAt(double ra, double dec);
	void	moveto(double ra, double dec);
	void	measure(int deltara, int deltadec);
	void	callback(const CalibrationPoint& calpoint);
	void	callback(const ProgressInfo& progressinfo);
	void	callback(const CalibrationPtr calibration);
	void	callback(const ImagePtr& image);
	void	callback(const std::exception& ex);
private:
	GPCalibrationProcess(const GPCalibrationProcess& other);
	GPCalibrationProcess&	operator=(const GPCalibrationProcess& other);
public:
	GPCalibrationProcess(GuiderBase *guider, camera::GuidePortPtr guideport,
		TrackerPtr tracker, persistence::Database database = NULL);
	~GPCalibrationProcess();
	void	focallength(double f) { _focallength = f; }
	virtual void	start();
	// the main function of the process
	void	main(astro::thread::Thread<GPCalibrationProcess>& thread);
private:
	void	main2(astro::thread::Thread<GPCalibrationProcess>& thread);
};

} // namespace guiding
} // namespace astro

#endif /* _GPCalibrationProcess_h */
