/*
 * FocusEvaluatorTest.cpp -- test the parabolic solver
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <math.h>
#include <AstroDebug.h>
#include <AstroFocus.h>
#include <cstdlib>

using namespace astro::focusing;

namespace astro {
namespace test {

class FocusEvaluatorTest : public CppUnit::TestFixture {
private:
public:
	void	setUp();
	void	tearDown();
	void	testBasic();

	CPPUNIT_TEST_SUITE(FocusEvaluatorTest);
	CPPUNIT_TEST(testBasic);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(FocusEvaluatorTest);

void	FocusEvaluatorTest::setUp() {
}

void	FocusEvaluatorTest::tearDown() {
}

void	FocusEvaluatorTest::testBasic() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testBasic() begin");
	Image<unsigned short>	*image = new Image<unsigned short>(200,150);
	int	limit = std::numeric_limits<unsigned short>::max();
	for (int x = 0; x < 200; x++) {
		for (int y = 0; y < 150; y++) {
			image->pixel(x, y) = rand() % limit;
		}
	}
	ImagePtr	imgptr(image);
	
	FocusEvaluatorPtr	evaluator
		= FocusEvaluatorFactory::get(
			FocusEvaluatorFactory::BrennerHorizontal);
	double	value = (*evaluator)(imgptr);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "value = %f", value);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testBasic() end");
}

} // namespace test
} // namespace astro
