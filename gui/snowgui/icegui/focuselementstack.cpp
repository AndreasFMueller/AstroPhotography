/*
 * focuselementstack.cpp -- implementation of focus element stack
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "focuselementstack.h"
#include "focuselementview.h"
#include <AstroDebug.h>

namespace snowgui {

/**
 * \brief Create a FocusElement stack
 *
 * \param parent	the parent widget
 */
FocusElementStack::FocusElementStack(QWidget *parent) : QStackedWidget(parent) {
	_restart = true;
}

/**
 * \brief destroy the stack
 */
FocusElementStack::~FocusElementStack() {
}

/**
 * \brief Process a new focus element
 *
 * \param element	the new focus element
 */
void	FocusElementStack::receiveFocusElement(snowstar::FocusElement element) {
	// remove all widgets if necessary
	if (_restart) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "removing old widgets");
		while (count() > 0) {
			removeWidget(widget(0));
		}
		_restart = false;
	}

	// create a new 
	FocusElementView	*focuselementview = new FocusElementView(this);
	focuselementview->setFocusElement(element);

	// add the view as the last view of the stack
	int	current = insertWidget(count(), focuselementview);
	if (current >= count()) {
		return;
	}
	setCurrentIndex(current);
}

/**
 * \brief Process a state change
 *
 * \param state		new state
 */
void	FocusElementStack::receiveState(snowstar::FocusState state) {
	switch (state) {
	case snowstar::FocusFOCUSED:
	case snowstar::FocusFAILED:
	case snowstar::FocusIDLE:
		_restart = true;
		break;
	default:
		break;
	}
}

} // namespace snowgui
