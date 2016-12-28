#include "tasksubmissionwidget.h"
#include "ui_tasksubmissionwidget.h"

namespace snowgui {

tasksubmissionwidget::tasksubmissionwidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::tasksubmissionwidget)
{
    ui->setupUi(this);
}

tasksubmissionwidget::~tasksubmissionwidget()
{
    delete ui;
}

} // namespace snowgui
