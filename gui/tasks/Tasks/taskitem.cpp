/*
 * taskitem.cpp -- TaskItem implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <taskitem.h>
#include <QPainter>

TaskItem::TaskItem(const Astro::TaskInfo& _info,
	const Astro::TaskParameters& _parameters, QWidget *parent) :
	QWidget(parent), info(_info), parameters(_parameters) {
}

TaskItem::~TaskItem() {
}

static QColor	pending_color(224., 224., 255.);
static QColor	executing_color(224., 255., 224.);
static QColor	failed_color(255., 224., 224.);
static QColor	cancelled_color(255., 255., 224.);
static QColor	completed_color(255., 224., 255.);

void	TaskItem::draw() {
	QString	statestring;
	QPainter	painter(this);
	QColor	statecolor;
	switch (info.state) {
	case Astro::TASK_PENDING:
		painter.fillRect(0, 0, height(), height(), pending_color);
		statestring = "pending";
		statecolor = QColor(0, 0, 255);
		break;
	case Astro::TASK_EXECUTING:
		painter.fillRect(0, 0, height(), height(), executing_color);
		statestring = "executing";
		statecolor = QColor(0, 0, 255);
		break;
	case Astro::TASK_FAILED:
		painter.fillRect(0, 0, height(), height(), failed_color);
		statestring = "failed";
		statecolor = QColor(255, 0, 0);
		break;
	case Astro::TASK_CANCELLED:
		painter.fillRect(0, 0, height(), height(), cancelled_color);
		statestring = "cancelled";
		statecolor = QColor(255, 255, 0);
		break;
	case Astro::TASK_COMPLETED:
		painter.fillRect(0, 0, height(), height(), completed_color);
		statestring = "completed";
		statecolor = QColor(0, 128, 0);
		break;
	}

	// task info
	char	buffer[128];
	int	h = height() / 4;
	int	flags = Qt::AlignLeft | Qt::AlignVCenter;

	int	ioff = height() + 5;
	int	wli = 50, wvi = 250;
	int	ivoff = ioff + wli;

	int	poff = ioff + 230;
	int	wlp = 70, wvp = 400;
	int	pvoff = poff + wlp;

	// draw all the labels, because they are in a lighter colour
	QPen	graypen = painter.pen();
	graypen.setColor(QColor(196, 196, 196));
	painter.setPen(graypen);

	// display all the form labels in gray
	painter.drawText(ioff,  0 * h, wli, h, flags, QString("State:"));
	painter.drawText(ioff,  1 * h, wli, h, flags, QString("When:"));
	if (info.state != Astro::TASK_COMPLETED) {
		if (strlen(info.cause.in()) > 0) {
			painter.drawText(ioff,  2 * h, wli, h, flags,
				QString("Cause:"));
		}
	} else {
		painter.drawText(ioff,  2 * h, wli, h, flags, QString("Size:"));
		painter.drawText(ioff,  3 * h, wli, h, flags, QString("File:"));
	}
	painter.drawText(poff,  0 * h, wlp, h, flags, QString("Camera:"));
	painter.drawText(poff,  1 * h, wlp, h, flags, QString("CCD:"));
	QString	templabel("Temperature");
	painter.drawText(pvoff +  40, 1 * h,  90, h, flags, templabel);
	painter.drawText(poff,  2 * h, wlp, h, flags, QString("Exposure:"));
	painter.drawText(poff,  3 * h, wlp, h, flags, QString("Filter:"));

	// switch to state color for the state label
	QPen	statepen = painter.pen();
	statepen.setColor(statecolor);
	painter.setPen(statepen);
	painter.drawText(ivoff, 0 * h, wvi, h, flags, statestring);

	// switch back to black
	QPen	blackpen = painter.pen();
	blackpen.setColor(QColor(0, 0, 0));
	painter.setPen(blackpen);

	time_t	t = info.lastchange;
	struct tm	*tmp = localtime(&t);
	strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tmp);
	QString	whenstring(buffer);

	painter.drawText(ivoff, 1 * h, wvi, h, flags, whenstring);

	if (info.state != Astro::TASK_COMPLETED) {
		if (strlen(info.cause.in()) > 0) {
			QString	cause(info.cause.in());
			painter.drawText(ivoff, 2 * h, wvi, h, flags,
				cause);
		}
	} else {
		snprintf(buffer, sizeof(buffer), "%d x %d @ (%d, %d)",
			info.frame.size.width, info.frame.size.height,
			info.frame.origin.x, info.frame.origin.y);
		painter.drawText(ivoff, 2 * h, wvi, h, flags, QString(buffer));

		QString	filename(info.filename.in());
		painter.drawText(ivoff, 3 * h, wvi, h, flags, filename);
	}

	// parameter info
	QString	cameraname(parameters.camera.in());
	painter.drawText(pvoff, 0 * h, wvp, h, flags, cameraname);

	QString	ccdnumber = tr("%1").arg(parameters.ccdid);
	painter.drawText(pvoff, 1 * h,  40, h, flags, ccdnumber);

	snprintf(buffer, sizeof(buffer), "%.1f",
		parameters.ccdtemperature - 273.15);
	QString	tempstring(buffer);
	painter.drawText(pvoff + 130, 1 * h,  60, h, flags, tempstring);

	// exposure time
	snprintf(buffer, sizeof(buffer), "%.3fs",
		parameters.exp.exposuretime);
	QString	exposure(buffer);

	// binning info
	snprintf(buffer, sizeof(buffer), "%dx%d",
		parameters.exp.mode.x, parameters.exp.mode.y);
	QString	binning(buffer);

	// shutter info
	QString	shutter;
	if (parameters.exp.shutter == Astro::SHUTTER_OPEN) {
		shutter = QString("LIGHT");
	} else {
		shutter = QString("DARK");
	}

	// frame info
	if ((parameters.exp.frame.size.width == 0)
		&& (parameters.exp.frame.size.height == 0)) {
		snprintf(buffer, sizeof(buffer), "full frame");
	} else {
		snprintf(buffer, sizeof(buffer), "%d x %d @ (%d, %d)",
			parameters.exp.frame.size.width,
			parameters.exp.frame.size.height,
			parameters.exp.frame.origin.x,
			parameters.exp.frame.origin.y);
	}
	QString	frame(buffer);

	// display stuff
	painter.drawText(pvoff,       2 * h,  80, h, flags, exposure);
	painter.drawText(pvoff +  80, 2 * h,  50, h, flags, binning);
	painter.drawText(pvoff + 130, 2 * h,  60, h, flags, shutter);
	painter.drawText(pvoff + 180, 2 * h, 100, h, flags, buffer);

	// filter
	if (strlen(parameters.filterwheel.in()) > 0) {
		snprintf(buffer, sizeof(buffer), "%d @ %s",
			parameters.filterposition,
			parameters.filterwheel.in());
	} else {
		buffer[0] = '\0';
	}
	QString	filter(buffer);
	painter.drawText(pvoff, 3 * h, wvp, h, flags, filter);

	// task id
	painter.drawRect(0, 0, width() - 1, height() - 1);
	QFont	font = painter.font();
	font.setPointSize(30);
	painter.setFont(font);
	painter.drawText(5, 5, height() - 10,  height() - 10, Qt::AlignCenter,
		tr("%1").arg(info.taskid));
}

void	TaskItem::paintEvent(QPaintEvent *event) {
	//QWidget::paintEvent(event);
	draw();
}
