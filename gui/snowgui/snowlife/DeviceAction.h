/*
 * DeviceAction.h -- menu actions that open devices
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _DeviceAction_h
#define _DeviceAction_h

#include <QAction>

namespace snowgui {

class DeviceAction : public QAction {
	Q_OBJECT
	std::string	_devicename;
public:
	explicit DeviceAction(const std::string& devicename, QString text,
		QObject *parent = NULL);
	virtual ~DeviceAction();
signals:
	void	openDevice(std::string);
public slots:
	void	doopen();
};

} // namespace snowgui

#endif /* _DeviceAction_h */
