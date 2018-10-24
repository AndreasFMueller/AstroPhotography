/*
 * focusscancontroller.h -- controller to perform a focus scan
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef FOCUSSCANCONTROLLER_H
#define FOCUSSCANCONTROLLER_H

#include <QWidget>
#include <AstroImage.h>

namespace snowgui {

namespace Ui {
	class focusscancontroller;
}

class focusscancontroller : public QWidget {
	Q_OBJECT

	int	centerposition;
	int	numberofsteps;
	int	stepsize;
	int	currentstep;
	int	position;
	bool	scanning;

public:
	explicit focusscancontroller(QWidget *parent = 0);
	~focusscancontroller();

signals:
	void	performCapture();
	void	movetoPosition(int);

private:
	Ui::focusscancontroller *ui;

	void	setScanning(bool);

public slots:
	void	changeCenter(int);
	void	startScan();
	void	stopScan();
	void	scanClicked();
	void	positionReached();
	void	imageReceived(astro::image::ImagePtr);
};

} // namespace snowgui

#endif // FOCUSSCANCONTROLLER_H
