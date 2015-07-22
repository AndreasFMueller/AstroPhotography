/*
 * InstrumentTest.cpp -- template for tests
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <AstroDebug.h>
#include <AstroConfig.h>

using namespace astro::config;
using namespace astro::persistence;
using namespace astro::device;

namespace astro {
namespace test {

static std::string	dbfilename("instruments.db");

class InstrumentTest: public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();
	void	testInstrument();
	void	testSave();
	void	testRead();
	void	testRemove();
	//void	testXXX();

	CPPUNIT_TEST_SUITE(InstrumentTest);
	CPPUNIT_TEST(testInstrument);
	CPPUNIT_TEST(testSave);
	CPPUNIT_TEST(testRead);
	CPPUNIT_TEST(testRemove);
	//CPPUNIT_TEST(testXXX);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(InstrumentTest);

void	InstrumentTest::setUp() {
}

void	InstrumentTest::tearDown() {
}

void	InstrumentTest::testInstrument() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testInstrument() begin");
	ConfigurationPtr	config = Configuration::get(dbfilename);
	Database	database = config->database();

	// create an instrument
	Instrument	instrument(database, "BLUBB");

	// add a few components
	InstrumentComponentPtr	camera = InstrumentComponentPtr(
		new InstrumentComponentDirect(DeviceName::Camera,
			DeviceName("camera:simulator/camera"), 7, "localhost"));
	instrument.add(camera);
	InstrumentComponentPtr	ccd = InstrumentComponentPtr(
		new InstrumentComponentDerived(DeviceName::Ccd,
			instrument, DeviceName::Camera, 5));
	instrument.add(ccd);

	// check instrument
	CPPUNIT_ASSERT(instrument.name() == "BLUBB");

	// has method
	debug(LOG_DEBUG, DEBUG_LOG, 0, "test 'has' method");
	CPPUNIT_ASSERT(instrument.has(DeviceName::Camera));
	CPPUNIT_ASSERT(instrument.has(DeviceName::Ccd));

	// component_type method
	debug(LOG_DEBUG, DEBUG_LOG, 0, "test 'component_type' method");
	CPPUNIT_ASSERT(instrument.component_type(DeviceName::Camera)
		== InstrumentComponent::direct);
	CPPUNIT_ASSERT(instrument.component_type(DeviceName::Ccd)
		== InstrumentComponent::derived);

	// devicename method
	debug(LOG_DEBUG, DEBUG_LOG, 0, "test 'devicename' method");
	CPPUNIT_ASSERT(instrument.devicename(DeviceName::Camera)
		== DeviceName("camera:simulator/camera"));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "ccd device: %s",
		instrument.devicename(DeviceName::Ccd).toString().c_str());
	CPPUNIT_ASSERT(instrument.devicename(DeviceName::Ccd)
		== DeviceName("camera:simulator/camera"));

	// name method
	debug(LOG_DEBUG, DEBUG_LOG, 0, "test 'name' method");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "name(camera) = %s",
		instrument.name(DeviceName::Camera).c_str());
	CPPUNIT_ASSERT(instrument.name(DeviceName::Camera)
		== DeviceName("camera:simulator/camera").toString());

	debug(LOG_DEBUG, DEBUG_LOG, 0, "name(ccd) = %s",
		instrument.name(DeviceName::Ccd).c_str());
	CPPUNIT_ASSERT(instrument.name(DeviceName::Ccd) == "camera");

	// unit method
	debug(LOG_DEBUG, DEBUG_LOG, 0, "test 'unit' method");
	CPPUNIT_ASSERT(instrument.unit(DeviceName::Camera) == 7);
	CPPUNIT_ASSERT(instrument.unit(DeviceName::Ccd) == 5);
	
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testInstrument() end");
}

void	InstrumentTest::testSave() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testSave() begin");
	ConfigurationPtr	config = Configuration::get(dbfilename);
	Database	database = config->database();
	DeviceMapperConfigurationPtr	devicemapperconfig
		= DeviceMapperConfiguration::get(config);

	// make sure we have an entry in the device mapper for TEST
	DeviceMapperPtr	devicemapper = devicemapperconfig->devicemapper();
	DeviceMap	mapentry(DeviceName("filterwheel:sx/0"));
	mapentry.name("TEST");
	mapentry.unitid(1291);
	mapentry.description("test filterwheel");
	devicemapper->add(mapentry);

	// create an instrument
	InstrumentPtr	instrument(new Instrument(database, "BLUBB"));

	// add a few components
	InstrumentComponentPtr	camera = InstrumentComponentPtr(
		new InstrumentComponentDirect(DeviceName::Camera,
			DeviceName("camera:simulator/camera"), 7, "localhost"));
	instrument->add(camera);
	InstrumentComponentPtr	ccd = InstrumentComponentPtr(
		new InstrumentComponentDerived(DeviceName::Ccd,
			*instrument, DeviceName::Camera, 5));
	instrument->add(ccd);
	InstrumentComponentPtr	filterwheel = InstrumentComponentPtr(
		new InstrumentComponentMapped(DeviceName::Filterwheel,
			database, "TEST"));
	instrument->add(filterwheel);

	// add the instrument to the database
	InstrumentConfigurationPtr	instrumentconfig
		= InstrumentConfiguration::get(config);
	instrumentconfig->addInstrument(instrument);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "testSave() end");
}

void	InstrumentTest::testRead() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testRead() begin");
	ConfigurationPtr	config = Configuration::get(dbfilename);
	InstrumentConfigurationPtr	instrumentconfig
		= InstrumentConfiguration::get(config);
	InstrumentPtr	instrument = instrumentconfig->instrument("BLUBB");

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
	CPPUNIT_ASSERT(instrument->component_type(DeviceName::Filterwheel)
		== InstrumentComponent::mapped);

	// devicename method
	debug(LOG_DEBUG, DEBUG_LOG, 0, "test 'devicename' method");
	CPPUNIT_ASSERT(instrument->devicename(DeviceName::Camera)
		== DeviceName("camera:simulator/camera"));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "ccd device: %s",
		instrument->devicename(DeviceName::Ccd).toString().c_str());
	CPPUNIT_ASSERT(instrument->devicename(DeviceName::Ccd)
		== DeviceName("camera:simulator/camera"));
	CPPUNIT_ASSERT(instrument->devicename(DeviceName::Filterwheel).toString()
		== "filterwheel:sx/0");

	// name method
	debug(LOG_DEBUG, DEBUG_LOG, 0, "test 'name' method");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "name(camera) = %s",
		instrument->name(DeviceName::Camera).c_str());
	CPPUNIT_ASSERT(instrument->name(DeviceName::Camera)
		== DeviceName("camera:simulator/camera").toString());

	debug(LOG_DEBUG, DEBUG_LOG, 0, "name(ccd) = %s",
		instrument->name(DeviceName::Ccd).c_str());
	CPPUNIT_ASSERT(instrument->name(DeviceName::Ccd) == "camera");
	CPPUNIT_ASSERT(instrument->name(DeviceName::Filterwheel)
		== "TEST");

	// unit method
	debug(LOG_DEBUG, DEBUG_LOG, 0, "test 'unit' method");
	CPPUNIT_ASSERT(instrument->unit(DeviceName::Camera) == 7);
	CPPUNIT_ASSERT(instrument->unit(DeviceName::Ccd) == 5);
	CPPUNIT_ASSERT(instrument->unit(DeviceName::Filterwheel) == 1291);
	
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testRead() end");
}

void	InstrumentTest::testRemove() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testRemove() begin");
	ConfigurationPtr	config = Configuration::get(dbfilename);

	InstrumentConfigurationPtr	instrumentconfig
		= InstrumentConfiguration::get(config);
	instrumentconfig->removeInstrument("BLUBB");
	
	DeviceMapperConfigurationPtr	devicemapperconfig
		= DeviceMapperConfiguration::get(config);
	devicemapperconfig->devicemapper()->remove("TEST");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testRemove() end");
}

#if 0
void	InstrumentTest::testXXX() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() begin");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() end");
}
#endif

} // namespace test
} // namespace astro
