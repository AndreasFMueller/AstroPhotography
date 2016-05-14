/*
 * BrennerSolverTest.cpp -- test the parabolic solver
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

class BrennerSolverTest : public CppUnit::TestFixture {
private:
	FocusItems	focusitems;
public:
	void	setUp();
	void	tearDown();
	void	testBasic();

	CPPUNIT_TEST_SUITE(BrennerSolverTest);
	CPPUNIT_TEST(testBasic);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(BrennerSolverTest);

void	BrennerSolverTest::setUp() {
	if (0 != focusitems.size()) {
		return;
	}
	focusitems.insert(FocusItem(17767, 2317628735488));
	focusitems.insert(FocusItem(18267, 2330542473216));
	focusitems.insert(FocusItem(18767, 2337968226304));
	focusitems.insert(FocusItem(19267, 2347332009984));
	focusitems.insert(FocusItem(19767, 2369990426624));
	focusitems.insert(FocusItem(20267, 2382638350336));
	focusitems.insert(FocusItem(20767, 2396592013312));
	focusitems.insert(FocusItem(21267, 2415844392960));
	focusitems.insert(FocusItem(21767, 2449393582080));
	focusitems.insert(FocusItem(22267, 2496893026304));
	focusitems.insert(FocusItem(22767, 2514703089664));
	focusitems.insert(FocusItem(23267, 2567158890496));
	focusitems.insert(FocusItem(23767, 2594093924352));
	focusitems.insert(FocusItem(24267, 2672381394944));
	focusitems.insert(FocusItem(24767, 2748844867584));
	focusitems.insert(FocusItem(25267, 2831175385088));
	focusitems.insert(FocusItem(25767, 2972209643520));
	focusitems.insert(FocusItem(26267, 3121271537664));
	focusitems.insert(FocusItem(26767, 3352410193920));
	focusitems.insert(FocusItem(27267, 3627573313536));
	focusitems.insert(FocusItem(27767, 3896349818880));
	focusitems.insert(FocusItem(28267, 4280891998208));
	focusitems.insert(FocusItem(28767, 4376207818752));
	focusitems.insert(FocusItem(29267, 5128434221056));
	focusitems.insert(FocusItem(29767, 5418767613952));
	focusitems.insert(FocusItem(30267, 5860056629248));
	focusitems.insert(FocusItem(30767, 7715173695488));
	focusitems.insert(FocusItem(31267, 7216581574656));
	focusitems.insert(FocusItem(31767, 8255149965312));
	focusitems.insert(FocusItem(32267, 8999638401024));
	focusitems.insert(FocusItem(32767, 9336104419328));
	focusitems.insert(FocusItem(33267, 9088040697856));
	focusitems.insert(FocusItem(33767, 8122378223616));
	focusitems.insert(FocusItem(34267, 7334054068224));
	focusitems.insert(FocusItem(34767, 7734902652928));
	focusitems.insert(FocusItem(35267, 5869927923712));
	focusitems.insert(FocusItem(35767, 5478468812800));
	focusitems.insert(FocusItem(36267, 5171866238976));
	focusitems.insert(FocusItem(36767, 4441235783680));
	focusitems.insert(FocusItem(37267, 4277159591936));
	focusitems.insert(FocusItem(37767, 3997515907072));
	focusitems.insert(FocusItem(38267, 3633977229312));
	focusitems.insert(FocusItem(38767, 3421198352384));
	focusitems.insert(FocusItem(39267, 3166597545984));
	focusitems.insert(FocusItem(39767, 2993363877888));
	focusitems.insert(FocusItem(40267, 2875949842432));
}

void	BrennerSolverTest::tearDown() {
}

void	BrennerSolverTest::testBasic() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testBasic() begin");
	BrennerSolver	bs;
	int	position = bs.position(focusitems);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "position found: %d", position);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testBasic() end");
}

} // namespace test
} // namespace astro
