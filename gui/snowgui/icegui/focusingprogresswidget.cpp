/*
 * focusingprogresswidget.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "focusingprogresswidget.h"
#include "ui_focusingprogresswidget.h"
#include <QTableWidget>
#include <QTableWidgetItem>
#include <focusing.h>
#include <AstroDebug.h>
#include <AstroFormat.h>

namespace snowgui {

FocusingProgressWidget::FocusingProgressWidget(QWidget *parent)
	: QWidget(parent), ui(new Ui::FocusingProgressWidget) {
	ui->setupUi(this);

	ui->pointTable->setSelectionBehavior(QAbstractItemView::SelectItems);
	ui->pointTable->setSelectionMode(QAbstractItemView::SingleSelection);

	ui->pointTable->setColumnCount(2);
	ui->pointTable->setRowCount(0);

	ui->pointTable->setColumnWidth(0, 70);
	ui->pointTable->setColumnWidth(1, 70);

	connect(ui->pointTable, SIGNAL(cellClicked(int, int)),
		this, SLOT(cellActivated(int, int)));
	connect(ui->pointTable, SIGNAL(currentCellChanged(int, int, int, int)),
		this, SLOT(cellChanged(int, int, int, int)));

	// set up the table view
	QStringList	headers;
	headers << "Position";
	headers << "Value";
	ui->pointTable->setHorizontalHeaderLabels(headers);
}

FocusingProgressWidget::~FocusingProgressWidget() {
	delete ui;
}

void	FocusingProgressWidget::receivePoint(snowstar::FocusPoint point) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got a new point: %d -> %f",
		point.position, point.value);

	// set up a new row
	int	row = ui->pointTable->rowCount();
	ui->pointTable->setRowCount(row + 1);
	ui->pointTable->setRowHeight(row, 19);

	// add the items
	QTableWidgetItem	*item
		= new QTableWidgetItem(QString::number(point.position));
	item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
	ui->pointTable->setItem(row, 0, item);

	std::string	s = astro::stringprintf("%.3f", point.value);
	item = new QTableWidgetItem(QString(s.c_str()));
	item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
	ui->pointTable->setItem(row, 1, item);
}

void	FocusingProgressWidget::receiveState(snowstar::FocusState state) {
	if ((_focused) && (state != snowstar::FocusFOCUSED)) {
		ui->pointTable->setRowCount(0);
		_focused = false;
	}
	switch (state) {
	case snowstar::FocusIDLE:
		ui->stateField->setText(QString("IDLE"));
		break;
	case snowstar::FocusMOVING:
		ui->stateField->setText(QString("MOVING"));
		break;
	case snowstar::FocusMEASURING:
		ui->stateField->setText(QString("MEASURING"));
		break;
	case snowstar::FocusMEASURED:
		ui->stateField->setText(QString("MEASUERED"));
		break;
	case snowstar::FocusFOCUSED:
		ui->stateField->setText(QString("FOCUSED"));
		_focused = true;
		break;
	case snowstar::FocusFAILED:
		ui->stateField->setText(QString("FAILED"));
		break;
	}
}

void	FocusingProgressWidget::cellActivated(int row, int /* column */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "row selected: %d", row);
	emit rowSelected(row);
}

void	FocusingProgressWidget::cellChanged(int row, int /* column */,
		int /* previousrow */, int /* previouscolumn */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "row selected: %d", row);
	emit rowSelected(row);
}

} // namespace snowgui
