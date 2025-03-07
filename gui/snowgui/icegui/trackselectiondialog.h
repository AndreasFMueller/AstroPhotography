/*
 * trackselectiondialog.h -- dialog to select a track from the database
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef SNOWGUI_TRACKSELECTIONDIALOG_H
#define SNOWGUI_TRACKSELECTIONDIALOG_H

#include <QDialog>
#include <guider.h>

namespace snowgui {

namespace Ui {
	class trackselectiondialog;
}

class trackselectiondialog : public QDialog {
	Q_OBJECT

	std::string      _instrumentname;
	snowstar::GuiderFactoryPrx      _guiderfactory;

	std::vector<snowstar::TrackingSummary>	_tracks;

public:
        void    setGuider(const std::string& instrumentname,
                        snowstar::GuiderFactoryPrx guiderfactory);

public:
	explicit trackselectiondialog(QWidget *parent = 0);
	~trackselectiondialog();

signals:
	void	trackSelected(snowstar::TrackingHistory);

private:
	Ui::trackselectiondialog *ui;

public slots:
	void	trackAccepted();
};

} // namespace snowgui

#endif // SNOWGUI_TRACKSELECTIONDIALOG_H
