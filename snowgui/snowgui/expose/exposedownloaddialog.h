/*
 * exposedownloadialog.h -- dialog to display progress of download
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef SNOWGUI_EXPOSEDOWNLOADDIALOG_H
#define SNOWGUI_EXPOSEDOWNLOADDIALOG_H

#include <QDialog>
#include "downloadthread.h"
#include <repository.h>

namespace snowgui {

namespace Ui {
	class exposedownloaddialog;
}

class exposedownloaddialog : public QDialog {
	Q_OBJECT

public:
	explicit exposedownloaddialog(QWidget *parent = 0);
	~exposedownloaddialog();

	void	set(snowstar::RepositoriesPrx repositories,
			const downloadlist& filelist);

public slots:
	void	reject();
	void	accept();
	void	updateStatus(downloaditem);
	void	downloadComplete();
	void	downloadAborted();

private:
	Ui::exposedownloaddialog *ui;
	snowstar::RepositoriesPrx	_repositories;
	downloadlist	_filelist;
	int	_counter;
	downloadthread	*thread;
};

} // namespace snowgui

#endif // SNOWGUI_EXPOSEDOWNLOADDIALOG_H
