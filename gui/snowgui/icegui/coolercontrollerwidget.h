/*
 * coolercontrollerwidget.h -- Widget to control a cooler
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef COOLERCONTROLLERWIDGET_H
#define COOLERCONTROLLERWIDGET_H

#include <InstrumentWidget.h>
#include <QTimer>
#include "CallbackIdentity.h"

namespace snowgui {

namespace Ui {
	class coolercontrollerwidget;
}

class coolercontrollerwidget;

/**
 * \brief The cooler callback implementation class
 */
class CoolerCallbackI : public QObject, public snowstar::CoolerCallback,
			public CallbackIdentity {
	Q_OBJECT

public:
	CoolerCallbackI();
	~CoolerCallbackI();

	void	updateCoolerInfo(const snowstar::CoolerInfo& info,
				const Ice::Current& current);

	void	updateSetTemperature(float settemperature,
				const Ice::Current& current);

	void	updateDewHeater(float dewheater,
                                const Ice::Current& current);
signals:
	void	callbackCoolerInfo(snowstar::CoolerInfo);
	void	callbackSetTemperature(float settemperature);
	void	callbackDewHeater(float dewheater);
};

/**
 * \brief A reusable component to control a cooler
 */
class coolercontrollerwidget : public InstrumentWidget {
	Q_OBJECT

	snowstar::CoolerPrx	_cooler;
	std::vector<std::string>	_cooler_names;

	Ice::ObjectPtr	_cooler_callback;
	Ice::Identity	identity();

	std::pair<float,float>	_dewheaterinterval;
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
	void	newSetTemperature(float);

private:

	void	setupCooler();
	void	coolerFailed(const std::exception&);
	Ui::coolercontrollerwidget *ui;
	QTimer	statusTimer;

	void	sendSetTemperature(double temp);

public slots:
	void	setActual();
	void	setSetTemperature(double t);
	void	displayActualTemperature(float actual);
	void	displaySetTemperature(float settemperature);
	void	guiChanged();
	void	dewHeaterChanged(int);
	void	coolerChanged(int index);
	void	editingFinished();
	void	activeToggled(bool);

	// set the dewheater state
	void	setDewHeater(float dewheatervalue);
	void	setDewHeaterSlider(float dewheatervalue);

	// callback slots
	void	callbackCoolerInfo(snowstar::CoolerInfo);
	void	callbackSetTemperature(float settemperature);
	void	callbackDewHeater(float dewheater);
};

} // namespace snowgui

#endif // COOLERCONTROLLERWIDGET_H
