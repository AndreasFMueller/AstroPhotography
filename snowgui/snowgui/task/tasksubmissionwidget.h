#ifndef SNOWGUI_TASKSUBMISSIONWIDGET_H
#define SNOWGUI_TASKSUBMISSIONWIDGET_H

#include <QWidget>

namespace snowgui {

namespace Ui {
class tasksubmissionwidget;
}

class tasksubmissionwidget : public QWidget
{
    Q_OBJECT

public:
    explicit tasksubmissionwidget(QWidget *parent = 0);
    ~tasksubmissionwidget();

private:
    Ui::tasksubmissionwidget *ui;
};


} // namespace snowgui
#endif // SNOWGUI_TASKSUBMISSIONWIDGET_H
