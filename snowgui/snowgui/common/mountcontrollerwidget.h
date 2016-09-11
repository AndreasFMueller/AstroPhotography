/*
 * mountcontrollerwidget.h
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef MOUNTCONTROLLERWIDGET_H
#define MOUNTCONTROLLERWIDGET_H

#include <InstrumentWidget.h>
#include <QTimer>

namespace snowgui {

namespace Ui {
	class mountcontrollerwidget;
}

class mountcontrollerwidget : public InstrumentWidget {
	Q_OBJECT

	snowstar::mountstate	_previousstate;
	snowstar::MountPrx	_mount;

public:
	explicit mountcontrollerwidget(QWidget *parent = 0);
	~mountcontrollerwidget();
	virtual void	instrumentSetup(
		astro::discover::ServiceObject serviceobject,
		snowstar::RemoteInstrument instrument);

signals:
	void	mountSelected(int);

private:
	Ui::mountcontrollerwidget *ui;
	QTimer	_statusTimer;

	void	setupMount();

public slots:
	void	mountChanged(int);
	void	gotoClicked();
	void	statusUpdate();
};

} // namespace snowgui

#endif // MOUNTCONTROLLERWIDGET_H
