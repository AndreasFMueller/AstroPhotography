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
 * \brief The cooler callback implementation class
 */
class CoolerCallbackI : public snowstar::CoolerCallback {
	coolercontrollerwidget&	_coolercontrollerwidget;
public:
	CoolerCallbackI(coolercontrollerwidget& c);
	~CoolerCallbackI();

	void	updateCoolerInfo(const snowstar::CoolerInfo& info,
				const Ice::Current& current);

	void	updateSetTemperature(float settemperature,
				const Ice::Current& current);

	void	updateDewHeater(float dewheater,
                                const Ice::Current& current);
};

/**
 * \brief A reusable component to control a cooler
 */
class coolercontrollerwidget : public InstrumentWidget {
	Q_OBJECT

	snowstar::CoolerPrx	_cooler;
	std::vector<std::string>	_cooler_names;

	Ice::ObjectPtr	_cooler_callback;
	Ice::Identity	_cooler_identity;

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
};

} // namespace snowgui

#endif // COOLERCONTROLLERWIDGET_H
