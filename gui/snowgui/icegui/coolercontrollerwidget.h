/*
 * coolercontrollerwidget.h -- Widget to control a cooler
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef COOLERCONTROLLERWIDGET_H
#define COOLERCONTROLLERWIDGET_H

#include <InstrumentWidget.h>
#include <QTimer>

namespace snowgui {

namespace Ui {
	class coolercontrollerwidget;
}

class coolercontrollerwidget;

/**
 * \brief udpate thread
 *
 * see filterwheelcontrollerwidget for the rationale behind this class
 */
class coolerupdatework : public QObject {
	Q_OBJECT
	coolercontrollerwidget	*_coolercontrollerwidget;
	std::recursive_mutex	_mutex;
public:
	coolerupdatework(coolercontrollerwidget *cc);
	~coolerupdatework();
public slots:
	void	statusUpdate();
};

/**
 * \brief A reusable component to control a cooler
 */
class coolercontrollerwidget : public InstrumentWidget {
	Q_OBJECT

	snowstar::CoolerPrx	_cooler;
	std::vector<std::string>	_cooler_names;
	QThread			*_updatethread;
	coolerupdatework	*_updatework;
public:
	explicit coolercontrollerwidget(QWidget *parent = 0);
	~coolercontrollerwidget();
	virtual void	instrumentSetup(
				astro::discover::ServiceObject serviceobject,
				snowstar::RemoteInstrument instrument);
	virtual void	setupComplete();

	void	statusUpdate();

signals:
	void	setTemperatureReached();
	void	coolerSelected(int);
	void	newCoolerState(float, float, bool);
	void	newActualTemperature(float);

private:
	void	displaySetTemperature(float settemperature);

	void	setupCooler();
	void	coolerFailed(const std::exception&);
	Ui::coolercontrollerwidget *ui;
	QTimer	statusTimer;

	void	sendSetTemperature(double temp);

public slots:
	void	setActual();
	void	setSetTemperature(double t);
	void	displayActualTemperature(float actual);
	void	guiChanged();
	void	coolerChanged(int index);
	void	editingFinished();
	void	activeToggled(bool);
};

} // namespace snowgui

#endif // COOLERCONTROLLERWIDGET_H
