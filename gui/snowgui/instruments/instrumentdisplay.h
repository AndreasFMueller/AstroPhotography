/*
 * instrumentdisplay.h
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef SNOWGUI_INSTRUMENTDISPLAY_H
#define SNOWGUI_INSTRUMENTDISPLAY_H

#include <QWidget>
#include <instruments.h>

namespace snowgui {

namespace Ui {
	class instrumentdisplay;
}

class instrumentdisplay : public QWidget {
	Q_OBJECT

public:
	explicit instrumentdisplay(QWidget *parent = 0);
	~instrumentdisplay();

	void	setInstrument(snowstar::InstrumentPrx instrument);

	void	add(const std::string& devicename,
			const std::string& servicename);
	void	addGuiderCCD(const std::string& devicename,
			const std::string& servicename);
	void	addFinderCCD(const std::string& devicename,
			const std::string& servicename);
	void	deleteSelected();

	void	redisplay();

public slots:
	void	propertyValueChanged(int row, int column);

private:
	Ui::instrumentdisplay *ui;
	snowstar::InstrumentPrx	_instrument;
	void 	toplevel(snowstar::InstrumentComponentType type,
			const std::string& name);
	void	alltoplevel();
	void	children(snowstar::InstrumentComponentType type);
	void	allchildren();
	void	property(int row, const std::string& propertyname,
			const std::string& description);
	void	allproperties();
};


} // namespace snowgui
#endif // SNOWGUI_INSTRUMENTDISPLAY_H
