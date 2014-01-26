/*
 * taskcreator.h -- define TaskCreator widget
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapprerswil
 */
#ifndef TASKCREATOR_H
#define TASKCREATOR_H

#include <QWidget>
#include <connectiondialog.h>
#include <module.hh>
#include <tasks.hh>

namespace Ui {
class TaskCreator;
}

class TaskCreator : public QWidget
{
    Q_OBJECT

	Astro::Modules_var	modules;
	Astro::Camera_var	camera;
	Astro::CcdInfo_var	ccdinfo;

	Astro::DeviceLocator_var	getDeviceLocator(const std::string& name);
	Astro::Camera_var	getCamera(const std::string& cameraname);
	Astro::FilterWheel_var	getFilterwheel(const std::string& filterwheelname);
	Astro::TaskQueue_var	_taskqueue;
public:
	void	taskqueue(Astro::TaskQueue_var t) {
		_taskqueue = Astro::TaskQueue::_duplicate(t);
	}

public:
    explicit TaskCreator(QWidget *parent = 0);
    ~TaskCreator();

public slots:
	void	selectCamera(int cameraposition);
	void	selectFilterwheel(int filterwheelposition);
	void	selectCcd(int ccdid);
	void	submitTask(int multiplicity);

private:
    Ui::TaskCreator *ui;
};

#endif // TASKCREATOR_H
