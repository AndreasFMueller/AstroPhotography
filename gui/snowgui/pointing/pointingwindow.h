/*
 * pointingwindow.h
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef SNOWGUI_POINTINGWINDOW_H
#define SNOWGUI_POINTINGWINDOW_H

#include <InstrumentWidget.h>
#include <ccdcontrollerwidget.h>

namespace snowgui {

namespace Ui {
	class pointingwindow;
}

class pointingwindow : public InstrumentWidget {
	Q_OBJECT
	ccddata	_ccddata;

	astro::RaDec	_finder_direction;
	astro::RaDec	_guider_direction;
	astro::RaDec	_imager_direction;
	ccddata		_finder_ccddata;
	ccddata		_guider_ccddata;
	ccddata		_imager_ccddata;
	astro::image::Binning	_finder_binning;
	astro::image::Binning	_guider_binning;
	astro::image::Binning	_imager_binning;

public:
	explicit pointingwindow(QWidget *parent = 0);
	virtual void	instrumentSetup(
		astro::discover::ServiceObject serviceobject,
		snowstar::RemoteInstrument instrument);
	virtual void	setupComplete();
	~pointingwindow();

private:
	void	pointSelected(astro::image::ImagePoint, const astro::RaDec&,
			const ccddata&, const astro::image::Binning&);

public slots:
	void	newImage(astro::image::ImagePtr image);
	void	finderPointSelected(astro::image::ImagePoint);
	void	guiderPointSelected(astro::image::ImagePoint);
	void	imagerPointSelected(astro::image::ImagePoint);
	void	ccddataSelected(ccddata);

private:
	Ui::pointingwindow *ui;

protected:
	void	closeEvent(QCloseEvent *);
};

} // namespace snowgui
#endif // SNOWGUI_POINTINGWINDOW_H
