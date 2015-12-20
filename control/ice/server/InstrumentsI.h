/*
 * InstrumentsI.h -- declaration of the InstrumentsI implementation class
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <instruments.h>

namespace snowstar {

class InstrumentsI : virtual public Instruments {
public:
	InstrumentsI();
	virtual ~InstrumentsI();
	virtual InstrumentList	list(const Ice::Current& current);
	virtual InstrumentPrx	get(const std::string& name,
					const Ice::Current& current);
	virtual void	remove(const std::string& name,
					const Ice::Current& current);
	virtual bool	has(const std::string& name,
					const Ice::Current& current);
};

} // namespace snowstar
