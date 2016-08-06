/*
 * guidercontrollerwidget.h -- widget to control aguider
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef GUIDERCONTROLLERWIDGET_H
#define GUIDERCONTROLLERWIDGET_H

#include <InstrumentWidget.h>
#include <guider.h>
#include <AstroCamera.h>
#include <AstroImage.h>

namespace Ui {
	class guidercontrollerwidget;
}

namespace snowgui {

class guidercontrollerwidget : public InstrumentWidget {
	Q_OBJECT

	snowstar::GuiderFactoryPrx	_guiderfactory;
	snowstar::GuiderDescriptor	_guiderdescriptor;
	snowstar::GuiderPrx		_guider;

	double	_guiderportinterval;
	double	_adaptiveopticsinterval;
	bool	_stepping;

	astro::camera::Exposure		_exposure;
	astro::image::ImagePoint	_star;

	QTimer	*statusTimer;

	void	setupGuider();

public:
	explicit guidercontrollerwidget(QWidget *parent = 0);
	virtual void    instrumentSetup(
		astro::discover::ServiceObject serviceobject,
		snowstar::RemoteInstrument instrument);
	~guidercontrollerwidget();

private:
	Ui::guidercontrollerwidget *ui;

public slots:
	void	setExposure(astro::camera::Exposure);
	void	setStar(astro::image::ImagePoint);
	void	setCcd(int);
	void	setGuiderport(int);
	void	setAdaptiveoptics(int);

	void	startGuiding();
	void	stopGuiding();

	void	statusUpdate();
	void	selectPoint(astro::image::ImagePoint);
	void	methodChanged(int);
};

} // namespace snowgui

#endif // GUIDERCONTROLLERWIDGET_H
