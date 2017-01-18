/*
 * downloadreportdialog.h -- widget to display result of download
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef DOWNLOADREPORTDIALOG_H
#define DOWNLOADREPORTDIALOG_H

#include <QDialog>
#include <downloadparameters.h>

namespace Ui {
class DownloadReportDialog;
}

class DownloadReportDialog : public QDialog
{
	Q_OBJECT

public:
	explicit DownloadReportDialog(const std::list<fileinfo>& files,
		QWidget *parent = 0);
	~DownloadReportDialog();

private:
	Ui::DownloadReportDialog *ui;
};

#endif // DOWNLOADREPORTDIALOG_H
