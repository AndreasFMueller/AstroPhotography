/*
 * FocusingCallbackI.h
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _FocusingCallback_h
#define _FocusingCallback_h

#include <focusing.h>
#include <QObject>

namespace snowgui {

class FocusingCallbackI : public QObject, public snowstar::FocusCallback {
	Q_OBJECT

public:
	FocusingCallbackI();
	virtual ~FocusingCallbackI();

	void	addPoint(const snowstar::FocusPoint&, const Ice::Current&);
	void	changeState(const snowstar::FocusState, const Ice::Current&);
	void	addFocusElement(const snowstar::FocusElement& element,
			const Ice::Current&);

signals:
	void	pointReceived(snowstar::FocusPoint);
	void	stateReceived(snowstar::FocusState);
	void	focuselementReceived(snowstar::FocusElement);
};

} // namespace snowgui

#endif /* _FocusingCallback_h */
