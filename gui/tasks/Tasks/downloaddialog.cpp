#include "downloaddialog.h"
#include "ui_downloaddialog.h"
#include <AstroDebug.h>

DownloadDialog::DownloadDialog(DownloadParameters& _parameters, QWidget *parent) :
    QDialog(parent),
	parameters(_parameters),
    ui(new Ui::DownloadDialog)
{
    ui->setupUi(this);
	//TaskMainWindow	*mainwindow = (TaskMainWindow *)parent;
	connect(this, SIGNAL(accepted()),
		this, SLOT(acceptedSlot()));
	connect(this, SIGNAL(accepted()),
		parent, SLOT(downloadParametersAccepted()),
		Qt::QueuedConnection);
}

DownloadDialog::~DownloadDialog()
{
    delete ui;
}

void	DownloadDialog::textChanged(const QString& text) {
	parameters.prefix = text;
}

void	DownloadDialog::dateToggled(bool checked) {
	parameters.date = checked;
}

void	DownloadDialog::exposureToggled(bool checked) {
	parameters.exposuretime = checked;
}

void	DownloadDialog::binningToggled(bool checked) {
	parameters.binning = checked;
}

void	DownloadDialog::lightToggled(bool checked) {
	parameters.shutter = checked;
}

void	DownloadDialog::filterToggled(bool checked) {
	parameters.filter = checked;
}

void	DownloadDialog::temperatureToggled(bool checked) {
	parameters.temperature = checked;
}

void	DownloadDialog::acceptedSlot() {
	parameters.prefix = ui->prefixField->text();
}
