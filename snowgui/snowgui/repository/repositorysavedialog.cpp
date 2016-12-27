/*
 * repositorysavedialog.cpp -- 
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "repositorysavedialog.h"
#include "ui_repositorysavedialog.h"
#include <AstroDebug.h>
#include <QPushButton>

namespace snowgui {

/**
 * \brief Construct a new repository savedialog
 */
repositorysavedialog::repositorysavedialog(QWidget *parent) 
	: QDialog(parent), ui(new Ui::repositorysavedialog) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "constructing repository save dialog");
	ui->setupUi(this);

	thread = NULL;

	connect(ui->buttonBox->button(QDialogButtonBox::Cancel),
		SIGNAL(clicked()),
		this,
		SLOT(reject()));
}

/**
 * \brief Destroy a repository savedialog
 */
repositorysavedialog::~repositorysavedialog() {
	delete ui;
}

/**
 * \brief get a list of items to download
 */
void	repositorysavedialog::set(const std::string& directory,
		snowstar::RepositoriesPrx repositories,
		const std::list<std::pair<std::string, int> >& images) {
	// directory
	_directory = directory;

	// set the repositories
	_repositories = repositories;
	if (!_repositories) {
		return;
	}

	// set images
	_images = images;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got %d images to download",
		_images.size());
	ui->totalField->setText(QString(astro::stringprintf("%d",
		_images.size()).c_str()));
	_counter = 0;

	// set the window title
	std::string	title = astro::stringprintf("save %d images to %s",
				_images.size(), _directory.c_str());
	setWindowTitle(QString(title.c_str()));

	// start the thread that processes the images
	thread = new savethread(this);
	qRegisterMetaType<downloadstatus>("downloadstatus");
	connect(thread,
		SIGNAL(sendStatus(downloadstatus)),
		this,
		SLOT(updateStatus(downloadstatus)));
	connect(thread,
		SIGNAL(downloadComplete()),
		this,
		SLOT(downloadComplete()));
	connect(thread,
		SIGNAL(downloadAborted()),
		this,
		SLOT(downloadAborted()));
	thread->set(_directory, _repositories, _images);
}

void	repositorysavedialog::reject() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "reject");
	if (thread) {
		thread->stopProcess();
		if (thread->isRunning()) {
			thread->wait();
		}
		if (thread->errormsg().size() > 0) {
			// XXX show an error message dialog
		}
		delete thread;
		thread = NULL;
	}
	QDialog::reject();
}

void	repositorysavedialog::accept() {
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

void	repositorysavedialog::updateStatus(downloadstatus s) {
	ui->counterField->setText(QString(astro::stringprintf("%d",
		++_counter).c_str()));
	ui->repositoryField->setText(s._reponame);
	ui->imageIdField->setText(QString(astro::stringprintf("%d",
		s._imageid).c_str()));
}

void	repositorysavedialog::downloadComplete() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "download complete");
	accept();
}

void	repositorysavedialog::downloadAborted() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "download aborted");
	reject();
}



} // namespace snowgui
