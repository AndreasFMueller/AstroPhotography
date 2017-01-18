#ifndef DOWNLOADDIALOG_H
#define DOWNLOADDIALOG_H

#include <QDialog>
#include <downloadparameters.h>
#include <tasks.hh>

namespace Ui {
class DownloadDialog;
}

class DownloadDialog : public QDialog {
	Q_OBJECT

	DownloadParameters&	parameters;

public:
	explicit DownloadDialog(DownloadParameters& _parameters,
		QWidget *parent = 0);
	~DownloadDialog();

public slots:
	void	acceptedSlot();
	void	textChanged(const QString& text);
	void	dateToggled(bool checked);
	void	exposureToggled(bool checked);
	void	binningToggled(bool checked);
	void	lightToggled(bool checked);
	void	filterToggled(bool checked);
	void	temperatureToggled(bool checked);

private:
    Ui::DownloadDialog *ui;
};

#endif // DOWNLOADDIALOG_H
