#ifndef CONNECTIONDIALOG_H
#define CONNECTIONDIALOG_H

#include <QDialog>

namespace snowgui {

namespace Ui {
	class ConnectionDialog;
}

class ConnectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConnectionDialog(QWidget *parent = 0);
    ~ConnectionDialog();

private:
    Ui::ConnectionDialog *ui;
};

} // namespace snowgui

#endif // CONNECTIONDIALOG_H
