/*
 * instrumentselectionialog.h -- basic instrument selection
 *
 * There will be dialogs that subclass this dialog to start different
 * applications
 */
#ifndef INSTRUMENTSELECTIONDIALOG_H
#define INSTRUMENTSELECTIONDIALOG_H

#include <QDialog>
#include <AstroDiscovery.h>
#include <instruments.h>
#include <RemoteInstrument.h>

namespace Ui {
	class InstrumentSelectionDialog;
}

class InstrumentSelectionDialog : public QDialog {
	Q_OBJECT
protected:
	astro::discover::ServiceObject	_serviceobject;
	snowstar::InstrumentsPrx	instruments;

public:
	explicit InstrumentSelectionDialog(QWidget *parent,
		astro::discover::ServiceObject serviceobject);
	~InstrumentSelectionDialog();

	virtual void	launch(const std::string& instrumentname);

public slots:
        void    accept();

private:
	Ui::InstrumentSelectionDialog *ui;


};

/**
 * \brief Template to launch the right application
 */
template<typename application>
class InstrumentSelectionApplication : public InstrumentSelectionDialog {
public:
	InstrumentSelectionApplication(QWidget *parent,
		astro::discover::ServiceObject serviceobject)
		: InstrumentSelectionDialog(parent, serviceobject) {
	}
	virtual void	launch(const std::string& instrumentname) {
		snowstar::RemoteInstrument	ri(instruments, instrumentname);
		application	*a = new application(parent, _serviceobject,
						ri);
		a->show();
	}
};

#endif // INSTRUMENTSELECTIONDIALOG_H
