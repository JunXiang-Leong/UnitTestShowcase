#pragma once
#include "../libMon/libMon.h"
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>
#include <cppunit/extensions/HelperMacros.h>
class LibmonTest : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(LibmonTest);
	CPPUNIT_TEST(testAceInit);
	CPPUNIT_TEST(testAceInitBounds);
	CPPUNIT_TEST(testInitMT);
	CPPUNIT_TEST(testInitMTBounds);
	CPPUNIT_TEST(testMsgCount);
	CPPUNIT_TEST(testMsgCountBounds);
	CPPUNIT_TEST(testErrMsgCount);
	CPPUNIT_TEST(testErrMsgCountBounds);
	CPPUNIT_TEST(testGetMsg);
	CPPUNIT_TEST(testGetMsgBounds);
	CPPUNIT_TEST(testShutDown);
	CPPUNIT_TEST(testShutDownBounds);

	CPPUNIT_TEST(combinedTest);
	CPPUNIT_TEST_SUITE_END();
public:
	void setUp()
	{

	}
	void tearDown()
	{
		ShutdownMT(1);
	}
	void testAceInit()
	{
		//only uses devnum = 1
		CPPUNIT_ASSERT_EQUAL(aceInitialize2(1,0,0,0,0,0) , aceInitialize2(1,0,0,0,0,0)); //test for similiar result
		CPPUNIT_ASSERT(aceInitialize2(1, 0, 0, 0, 0, 0) != aceInitialize2(2, 0, 0, 0, 0, 0)); //test for difference result
		
	}
	void testAceInitBounds()
	{
		CPPUNIT_ASSERT(aceInitialize2(-1, 0, 0, 0, 0, 0) == -1); //bounds check < 0
		CPPUNIT_ASSERT(aceInitialize2(-100, 0, 0, 0, 0, 0) == -1);
		CPPUNIT_ASSERT(aceInitialize2(32, 0, 0, 0, 0, 0) == -1); //bounds check > 31
		CPPUNIT_ASSERT(aceInitialize2(100, 0, 0, 0, 0, 0) == -1);
	}
	void testInitMT()
	{
		//uses devnum 1, 2
		CPPUNIT_ASSERT_EQUAL(InitMT(1), InitMT(1));
		CPPUNIT_ASSERT(InitMT(1) != InitMT(2));

	}
	void testInitMTBounds()
	{
		//uses devnum 1, 2
		CPPUNIT_ASSERT(InitMT(-1) == -1);
		CPPUNIT_ASSERT(InitMT(100)  == -1);

	}
	void testMsgCount()
	{
		//uses devnum 1, 3
		unsigned int count = 0;
		unsigned int count_check = 0;
		GetMTMsgCount(3, &count);
		GetMTMsgCount(3, &count_check);

		CPPUNIT_ASSERT(count == count_check);
		
		InitMT(1);
		GetMTMsgCount(1, &count);
		GetMTMsgCount(3, &count_check);

		CPPUNIT_ASSERT(count != count_check);

	}
	void testMsgCountBounds()
	{
		unsigned int count = 0;
		
		CPPUNIT_ASSERT(GetMTMsgCount(-1, &count) == -1);
		CPPUNIT_ASSERT(GetMTMsgCount(100, &count) == -1);

	}
	void testErrMsgCount()
	{
		//uses devnum 1, 3
		unsigned int count = 0;
		unsigned int count_check = 0;
		GetMTMsgErrCount(3, &count);
		GetMTMsgErrCount(3, &count_check);

		CPPUNIT_ASSERT(count == count_check);

		InitMT(1);
		GetMTMsgErrCount(1, &count);
		GetMTMsgErrCount(3, &count_check);

		CPPUNIT_ASSERT(count != count_check);

	}
	void testErrMsgCountBounds()
	{
		unsigned count = 0;
		CPPUNIT_ASSERT(GetMTMsgErrCount(-1, &count) == -1);
		CPPUNIT_ASSERT(GetMTMsgErrCount(100, &count) == -1);
	}
	void testGetMsg()
	{
		U16BIT a = 0;
		U16BIT b = 0;
		MSGSTRUCT msg;
		MSGSTRUCT msg_check;
		msg.error.clear();
		msg_check.error.clear();
		InitMT(4);
		GetMTMsg(4, &a, &b, &msg);
		GetMTMsg(4, &b, &a, &msg_check);
		printf("\n start\n %s\n%s \n end\n", msg.error.c_str(), msg_check.error.c_str());
		CPPUNIT_ASSERT(msg.error == msg_check.error);

		InitMT(1);
		GetMTMsg(1, &a, &b, &msg);
		GetMTMsg(4, &b, &a, &msg_check);
		CPPUNIT_ASSERT(msg.error != msg_check.error);

	}
	void testGetMsgBounds()
	{
		U16BIT a = 0;
		U16BIT b = 0;
		MSGSTRUCT msg;
		CPPUNIT_ASSERT(GetMTMsg(-1, &a, &b, &msg) == -1);
		CPPUNIT_ASSERT(GetMTMsg(100, &a, &b, &msg) == -1);
	}
	void testShutDown()
	{
		unsigned count = 0;
		unsigned e_count = 0;
		unsigned count_afterShutdown = 0;
		unsigned e_count_afterShutdown = 0;
		InitMT(1);
		GetMTMsgCount(1, &count);
		GetMTMsgErrCount(1, &e_count);
		ShutdownMT(1);
		GetMTMsgCount(1, &count_afterShutdown);
		GetMTMsgErrCount(1, &e_count_afterShutdown);

		CPPUNIT_ASSERT(count != count_afterShutdown);
		CPPUNIT_ASSERT(e_count != e_count_afterShutdown);
	}
	void testShutDownBounds()
	{
		CPPUNIT_ASSERT(ShutdownMT(-1) == -1);
		CPPUNIT_ASSERT(ShutdownMT(100) == -1);
	}

	void combinedTest()
	{
		ShutdownMT(1);
		InitMT(1);
		InitMT(1);
		InitMT(1);
		unsigned count = 0;
		unsigned e_count = 0;

		GetMTMsgCount(1, &count);
		GetMTMsgErrCount(1, &e_count);
		
		CPPUNIT_ASSERT(count == 3);
		CPPUNIT_ASSERT(e_count == 3);

		U16BIT a = 1;
		U16BIT b = 1;
		MSGSTRUCT msg;
		GetMTMsg(1, &a, &b, &msg);
		CPPUNIT_ASSERT(msg.error == "error number 1");
		b = 15;
		GetMTMsg(1, &a, &b, &msg);
		CPPUNIT_ASSERT(msg.error == "no message");

		ShutdownMT(1);
		GetMTMsgCount(1, &count);
		GetMTMsgErrCount(1, &e_count);
		CPPUNIT_ASSERT(count == 0);
		CPPUNIT_ASSERT(e_count == 0);
	}
};
