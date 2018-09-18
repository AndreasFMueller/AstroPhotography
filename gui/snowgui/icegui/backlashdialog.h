/*
 * backlashdialog.h -- dialog to control backlash characterization
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef SNOWGUI_BACKLASHDIALOG_H
#define SNOWGUI_BACKLASHDIALOG_H

#include <QDialog>
#include <QTimer>
#include <guider.h>

namespace snowgui {

class BacklashMonitor;

namespace Ui {
class BacklashDialog;
}

/**
 * \brief Dialog about backlash assessment
 *
 * This dialog can be used to assess the amount of backlash of a mount.
 * It can then be used to tune the backlash compensation.
 */
class BacklashDialog : public QDialog {
	Q_OBJECT

	snowstar::GuiderPrx	_guider;
	snowstar::GuiderState	_previousstate;
	snowstar::BacklashDirection	_direction;
	snowstar::BacklashData		_data;
	BacklashMonitor	*_monitor;
	Ice::Identity	_monitoridentity;
	
	QTimer	statusTimer;
public:
	explicit BacklashDialog(QWidget *parent = 0);
	~BacklashDialog();

	void	guider(snowstar::GuiderPrx guider);
	snowstar::BacklashDirection	direction() const;
	void	direction(snowstar::BacklashDirection d);


private:
	Ui::BacklashDialog *ui;

	void	showResult();
	void	reloadPoints();
	void	addPoint(const snowstar::BacklashPoint&);
	void	windowTitle();

public slots:
	void	startClicked();
	void	statusUpdate();

	void	stopSignaled();
	void	updatePointSignaled(snowstar::BacklashPoint);
	void	updateResultSignaled(snowstar::BacklashResult);
	void	lastpointsChanged(int);
};

} // namespace snowgui

#endif // SNOWGUI_BACKLASHDIALOG_H
