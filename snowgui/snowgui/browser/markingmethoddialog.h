/*
 * markingmethoddialog.h
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef SNOWGUI_MARKINGMETHODDIALOG_H
#define SNOWGUI_MARKINGMETHODDIALOG_H

#include <QDialog>

namespace snowgui {

typedef enum { MarkSubdirectory, MarkPrefix } MarkingMethod;

namespace Ui {
	class markingmethoddialog;
}

class markingmethoddialog : public QDialog {
	Q_OBJECT

	MarkingMethod	_method;
	std::string	_subdirectory;
	std::string	_prefix;
public:
	explicit markingmethoddialog(QWidget *parent = 0);
	~markingmethoddialog();

	MarkingMethod	method() const { return _method; }
	void	method(MarkingMethod m);

	const std::string&	prefix() const { return _prefix; }
	const std::string&	subdirectory() const { return _subdirectory; }

public slots:
	void	subdirClicked(bool);
	void	prefixClicked(bool);
	void	subdirEditingFinished();
	void	prefixEditingFinished();

private:
	Ui::markingmethoddialog *ui;
};


} // namespace snowgui
#endif // SNOWGUI_MARKINGMETHODDIALOG_H
