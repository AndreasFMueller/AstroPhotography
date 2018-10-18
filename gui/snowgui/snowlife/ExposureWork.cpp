/*
 * ExposureWork.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "liveview.h"

namespace snowgui {

ExposureWork::ExposureWork(LiveView *liveview)
	: QObject(NULL), _liveview(liveview) {
}

ExposureWork::~ExposureWork() {
}

void	ExposureWork::doExposure() {
	_liveview->doExposure();
}

} // namespace snowgui
