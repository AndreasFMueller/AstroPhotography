/*
 * repositorysavedialog.h -- dialog to save images from repository
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef SNOWGUI_REPOSITORYSAVEDIALOG_H
#define SNOWGUI_REPOSITORYSAVEDIALOG_H

#include <QDialog>
#include <AstroDiscovery.h>
#include <repository.h>
#include <QTreeWidgetItem>
#include "savethread.h"

namespace snowgui {

namespace Ui {
	class repositorysavedialog;
}

class repositorysavedialog : public QDialog {
	Q_OBJECT

public:
	repositorysavedialog(QWidget *parent);
	~repositorysavedialog();

	void	set(const std::string& directory,
			snowstar::RepositoriesPrx,
			const std::list<std::pair<std::string, int> >& images);
private:
	Ui::repositorysavedialog *ui;
	snowstar::RepositoriesPrx	_repositories;
	std::string	_directory;
	std::list<std::pair<std::string, int> >	_images;
	savethread	*thread;
	int	_counter;

public slots:
	void	reject();
	void	accept();
	void	updateStatus(downloadstatus);
	void	downloadComplete();
	void	downloadAborted();

protected:
};


} // namespace snowgui
#endif // SNOWGUI_REPOSITORYSAVEDIALOG_H
