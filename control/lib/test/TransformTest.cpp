/*
 * TransformTest.cpp -- test image transforms
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroTransform.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <debug.h>
#include <iostream>

using namespace astro::image;
using namespace astro::image::transform;

namespace astro {
namespace test {

class TransformTest : public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();

	void	testIdentity();
	void	testPoints();
	void	testTranslation();
	void	testProduct();
	void	testHomothety();
	void	testAnglePreserving();
	void	testAreaPreserving();
	void	testOperator();

	CPPUNIT_TEST_SUITE(TransformTest);
	CPPUNIT_TEST(testIdentity);
	CPPUNIT_TEST(testPoints);
	CPPUNIT_TEST(testTranslation);
	CPPUNIT_TEST(testProduct);
	CPPUNIT_TEST(testHomothety);
	CPPUNIT_TEST(testAnglePreserving);
	CPPUNIT_TEST(testAreaPreserving);
	CPPUNIT_TEST(testOperator);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TransformTest);

void	TransformTest::setUp() {
}

void	TransformTest::tearDown() {
}

void	TransformTest::testIdentity() {
	Transform	t;
	CPPUNIT_ASSERT(t.isIdentity());
}

void	TransformTest::testPoints() {
	Point	P1(0, 0);
	Point	P2(1, 0);
	Point	P3(1, 1);
	std::vector<Point>	frompoints;
	frompoints.push_back(P1);
	frompoints.push_back(P2);
	frompoints.push_back(P3);

	Point	Q1(2, 3);
	Point	Q2(2, 4);
	Point	Q3(1, 4);
	std::vector<Point>	topoints;
	topoints.push_back(Q1);
	topoints.push_back(Q2);
	topoints.push_back(Q3);
	
	Transform	t1(frompoints, topoints);
	Transform	t2(M_PI/2, Point(2, 3));
	CPPUNIT_ASSERT(t1 == t2);
}

void	TransformTest::testTranslation() {
	Transform	t1(0, Point(4, 5));
	CPPUNIT_ASSERT(t1.isTranslation());
	Transform	t2(0, Point(4, 5), 2);
	CPPUNIT_ASSERT(!t2.isTranslation());
}

void	TransformTest::testProduct() {
	Transform	t1(10, Point(0, 0), 2);
	Transform	t2(-10, Point(0, 0), 0.5);
	Transform	t3 = t1 * t2;
	CPPUNIT_ASSERT(t3.isIdentity());
}

void	TransformTest::testHomothety() {
	Transform	t1(0, Point(0, 0), 3);
	CPPUNIT_ASSERT(t1.isHomothety());
}

void	TransformTest::testAnglePreserving() {
	Transform	t1(5, Point(1,2), 2);
	CPPUNIT_ASSERT(t1.isAnglePreserving());
}

void	TransformTest::testAreaPreserving() {
	Point	P1(0, 0);
	Point	P2(1, 0);
	Point	P3(1, 1);
	std::vector<Point>	frompoints;
	frompoints.push_back(P1);
	frompoints.push_back(P2);
	frompoints.push_back(P3);

	Point	Q1(0, 0);
	Point	Q2(1, 0);
	Point	Q3(0, 1);
	std::vector<Point>	topoints;
	topoints.push_back(Q1);
	topoints.push_back(Q2);
	topoints.push_back(Q3);
	
	Transform	t1(frompoints, topoints);
	CPPUNIT_ASSERT(t1.isAreaPreserving());
}

void	TransformTest::testOperator() {
	Transform	t(M_PI/4, Point(1,2));
	Point	p1(1, 0);
	Point	q1 = t(p1);
	CPPUNIT_ASSERT(fabs(q1.x - (1 + sqrt(0.5))) < 0.000001);
	CPPUNIT_ASSERT(fabs(q1.y - (2 + sqrt(0.5))) < 0.000001);
	Point	p2(0, 1);
	Point	q2 = t(p2);
	CPPUNIT_ASSERT(fabs(q2.x - (1 - sqrt(0.5))) < 0.000001);
	CPPUNIT_ASSERT(fabs(q2.y - (2 + sqrt(0.5))) < 0.000001);
}

} // namespace test
} // namespace astro
