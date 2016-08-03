/*
 * focusercontrollerwidget.h -- Focuser controller widget declaration
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef FOCUSERCONTROLLERWIDGET_H
#define FOCUSERCONTROLLERWIDGET_H

#include <InstrumentWidget.h>
#include <AstroCamera.h>

namespace Ui {
	class focusercontrollerwidget;
}

namespace snowgui {

class focusercontrollerwidget : public InstrumentWidget {
	Q_OBJECT

	snowstar::FocuserPrx	_focuser;
public:
	explicit focusercontrollerwidget(QWidget *parent);
	~focusercontrollerwidget();
	virtual void	instrumentSetup(
		astro::discover::ServiceObject serviceobject,
		snowstar::RemoteInstrument instrument);
	int	getCurrentPosition();

signals:
	void	targetPositionReached();

private:
	void	setupFocuser();

	void	displayCurrent(int current);
	void	displayTarget(int target);

	void	startMoving(int target);

	Ui::focusercontrollerwidget *ui;
	QTimer	*statusTimer;
	int	delta;

public slots:
	void	setCurrent();
	void	setTarget(int position);
	void	movetoPosition(int position);
	void	statusUpdate();
	void	guiChanged();
	void	focuserChanged(int);
	void	editingFinished();
};

} // namespace snowgui

#endif // FOCUSERCONTROLLERWIDGET_H
