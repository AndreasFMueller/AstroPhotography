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

	// headers for the table
	QStringList	headerlist;
        headerlist << "Time" << "RA" << "DEC" << "Star x" << "Star y";
        ui->calibrationpointsTable->setHorizontalHeaderLabels(headerlist);
        ui->calibrationpointsTable->horizontalHeader()->setStretchLastSection(true);

	ui->calibrationpointsTable->setColumnWidth(0, 55);
	ui->calibrationpointsTable->setColumnWidth(1, 55);
	ui->calibrationpointsTable->setColumnWidth(2, 55);
	ui->calibrationpointsTable->setColumnWidth(3, 55);
	ui->calibrationpointsTable->setColumnWidth(4, 55);
}

calibrationdetaildialog::~calibrationdetaildialog() {
    delete ui;
}

void	calibrationdetaildialog::setCalibration(snowstar::Calibration calibration) {
	_calibration = calibration;

	// update the window title
	setWindowTitle(QString(astro::stringprintf("Calibration %d", _calibration.id).c_str()));

	// update fields
	ui->instrumentField->setText(QString(_calibration.guider.instrumentname.c_str()));
	ui->ccdField->setText(QString(astro::stringprintf("%d/%d/%d",
		_calibration.guider.ccdIndex,
		_calibration.guider.guiderportIndex,
		_calibration.guider.adaptiveopticsIndex).c_str()));
	ui->resolutionField->setText(QString(astro::stringprintf("%.1f\"/px",
		_calibration.masPerPixel / 1000).c_str()));
	char	buffer[100];
	time_t	when = snowstar::converttime(_calibration.timeago);
	struct tm	*tmp = localtime(&when);
	strftime(buffer, sizeof(buffer), "%F", tmp);
	ui->dateField->setText(QString(buffer));
	ui->qualityField->setText(QString(astro::stringprintf("%.1f%%",
		_calibration.quality).c_str()));
	ui->angleField->setText(QString(astro::stringprintf("XXX").c_str()));
	ui->pointsField->setText(QString(astro::stringprintf("%d",
		_calibration.points.size()).c_str()));

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
}

} // namespace snowgui
