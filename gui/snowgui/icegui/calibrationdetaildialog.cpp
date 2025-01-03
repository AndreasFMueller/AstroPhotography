/*
 * calibrationdetaildialog.cpp -- detail display of calibration data
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "calibrationdetaildialog.h"
#include "ui_calibrationdetaildialog.h"
#include <AstroFormat.h>
#include <AstroDebug.h>
#include <IceConversions.h>

namespace snowgui {

calibrationdetaildialog::calibrationdetaildialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::calibrationdetaildialog)
{
    	ui->setupUi(this);

	// initialize the calibration data
	_calibration.id = -1;

	// headers for the point table
	QStringList	headerlist;
        headerlist << "Time" << "RA" << "DEC" << "Star x" << "Star y";
        ui->calibrationpointsTable->setHorizontalHeaderLabels(headerlist);
        ui->calibrationpointsTable->horizontalHeader()->setStretchLastSection(true);

	ui->calibrationpointsTable->setColumnWidth(0, 55);
	ui->calibrationpointsTable->setColumnWidth(1, 55);
	ui->calibrationpointsTable->setColumnWidth(2, 55);
	ui->calibrationpointsTable->setColumnWidth(3, 55);
	ui->calibrationpointsTable->setColumnWidth(4, 55);

	ui->calibrationdisplayWidget->pointlabels(true);

	// configure the coefficient
	QStringList	coefficientlist;
	coefficientlist << "RA" << "DEC" << "t";

        ui->coefficientTable->setHorizontalHeaderLabels(coefficientlist);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "setting up coefficient table");
	for (int column = 0; column < 3; column++) {
		ui->coefficientTable->setColumnWidth(column, 60);
		for (int row = 0; row < 2; row++) {
			QTableWidgetItem        *item =
				new QTableWidgetItem(QString("0.00"));
			item->setTextAlignment(Qt::AlignRight);
			ui->coefficientTable->setItem(row, column, item);
		}
	}
	ui->coefficientTable->setRowHeight(0, 18);
	ui->coefficientTable->setRowHeight(1, 18);

}

calibrationdetaildialog::~calibrationdetaildialog() {
    delete ui;
}

static double	angle(const snowstar::Calibration& c) {
	double	a[6];
	for (int i = 0; i < 6; i++) {
		a[i] = c.coefficients[i];
	}
	double	l0 = hypot(a[0], a[3]);
	double	l1 = hypot(a[1], a[4]);
	return acos((a[0] * a[1] + a[3] * a[4]) / (l0 * l1));
}

void	calibrationdetaildialog::setCalibration(snowstar::Calibration calibration) {
	_calibration = calibration;

	// update the window title
	setWindowTitle(QString(astro::stringprintf("Calibration %d",
		_calibration.id).c_str()));

	// update fields
	ui->instrumentField->setText(QString(
		_calibration.instrument.c_str()));
	ui->resolutionField->setText(QString(astro::stringprintf("%.1f\"/px",
		_calibration.masPerPixel / 1000).c_str()));
	ui->intervalField->setText(QString(astro::stringprintf("%.1fs",
		_calibration.interval).c_str()));
	char	buffer[100];
	time_t	when = snowstar::converttime(_calibration.timeago);
	struct tm	*tmp = localtime(&when);
	strftime(buffer, sizeof(buffer), "%F %T", tmp);
	ui->dateField->setText(QString(buffer));
	ui->qualityField->setText(QString(astro::stringprintf("%.1f%%",
		100 * _calibration.quality).c_str()));
	ui->angleField->setText(QString(astro::stringprintf("XXX").c_str()));
	ui->pointsField->setText(QString(astro::stringprintf("%d",
		_calibration.points.size()).c_str()));
	ui->angleField->setText(QString(astro::stringprintf("%.1f˚",
		angle(_calibration) * 180 / M_PI).c_str()));
	ui->detField->setText(QString(astro::stringprintf("%.1f",
		_calibration.det).c_str()));

	int	eastsign = (_calibration.east) ? 1 : -1;
	QString	stylesheet(((eastsign * _calibration.det) < 0)
			? "{ color: red }" : "{ color: black }");
	// XXX the color stylesheet does not work
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stylesheet: %s",
		stylesheet.toLatin1().data());
	ui->eastField->setStyleSheet(stylesheet);
	ui->eastField->setText(QString((_calibration.east) ? "east" : "west"));

	// give the data to the calibration display
	ui->calibrationdisplayWidget->setCalibration(_calibration);

	// XXX set the points
	QTableWidget	*table = ui->calibrationpointsTable;
	int	row = 0;
	table->setRowCount(_calibration.points.size());
	std::for_each(_calibration.points.begin(), _calibration.points.end(),
		[table,&row](const snowstar::CalibrationPoint& p) mutable {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "display point %f", p.t);
			table->setRowHeight(row, 15);
			QTableWidgetItem	*i;
			i = new QTableWidgetItem(QString(astro::stringprintf("%.1f", p.t).c_str()));
			i->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
			table->setItem(row, 0, i);
                        i = new QTableWidgetItem(QString(astro::stringprintf("%.1f", p.offset.x).c_str()));
			i->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                        table->setItem(row, 1, i);
                        i = new QTableWidgetItem(QString(astro::stringprintf("%.1f", p.offset.y).c_str()));
			i->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                        table->setItem(row, 2, i);
                        i = new QTableWidgetItem(QString(astro::stringprintf("%.1f", p.star.x).c_str()));
			i->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                        table->setItem(row, 3, i);
                        i = new QTableWidgetItem(QString(astro::stringprintf("%.1f", p.star.y).c_str()));
			i->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                        table->setItem(row, 4, i);
                        row++;
		}
	);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d points", row);

	//ui->calibrationpointsTable->resizeColumnsToContents();

	// set the calibration coefficients
	ui->coefficientTable->item(0,0)->setText(
		QString(astro::stringprintf("%.2f",
			_calibration.coefficients[0]).c_str()));
	ui->coefficientTable->item(0,1)->setText(
		QString(astro::stringprintf("%.2f",
			_calibration.coefficients[1]).c_str()));
	ui->coefficientTable->item(0,2)->setText(
		QString(astro::stringprintf("%.2f",
			_calibration.coefficients[2]).c_str()));
	ui->coefficientTable->item(1,0)->setText(
		QString(astro::stringprintf("%.2f",
			_calibration.coefficients[3]).c_str()));
	ui->coefficientTable->item(1,1)->setText(
		QString(astro::stringprintf("%.2f",
			_calibration.coefficients[4]).c_str()));
	ui->coefficientTable->item(1,2)->setText(
		QString(astro::stringprintf("%.2f",
			_calibration.coefficients[5]).c_str()));
}

} // namespace snowgui
