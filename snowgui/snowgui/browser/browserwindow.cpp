/*
 * browserwindow.cpp -- implementation
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "browserwindow.h"
#include "ui_browserwindow.h"
#include <QDirIterator>
#include <QDateTime>
#include <QCheckBox>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <AstroIO.h>
#include "markingmethoddialog.h"
#include <sys/stat.h>

namespace snowgui {

/**
 * \brief Construct a new browser window
 */
browserwindow::browserwindow(QWidget *parent)
	: QWidget(parent), ui(new Ui::browserwindow) {
	ui->setupUi(this);

	QStringList	headers;
	headers << "OK";
	headers << "Filename";
	headers << "Size";
	headers << "Date";
	ui->fileTree->setHeaderLabels(headers);
	ui->fileTree->header()->resizeSection(0, 40);
	ui->fileTree->header()->resizeSection(1, 150);
	ui->fileTree->header()->resizeSection(2, 80);

	connect(ui->markButton, SIGNAL(clicked()),
		this, SLOT(markClicked()));
	connect(ui->selectallButton, SIGNAL(clicked()),
		this, SLOT(selectAllClicked()));
	connect(ui->invertselectionButton, SIGNAL(clicked()),
		this, SLOT(invertSelectionClicked()));
	connect(ui->fileTree,
		SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
		this,
		SLOT(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)));
}

/**
 * \brief destroy the browser window
 */
browserwindow::~browserwindow() {
	delete ui;
}

/**
 * \brief Scan the directory for FITS files and display file info in the list
 */
void	browserwindow::setDirectory(const std::string& d) {
	_directory = d;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "working on directory %s", d.c_str());

	// empty the tree
	while (ui->fileTree->topLevelItemCount()) {
		delete ui->fileTree->takeTopLevelItem(0);
	}

	// read the directory and fill the tree again
	QStringList	namefilters;
	namefilters << "*.fits";
	QDirIterator	dirit(QString(_directory.c_str()), namefilters);
	while (dirit.hasNext()) {
		dirit.next();
		QFileInfo	info = dirit.fileInfo();
		QStringList	list;
		list << "";
		list << info.fileName();
		list << QString::number(info.size());
		list << info.lastModified().toString("yyyy-MM-dd hh:mm::s");
		QTreeWidgetItem	*item = new QTreeWidgetItem(list,
			QTreeWidgetItem::Type);
		item->setTextAlignment(0, Qt::AlignLeft);
		item->setTextAlignment(1, Qt::AlignLeft);
		item->setTextAlignment(2, Qt::AlignRight);
		item->setTextAlignment(3, Qt::AlignLeft);
		ui->fileTree->addTopLevelItem(item);
		QCheckBox	*checkbox = new QCheckBox();
		checkbox->setChecked(true);
		ui->fileTree->setItemWidget(item, 0, checkbox);
	}
}

/**
 * \brief Slot used to handle click on the mark button
 */
void	browserwindow::markClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "markClicked()");
	markingmethoddialog	md;
	if (md.exec()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "marking fields accepted");
		switch (md.method()) {
		case MarkSubdirectory:
			markSubdirectory(md.subdirectory());
			break;
		case MarkPrefix:
			markPrefix(md.prefix());
			break;
		}
	}
}

/**
 * \brief Display a file
 */
void	browserwindow::currentItemChanged(QTreeWidgetItem *current,
		QTreeWidgetItem *) {
	std::string	filename(current->text(1).toLatin1().data());
	std::string	path = astro::stringprintf("%s/%s",
				_directory.c_str(), filename.c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "open file: %s", path.c_str());
	ImagePtr	image;
	try {
		astro::io::FITSin	infile(path);
		image = infile.read();
	} catch (const std::exception& x) {
		// XXX error dialog
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot read file: %s",
			x.what());
		return;
	}
	ui->imageWidget->receiveImage(image);
	std::string	title = astro::stringprintf("Browse: %s",
				filename.c_str());
	setWindowTitle(QString(title.c_str()));
}

