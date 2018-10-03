/*
 * HideProgress.h
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _HideProgress_h
#define _HideProgress_h

#include <QWidget>
#include <QTimer>
#include <QProgressBar>
#include <AstroUtils.h>

namespace snowgui {

/**
 * \brief Progress indicator that overlays an existing widget
 *
 * This class is used in the ccdcontrollerwidget to display how the 
 * exposure time passes.
 */
class HideProgress : public QWidget {
	Q_OBJECT
	float	_duration;
	astro::Timer	_start;
	QTimer	*_timer;
	QProgressBar	*_progressbar;
public:
	explicit HideProgress(float duration, QWidget *parent = NULL);
	virtual ~HideProgress();
protected:
	void	resizeEvent(QResizeEvent *event);
public slots:
	void	update();
};

} // namespace snowgui

#endif /* _HideProgress_h */
