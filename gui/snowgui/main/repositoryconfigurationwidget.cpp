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

}
