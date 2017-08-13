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
	snowstar::BacklashDirection	direction() const { return _direction; }
	void	direction(snowstar::BacklashDirection d) {
		_direction = d;
	}


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
};

} // namespace snowgui

#endif // SNOWGUI_BACKLASHDIALOG_H
