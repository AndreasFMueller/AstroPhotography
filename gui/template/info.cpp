#include "info.h"
#include "ui_info.h"

Info::Info(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Info)
{
    ui->setupUi(this);
}

Info::~Info()
{
    delete ui;
}

void	Info::setText(const std::string& text) {
	ui->plainTextEdit->setPlainText(QString(text.c_str()));
}
