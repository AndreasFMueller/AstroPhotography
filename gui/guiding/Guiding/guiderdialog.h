#ifndef GUIDERDIALOG_H
#define GUIDERDIALOG_H

#include <guider.hh>

#include <QDialog>

namespace Ui {
class GuiderDialog;
}

class GuiderDialog : public QDialog
{
    Q_OBJECT

	Astro::Guider_var	_guider;

public:
    explicit GuiderDialog(Astro::Guider_var guider, QWidget *parent = 0);
    ~GuiderDialog();

	Astro::Guider_var	guider() { return _guider; }

private:
    Ui::GuiderDialog *ui;
};

#endif // GUIDERDIALOG_H
