/*
 * focusercontrollerwidget.h -- Focuser controller widget declaration
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef FOCUSERCONTROLLERWIDGET_H
#define FOCUSERCONTROLLERWIDGET_H

#include <InstrumentWidget.h>
#include <AstroCamera.h>
#include <QTimer>

namespace snowgui {

namespace Ui {
	class focusercontrollerwidget;
}

/**
 * \brief A reusable GUI component to control a focuser
 */
class focusercontrollerwidget : public InstrumentWidget {
	Q_OBJECT

	snowstar::FocuserPrx	_focuser;
	bool	_backlash_enabled;
	bool	_backlashing;
	int	_backlash;
public:
	explicit focusercontrollerwidget(QWidget *parent = NULL);
	~focusercontrollerwidget();
	virtual void	instrumentSetup(
		astro::discover::ServiceObject serviceobject,
		snowstar::RemoteInstrument instrument);
	virtual void	setupComplete();
	int	getCurrentPosition();
	int	_previousposition;

signals:
	void	targetPositionReached();
	void	newFocuserPosition(int);
	void	focuserSelected(snowstar::FocuserPrx);
	void	focuserSelected(int);

private:
	void	setupFocuser();

	void	displayCurrent(int current);
	void	displayTarget(int target);

	void	startMoving(int target);

	Ui::focusercontrollerwidget *ui;
	QTimer	statusTimer;
	int	delta;

public slots:
	void	setCurrent();
	void	setTarget(int position);
	void	movetoPosition(int position);
	void	statusUpdate();
	void	focuserChanged(int);
	void	editingFinished();
	void	guiChanged();
	void	backlashChanged(int);
};

} // namespace snowgui

#endif // FOCUSERCONTROLLERWIDGET_H
