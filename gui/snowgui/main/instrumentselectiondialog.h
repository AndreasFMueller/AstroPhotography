/*
 * instrumentselectionialog.h -- basic instrument selection
 *
 * There will be dialogs that subclass this dialog to start different
 * applications
 */
#ifndef INSTRUMENTSELECTIONDIALOG_H
#define INSTRUMENTSELECTIONDIALOG_H

#include <QDialog>
#include <QApplication>
#include <AstroDiscovery.h>
#include <AstroDebug.h>
#include <instruments.h>
#include <RemoteInstrument.h>

namespace snowgui {

namespace Ui {
	class InstrumentSelectionDialog;
}

class InstrumentWidget;

/**
 * \brief create an instrument selection dialog
 */
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
	void	launch(const std::string& instrumentname, InstrumentWidget *a);

public slots:
        void    accept();

private:
	Ui::InstrumentSelectionDialog *ui;
};

class MainWindow;

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
		application	*a = new application(NULL);
#if 0
		snowstar::RemoteInstrument	ri(instruments, instrumentname);
		// get the main window and connect the offerImage signal
		// to the imageForSaving option
		debug(LOG_DEBUG, DEBUG_LOG, 0, "connect offerImage()");

		// start the instrument setup thread
		InstrumentSetupThread	*setupthread
			= new InstrumentSetupThread(a, _serviceobject, ri);
		setupthread->start();

		// make the application visible
		a->show();
		QApplication::setActiveWindow(a);
		a->raise();
#endif
		InstrumentSelectionDialog::launch(instrumentname, (InstrumentWidget *)a);
	}
};

} // namespace snowgui

#endif // INSTRUMENTSELECTIONDIALOG_H
