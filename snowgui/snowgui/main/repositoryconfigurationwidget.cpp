/*
 * repositoryconfigurationwidget.cpp
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "repositoryconfigurationwidget.h"
#include "ui_repositoryconfigurationwidget.h"

namespace snowgui {

/**
 * \brief Construct a new repositoryconfiguration widget
 */
repositoryconfigurationwidget::repositoryconfigurationwidget(QWidget *parent)
	: QWidget(parent), ui(new UI::repositoryconfigurationwidget) {
};

/**
 * \brief Destroy the repositoryconfiguration widget
 */
repositoryconfigurationwidget::~repositoryconfigurationwdiget() {
	delete ui;
}

}
