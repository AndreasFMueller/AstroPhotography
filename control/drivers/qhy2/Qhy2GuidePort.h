/*
 * Qhy2GuidePort.h -- QHY guide port
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _Qhy2GuidePort_h
#define _Qhy2GuidePort_h

#include <AstroCamera.h>
#include <Qhy2Camera.h>
#include <chrono>

namespace astro {
namespace camera {
namespace qhy2 {

class Qhy2GuidePort : public GuidePort {
	Qhy2Camera&	camera;
	std::chrono::system_clock::time_point	until_raplus;
	std::chrono::system_clock::time_point	until_raminus;
	std::chrono::system_clock::time_point	until_decplus;
	std::chrono::system_clock::time_point	until_decminus;
	void	activate(std::chrono::system_clock::time_point& until,
			float duration_ms, int direction);
public:
	Qhy2GuidePort(Qhy2Camera& _camera);
	virtual ~Qhy2GuidePort();
	virtual uint8_t	active();
	virtual void	activate(float raplus, float raminus,
				float decplus, float decminus);
};

} // namepsace qhy2
} // namespace camera
} // namespace astro

#endif /* _Qhy2GuidePort_h */
