/*
 * AtikCcd.cpp -- ATIK CCD implementation
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AtikCcd.h>

namespace astro {
namespace camera {
namespace atik {

AtikCcd::AtikCcd(CcdInfo& info, ::AtikCamera *camera)
	: Ccd(info), _camera(camera) {
}

bool	AtikCcd::hasCooler() {
	return false;
}

} // namespace atik
} // namespace camera
} // namespace astro
