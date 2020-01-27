/*
 * repositoryconfigurationwidget.cpp
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "repositoryconfigurationwidget.h"
#include "ui_repositoryconfigurationwidget.h"
#include <AstroDebug.h>
#include <QMessageBox>
#include <QCheckBox>
#include <QTableWidgetItem>
#include "repoenablebox.h"

namespace snowgui {

/**
 * \brief Construct a new repositoryconfiguration widget
 */
repositoryconfigurationwidget::repositoryconfigurationwidget(QWidget *parent)
	: QWidget(parent), ui(new Ui::repositoryconfigurationwidget) {
	ui->setupUi(this);

	QStringList	headers;
	headers << "Repository" << "enabled" << "Directory";
	ui->repositoryTable->setHorizontalHeaderLabels(headers);
	ui->repositoryTable->horizontalHeader()->setStretchLastSection(true);

	connect(ui->createButton, SIGNAL(clicked()),
		this, SLOT(createClicked()));
	connect(ui->repositoryDirectoryField, SIGNAL(textChanged(QString)),
		this, SLOT(pathChanged(QString)));
	connect(ui->repositoryNameField, SIGNAL(textChanged(QString)),
		this, SLOT(reponameChanged(QString)));
};

/**
 * \brief Destroy the repositoryconfiguration widget
 */
repositoryconfigurationwidget::~repositoryconfigurationwidget() {
	delete ui;
}

/**
 * \brief Set the repositories proxy
 */
void	repositoryconfigurationwidget::setRepositories(
		snowstar::RepositoriesPrx repositories) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got repository proxy");
	_repositories = repositories;

	// read repositories
	readRepositories();
}

/**
 * \brief Set the daemon proxy
 */
void	repositoryconfigurationwidget::setDaemon(
		snowstar::DaemonPrx daemon) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got demon proxy");
	_daemon = daemon;
}

/**
 * \brief 
 */
void	repositoryconfigurationwidget::readRepositories() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "reading repositories");
	if (!_repositories) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no repositories proxy");
		return;
	}
	snowstar::reposummarylist	l = _repositories->summarylist();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "repositories found: %d", l.size());
	ui->repositoryTable->setRowCount(l.size());
	int	row = 0;
	for (auto ptr = l.begin(); ptr != l.end(); ptr++) {
		snowstar::RepositorySummary	summary = *ptr;
		std::string	reponame = summary.name;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "found repository: %s",
			summary.name.c_str());

		ui->repositoryTable->setRowHeight(row, 19);

		QTableWidgetItem	*i;
		i = new QTableWidgetItem(reponame.c_str());
		ui->repositoryTable->setItem(row, 0, i);

		i = new QTableWidgetItem();
		repoenablebox	*box = new repoenablebox(NULL);
		box->reponame(reponame);
		box->setRepositories(_repositories);
		bool	enabled = !summary.hidden;
		box->setChecked(enabled);
		ui->repositoryTable->setItem(row, 1, i);
		ui->repositoryTable->setCellWidget(row, 1, box);
		connect(box, SIGNAL(toggled(bool)),
			box, SLOT(enableToggled(bool)));

		i = new QTableWidgetItem(summary.directory.c_str());
		ui->repositoryTable->setItem(row, 2, i);
		row++;
	}
	ui->repositoryTable->resizeColumnsToContents();
}

/**
 * \brief 
 */
void	repositoryconfigurationwidget::createClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create clicked");
	if (!_repositories) {
		return;
	}

	std::string	directory
		= ui->repositoryDirectoryField->text().toLatin1().data();
	std::string	reponame
		= ui->repositoryNameField->text().toLatin1().data();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create new directory: %s/%s",
		reponame.c_str(), directory.c_str());

	if (_repositories->has(reponame)) {
		QMessageBox	message(this);
		message.setText(QString("Repository exists"));
		std::ostringstream      out;
		out << "The repository '" << reponame << "' already exists.";
		message.setInformativeText(QString(out.str().c_str()));
		message.exec();
		return;
	}

	try {
		_repositories->add(reponame, directory);
	} catch (const std::exception& x) {
		QMessageBox	message(this);
		message.setText("Repository creation failed");
		std::ostringstream      out;
		out << "The repository '" << reponame;
		out << "' could not be created. An exception was thrown. ";
		out << "The cause of the exception was: ";
		out << x.what();
		message.setInformativeText(QString(out.str().c_str()));
		message.exec();
		return;
	}

	ui->repositoryDirectoryField->setText(QString());
	ui->repositoryNameField->setText(QString());

	readRepositories();
}

/**
 * \brief The path configuration has changed
 */
void	repositoryconfigurationwidget::pathChanged(QString path) {
	std::string	dirname(path.toLatin1().data());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new path: %s", dirname.c_str());

	// files
	try {
		snowstar::FileInfo	fileinfo = _daemon->statFile(dirname);
		ui->createButton->setEnabled(false);
		return;
	} catch (const std::exception& x) {
		// this is not a file
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s is not a file",
			dirname.c_str());
	}

	// existing directories
	try {
		snowstar::DirectoryInfo dirinfo
			= _daemon->statDirectory(dirname);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "found a directory");
		if (dirinfo.writeable) {
			ui->createButton->setText(QString("Open"));
			ui->createButton->setEnabled(true);
		} else {
			ui->createButton->setEnabled(false);
		}
		return;
	} catch (const std::exception& x) {
		// this is not a directory
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s is not a directory",
			dirname.c_str());
	}

	// parent directory
	size_t	l = dirname.rfind('/');
	if (std::string::npos != l) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "l = %d", l);
		std::string     d = dirname.substr(0, l);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "dirname = '%s'",
			dirname.c_str());
		std::string     f = dirname.substr(l + 1);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "filename = '%s'",
			f.c_str());
		if (f.size() == 0) {
			ui->createButton->setEnabled(false);
			debug(LOG_DEBUG, DEBUG_LOG, 0, "just a directory name");
			return;
		}
		dirname = d;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "check directory '%s'",
		dirname.c_str());

	// check whether the directory is writable
	try {
		snowstar::DirectoryInfo dirinfo
			= _daemon->statDirectory(dirname);
		if (dirinfo.writeable) {
			ui->createButton->setText("Create");
			ui->createButton->setEnabled(true);
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"found a creatable file %s", dirname.c_str());
			return;
		}
	} catch (const std::exception& x) {
		// this file is not writable
	}       
	ui->createButton->setEnabled(false);
}

void	repositoryconfigurationwidget::reponameChanged(QString reponame) {
	std::string	rn(reponame.toLatin1().data());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "checking repo name: %s", rn.c_str());
	if (!_repositories) {
		return;
	}
	try {
		if (_repositories->has(rn)) {
			ui->createButton->setEnabled(false);
		} else {
			ui->createButton->setEnabled(true);
		}
	} catch (...) {
	}
}

}
