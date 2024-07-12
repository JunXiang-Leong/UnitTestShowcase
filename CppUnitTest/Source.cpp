#include "Tests.h"
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <iostream>
CPPUNIT_TEST_SUITE_REGISTRATION(LibmonTest);
int main()
{
	CppUnit::TextUi::TestRunner runner;
	CppUnit::TestFactoryRegistry& registry = CppUnit::TestFactoryRegistry::getRegistry();
	runner.addTest(registry.makeTest());
	bool result = runner.run();
	LibmonTest::SetBounds(11,32);
	result = runner.run();
	if (result)
		printf("all tests passed!\n");
	int a = 0;
	std::cin >> a;
	return 0;
}