/**
 * \brief Slot to invert the current selection
 */
void	browserwindow::invertSelectionClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "invert selection clicked");
	for (int i = 0; i < ui->fileTree->topLevelItemCount(); i++) {
		QTreeWidgetItem	*item = ui->fileTree->topLevelItem(i);
		QWidget	*widget = ui->fileTree->itemWidget(item, 0);
		QCheckBox	*box = (QCheckBox *)widget;
		box->setChecked(!box->isChecked());
	}
}

/**
 * \brief Slot to select all files
 */
void	browserwindow::selectAllClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "select all clicked");
	for (int i = 0; i < ui->fileTree->topLevelItemCount(); i++) {
		QTreeWidgetItem	*item = ui->fileTree->topLevelItem(i);
		QWidget	*widget = ui->fileTree->itemWidget(item, 0);
		QCheckBox	*box = (QCheckBox *)widget;
		box->setChecked(true);
	}
}

/**
 * \brief Auxiliary function to create a directory
 */
void	browserwindow::makedirectory(const std::string& subdirectory) {
	if (mkdir(subdirectory.c_str(), 0777) < 0) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot create %s: %s",
			subdirectory.c_str(), strerror(errno));
	}
}

/**
 * \brief Mark files by moving them to a subdirectory
 */
void	browserwindow::markSubdirectory(const std::string& subdirectory) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "moving to subdirectory: %s",
		subdirectory.c_str());
	// make sure the subdirectory exsits
	std::string	subdirpath = _directory + "/" + subdirectory;
	struct stat	sb;
	int	rc;
	if ((rc = stat(subdirpath.c_str(), &sb)) < 0) {
		if (errno == ENOENT) {
			makedirectory(subdirpath);
		}
	}
	if ((rc = stat(subdirpath.c_str(), &sb)) < 0) {
		debug(LOG_ERR, DEBUG_LOG, 0, "directory not found: %s",
			strerror(errno));
		return;
	}
	if (!(sb.st_mode & S_IFDIR)) {
		debug(LOG_ERR, DEBUG_LOG, 0, "not a directory");
		return;
	}

	// now move the files
	for (int i = 0; i < ui->fileTree->topLevelItemCount(); i++) {
		QTreeWidgetItem	*item = ui->fileTree->topLevelItem(i);
		QWidget	*widget = ui->fileTree->itemWidget(item, 0);
		QCheckBox	*box = (QCheckBox *)widget;
		if (!box->isChecked()) {
			std::string	filename
				= item->text(1).toLatin1().data();
			std::string	frompath = _directory + "/" + filename;
			std::string	topath = _directory + "/"
				+ subdirectory + "/" + filename;
			rc = rename(frompath.c_str(), topath.c_str());
			if (rc < 0) {
				// XXX cannot rename
				debug(LOG_ERR, DEBUG_LOG, 0,
					"cannot rename: %s", strerror(errno));
			}
		}
	}
}

/**
 * \brief mark bad files by prefixing them with a prefix
 */
void	browserwindow::markPrefix(const std::string& prefix) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "marking with prefix: %s",
		prefix.c_str());
	for (int i = 0; i < ui->fileTree->topLevelItemCount(); i++) {
		QTreeWidgetItem	*item = ui->fileTree->topLevelItem(i);
		QWidget	*widget = ui->fileTree->itemWidget(item, 0);
		QCheckBox	*box = (QCheckBox *)widget;
		if (!box->isChecked()) {
			std::string	filename
				= item->text(1).toLatin1().data();
			std::string	frompath = _directory + "/" + filename;
			std::string	topath = _directory + "/" + prefix + filename;
			int	rc = rename(frompath.c_str(), topath.c_str());
			if (rc < 0) {
				// XXX cannot rename
				debug(LOG_ERR, DEBUG_LOG, 0,
					"cannot rename: %s", strerror(errno));
			}
		}
	}
}

} // namespace snowgui
