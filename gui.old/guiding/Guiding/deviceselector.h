/*
 * deviceselector.h -- ComboBox to select device from a server
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _deviceselector_h
#define _deviceselector_h

#include <QComboBox>
#include <module.hh>

class DeviceSelector : public QComboBox {
	Q_OBJECT

private:

public:
	DeviceSelector(QWidget *parent = NULL);
	virtual ~DeviceSelector();

	void	set(Astro::Modules_var& modules,
			const Astro::DeviceLocator::device_type& type);

};

#endif /* _deviceselector_h */
