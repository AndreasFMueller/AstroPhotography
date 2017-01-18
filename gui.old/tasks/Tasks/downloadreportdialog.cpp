/*
 * downloadreportdialog.cpp -- implement download report dialog
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "downloadreportdialog.h"
#include "ui_downloadreportdialog.h"
#include <AstroDebug.h>

/**
 * \brief Construct a dialog to report about a list of files
 *
 * \param files		a list of files to be displayed in the report
 * \param parent	parent widget
 */
DownloadReportDialog::DownloadReportDialog(const std::list<fileinfo>& files,
	QWidget *parent) :
	QDialog(parent),
	ui(new Ui::DownloadReportDialog)
{
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating report for %d files",
		files.size());
	ui->setupUi(this);

	// compute the total size
	unsigned long	totalsize = 0;
	std::list<fileinfo>::const_iterator	i;
	for (i = files.begin(); i != files.end(); i++) {
		totalsize += i->size;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "total size: %lu", totalsize);

	// update the summary
	if (0 == files.size()) {
		ui->downloadSummary->setText(QString("no files downloaded"));
	} else if (1 == files.size()) {
		ui->downloadSummary->setText(QString("one file downloaded"));
	} else {
		char	buffer[128];
		snprintf(buffer, sizeof(buffer), "%lu files downloaded",
			files.size());
		ui->downloadSummary->setText(QString(buffer));
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "summary set");

	// compute the maximum file name length
	unsigned int	filenamesize = 8;
	for (i = files.begin(); i != files.end(); i++) {
		if (i->name.size() > filenamesize) {
			filenamesize = i->name.size();
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "file name size: %u", filenamesize);

	// add all entries to the list
	for (i = files.begin(); i != files.end(); i++) {
		char	buffer[128];
		snprintf(buffer, sizeof(buffer),
			"%-*.*s  %8ldkB", filenamesize, filenamesize,
			i->name.c_str(), i->size / 1024);
		ui->downloadList->addItem(buffer);
	}
}

/**
 * \brief Destroy the DownloadReportDialog
 */
DownloadReportDialog::~DownloadReportDialog()
{
	delete ui;
}
