#ifndef CALIBRATIONSELECTIONDIALOG_H
#define CALIBRATIONSELECTIONDIALOG_H

#include <QWidget>

namespace Ui {
class calibrationselectiondialog;
}

class calibrationselectiondialog : public QWidget
{
    Q_OBJECT

public:
    explicit calibrationselectiondialog(QWidget *parent = 0);
    ~calibrationselectiondialog();

private:
    Ui::calibrationselectiondialog *ui;
};

#endif // CALIBRATIONSELECTIONDIALOG_H
