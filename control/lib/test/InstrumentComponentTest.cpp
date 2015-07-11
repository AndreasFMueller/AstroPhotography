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
	void	testInstrumentComponent();
	//void	testXXX();

	CPPUNIT_TEST_SUITE(InstrumentTest);
	CPPUNIT_TEST(testInstrumentComponentTable);
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
	discover::InstrumentComponentKey	key("INSTRUMENT", discover::InstrumentComponentKey::CCD, 0);
	discover::InstrumentComponent	instrument(key, "blubber", "ccd:sx/1-2-3/0");
	discover::InstrumentComponentInfo	info(instrument);
	discover::InstrumentComponentRecord	record(instrument);
	table.add(record);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "testInstrumentComponentTable() end");
}

void	InstrumentTest::testInstrumentComponent() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testInstrument() begin");
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
	
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testInstrument() end");
}

#if 0
void	InstrumentTest::testXXX() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() begin");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() end");
}
#endif

} // namespace test
} // namespace astro
