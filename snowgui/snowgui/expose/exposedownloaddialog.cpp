/*
 * exposedownloaddialog.cpp -- implementation of download progress dialog
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "exposedownloaddialog.h"
#include "ui_exposedownloaddialog.h"
#include <AstroDebug.h>
#include <QPushButton>

namespace snowgui {

/**
 * \brief Construct a new dialog
 */
exposedownloaddialog::exposedownloaddialog(QWidget *parent)
	: QDialog(parent), ui(new Ui::exposedownloaddialog) {
	ui->setupUi(this);

	thread = NULL;

	connect(ui->buttonBox->button(QDialogButtonBox::Cancel),
		SIGNAL(clicked()),
		this,
		SLOT(reject()));
}

/**
 * \brief Destroy the dialog
 */
exposedownloaddialog::~exposedownloaddialog() {
	delete ui;
}

/**
 * \brief set the parameters and start the download
 */
void	exposedownloaddialog::set(snowstar::RepositoriesPrx repositories,
			const downloadlist& filelist) {
	ui->totalField->setText(QString::number(filelist.size()));
	ui->numberField->setText(QString(""));
	thread = new downloadthread(this);
	qRegisterMetaType<downloaditem>("downloaditem");
	connect(thread,
		SIGNAL(sendStatus(downloaditem)),
		this,
		SLOT(updateStatus(downloaditem)));
	connect(thread,
		SIGNAL(downloadComplete()),
		this,
		SLOT(downloadComplete()));
	connect(thread,
		SIGNAL(downloadAborted()),
		this,
		SLOT(downloadAborted()));
	thread->set(repositories, filelist);
}

/**
 * \brief reject the dialog
 */
void	exposedownloaddialog::reject() {
	if (thread) {
		thread->stopProcess();
		if (thread->isRunning()) {
			thread->wait();
		}
		if (thread->errormsg().size() > 0) {
			// XXX show error message dialog
		}
		delete thread;
		thread = NULL;
	}
	QDialog::reject();
}

/**
 * \brief accdept the dialog
 */
void	exposedownloaddialog::accept() {
	if (thread) {
		thread->stopProcess();
		if (thread->isRunning()) {
			thread->wait();
		}
		delete thread;
		thread = NULL;
	}
	QDialog::accept();
}

/**
 * \brief displaying a status update
 */
void	exposedownloaddialog::updateStatus(downloaditem item) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new download item");
	ui->numberField->setText(QString::number(++_counter));
	ui->imageidField->setText(QString::number(item.imageid()));
	ui->repositoryField->setText(item._reponame);
}

/**
 * \brief Slot to handle completion of the download
 */
void	exposedownloaddialog::downloadComplete() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "download complete");
	accept();
}

/**
 * \brief Slot to handle aborting the download
 */
void	exposedownloaddialog::downloadAborted() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "download avorted");
	reject();
}

} // namespace snowgui
