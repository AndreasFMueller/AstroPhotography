/*
 * InstrumentComponentTest.cpp -- template for tests
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <AstroDebug.h>
#include <AstroConfig.h>
#include <AstroFormat.h>
#include <ServiceDiscovery.h>
#include <InstrumentComponentTable.h>

using namespace astro::config;
using namespace astro::persistence;

namespace astro {
namespace test {

static std::string	dbfilename("instrumentcomponent.db");

class InstrumentTest: public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();
	void	testInstrumentComponentTable();
	void	testInstrumentBackend();
	void	testInstrumentComponent();
	//void	testXXX();

	CPPUNIT_TEST_SUITE(InstrumentTest);
	CPPUNIT_TEST(testInstrumentComponentTable);
	CPPUNIT_TEST(testInstrumentBackend);
	CPPUNIT_TEST(testInstrumentComponent);
	//CPPUNIT_TEST(testXXX);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(InstrumentTest);

void	InstrumentTest::setUp() {
}

void	InstrumentTest::tearDown() {
}

void	InstrumentTest::testInstrumentComponentTable() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testInstrumentComponentTable() begin");
	Database	database = DatabaseFactory::get(dbfilename);

	discover::InstrumentComponentTable	table(database);
	std::string	query("delete from instrumentcomponents;");
	database->query(query);
	std::string	names[2] = { "INSTRUMENT", "TELESCOPE" };

	for (int j = 0; j < 2; j++) {
		for (int i = 0; i < 5; i++) {
			discover::InstrumentComponentKey	key(names[j],
				discover::InstrumentComponentKey::CCD, i);
			discover::InstrumentComponent	component(key, "blubber",
				stringprintf("ccd:sx/1-2-3/%d", i));
			discover::InstrumentComponentRecord	record(component);

			table.add(record);
		}
		for (int i = 0; i < 5; i++) {
			discover::InstrumentComponentKey	key(names[j],
				discover::InstrumentComponentKey::Cooler, i);
			discover::InstrumentComponent	component(key, "blubber",
				stringprintf("cooler:sx/1-2-3/%d", i));
			discover::InstrumentComponentRecord	record(component);

			table.add(record);
		}
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "testInstrumentComponentTable() end");
}

void	InstrumentTest::testInstrumentBackend() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testInstrumentBackend() begin");
	Database	database = DatabaseFactory::get(dbfilename);
	discover::InstrumentBackend	backend(database);
	std::list<std::string>	l = backend.names();
	CPPUNIT_ASSERT(l.size() == 2);

	discover::InstrumentPtr	instrument = backend.get("INSTRUMENT");
	CPPUNIT_ASSERT(instrument->nComponentsOfType(discover::InstrumentComponentKey::CCD) == 5);
	CPPUNIT_ASSERT(instrument->nComponentsOfType(discover::InstrumentComponentKey::GuiderCCD) == 0);
	CPPUNIT_ASSERT(instrument->nComponentsOfType(discover::InstrumentComponentKey::Cooler) == 5);
	CPPUNIT_ASSERT(instrument->nComponentsOfType(discover::InstrumentComponentKey::GuiderPort) == 0);
	CPPUNIT_ASSERT(instrument->nComponentsOfType(discover::InstrumentComponentKey::Focuser) == 0);
	CPPUNIT_ASSERT(instrument->nComponentsOfType(discover::InstrumentComponentKey::AdaptiveOptics) == 0);

	discover::InstrumentComponentKey	key(instrument->name(), discover::InstrumentComponentKey::GuiderPort);
	discover::InstrumentComponent	component(key, "mount", "guiderport:guiderport/0");
	instrument->add(component);
	component.deviceurl("guiderport:guiderport/1");
	instrument->add(component);
	CPPUNIT_ASSERT(instrument->nComponentsOfType(discover::InstrumentComponentKey::GuiderPort) == 2);

	discover::InstrumentComponent	component2 = instrument->get(discover::InstrumentComponentKey::GuiderPort, 1);
	CPPUNIT_ASSERT(component2.name() == "INSTRUMENT");
	CPPUNIT_ASSERT(component2.type() == discover::InstrumentComponentKey::GuiderPort);
	CPPUNIT_ASSERT(component2.index() == 1);
	CPPUNIT_ASSERT(component2.servicename() == "mount");
	CPPUNIT_ASSERT(component2.deviceurl() == "guiderport:guiderport/1");

	component2.servicename("cgepro");
	instrument->update(component2);

	discover::InstrumentComponent	component3 = instrument->get(discover::InstrumentComponentKey::GuiderPort, 1);

	CPPUNIT_ASSERT(component3.servicename() == "cgepro");

	instrument->remove(discover::InstrumentComponent::CCD, 1);
	instrument->remove(discover::InstrumentComponent::CCD, 1);

	discover::InstrumentComponent	component4 = instrument->get(discover::InstrumentComponentKey::CCD, 2);
	CPPUNIT_ASSERT(component4.name() == "INSTRUMENT");
	CPPUNIT_ASSERT(component4.type() == discover::InstrumentComponentKey::CCD);
	CPPUNIT_ASSERT(component4.index() == 2);
	CPPUNIT_ASSERT(component4.servicename() == "blubber");
	CPPUNIT_ASSERT(component4.deviceurl() == "ccd:sx/1-2-3/4");
	

	debug(LOG_DEBUG, DEBUG_LOG, 0, "testInstrumentBackend() end");
}


void	InstrumentTest::testInstrumentComponent() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testInstrumentComponent() begin");
	Database	database = DatabaseFactory::get(dbfilename);


#if 0
	// create an instrument
	Instrument
	InstrumentPtr	instrument
		= InstrumentPtr(new Instrument(database, "BLUBB"));

	// add a few components
	InstrumentComponentPtr	camera = InstrumentComponentPtr(
		new InstrumentComponentDirect(DeviceName::Camera,
			DeviceName("camera:simulator/camera"), 7));
	instrument->add(camera);
	InstrumentComponentPtr	ccd = InstrumentComponentPtr(
		new InstrumentComponentDerived(DeviceName::Ccd,
			instrument, DeviceName::Camera, 5));
	instrument->add(ccd);

	// check instrument
	CPPUNIT_ASSERT(instrument->name() == "BLUBB");

	// has method
	debug(LOG_DEBUG, DEBUG_LOG, 0, "test 'has' method");
	CPPUNIT_ASSERT(instrument->has(DeviceName::Camera));
	CPPUNIT_ASSERT(instrument->has(DeviceName::Ccd));

	// component_type method
	debug(LOG_DEBUG, DEBUG_LOG, 0, "test 'component_type' method");
	CPPUNIT_ASSERT(instrument->component_type(DeviceName::Camera)
		== InstrumentComponent::direct);
	CPPUNIT_ASSERT(instrument->component_type(DeviceName::Ccd)
		== InstrumentComponent::derived);

	// devicename method
	debug(LOG_DEBUG, DEBUG_LOG, 0, "test 'devicename' method");
	CPPUNIT_ASSERT(instrument->devicename(DeviceName::Camera)
		== DeviceName("camera:simulator/camera"));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "ccd device: %s",
		instrument->devicename(DeviceName::Ccd).toString().c_str());
	CPPUNIT_ASSERT(instrument->devicename(DeviceName::Ccd)
		== DeviceName("camera:simulator/camera"));

	// name method
	debug(LOG_DEBUG, DEBUG_LOG, 0, "test 'name' method");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "name(camera) = %s",
		instrument->name(DeviceName::Camera).c_str());
	CPPUNIT_ASSERT(instrument->name(DeviceName::Camera)
		== DeviceName("camera:simulator/camera").toString());

	debug(LOG_DEBUG, DEBUG_LOG, 0, "name(ccd) = %s",
		instrument->name(DeviceName::Ccd).c_str());
	CPPUNIT_ASSERT(instrument->name(DeviceName::Ccd) == "camera");

	// unit method
	debug(LOG_DEBUG, DEBUG_LOG, 0, "test 'unit' method");
	CPPUNIT_ASSERT(instrument->unit(DeviceName::Camera) == 7);
	CPPUNIT_ASSERT(instrument->unit(DeviceName::Ccd) == 5);
#endif
	
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testInstrumentComponent() end");
}

#if 0
void	InstrumentTest::testXXX() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() begin");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() end");
}
#endif

} // namespace test
} // namespace astro
