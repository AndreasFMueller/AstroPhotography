/* 
 * browserwindow.h -- browse images for artefacts
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef SNOWGUI_BROWSERWINDOW_H
#define SNOWGUI_BROWSERWINDOW_H

#include <QSplitter>
#include <QTreeWidgetItem>

namespace snowgui {

namespace Ui {
	class browserwindow;
}

class browserwindow : public QSplitter {
	Q_OBJECT

	std::string	_directory;
public:
	explicit browserwindow(QWidget *parent = 0);
	~browserwindow();

	const std::string&	directory() const { return _directory; }
	void	setDirectory(const std::string& d);

public slots:
	void	markClicked();
	void	invertSelectionClicked();
	void	selectAllClicked();
	void	currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*);

private:
	Ui::browserwindow *ui;

	void	markSubdirectory(const std::string& subdirectory);
	void	markPrefix(const std::string& prefix);
	void	makedirectory(const std::string& directory);
};

} // namespace snowgui

#endif // SNOWGUI_BROWSERWINDOW_H
