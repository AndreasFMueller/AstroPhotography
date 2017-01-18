#ifndef GUIDEROPENDIALOG_H
#define GUIDEROPENDIALOG_H

#include <QDialog>

namespace Ui {
class GuiderOpenDialog;
}

class GuiderOpenDialog : public QDialog
{
	Q_OBJECT

public:
	explicit GuiderOpenDialog(QWidget *parent = 0);
	~GuiderOpenDialog();

public slots:
	void	accept();

private:
	Ui::GuiderOpenDialog *ui;
};

#endif // GUIDEROPENDIALOG_H
