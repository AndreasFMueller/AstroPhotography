#ifndef GUIDERMONITORDIALOG_H
#define GUIDERMONITORDIALOG_H

#include <QDialog>
#include <guider.hh>

namespace Ui {
class GuiderMonitorDialog;
}

/**
 * \brief Separate namespace for the monitoring callback servants
 */
namespace guidermonitor {

class TrackingMonitor_impl;
class TrackingImageMonitor_impl;

} // namespace guidermonitor

class GuiderMonitorDialog : public QDialog
{
	Q_OBJECT
        Astro::Guider_var       _guider;
	guidermonitor::TrackingMonitor_impl	*tm_impl;
	long	monitorid;
	guidermonitor::TrackingImageMonitor_impl	*tim_impl;
	long	imagemonitorid;
	Astro::TrackingInfo	trackinginfo;

	// image information
	pthread_mutex_t	mutex;			// lock to protect
	Astro::ImageSize	imagesize;	// image size
	unsigned short	*data;			// image pixel data
	QPixmap	image2pixmap(const Astro::ImageSize& size,
			const unsigned short *imagedata);

public:
	explicit GuiderMonitorDialog(Astro::Guider_var guider,
		QWidget *parent = 0);
	~GuiderMonitorDialog();

	void	update(const Astro::TrackingInfo& ti);
	void	update(const Astro::ImageSize& size,
			const Astro::ShortSequence& imagedata);

signals:
	void	trackingInfoUpdated();
	void	trackingImageUpdated();
	void	xUpdate(double x);
	void	yUpdate(double y);

public slots:
	void	displayTrackingInfo();
	void	displayTrackingImage();

private:
	Ui::GuiderMonitorDialog *ui;
};


namespace guidermonitor {

/**
 * \brief Tracking Monitor for the Qt GuiderMonitor
 */
class TrackingMonitor_impl : public POA_Astro::TrackingMonitor {
	GuiderMonitorDialog&	_guidermonitordialog;
public:
	TrackingMonitor_impl(GuiderMonitorDialog& guidermonitordialog)
		: _guidermonitordialog(guidermonitordialog) { }
	virtual void	update(const ::Astro::TrackingInfo& ti);
};

class TrackingImageMonitor_impl : public POA_Astro::TrackingImageMonitor {
	GuiderMonitorDialog&	_guidermonitordialog;
public:
	TrackingImageMonitor_impl(GuiderMonitorDialog& guidermonitordialog)
		: _guidermonitordialog(guidermonitordialog) { }
	virtual void	update(const ::Astro::ImageSize& size,
				const ::Astro::ShortSequence& imagedata);
};

} // namespace guidermonitor

#endif // GUIDERMONITORDIALOG_H
