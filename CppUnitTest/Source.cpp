#include "Tests.h"
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <iostream>
CPPUNIT_TEST_SUITE_REGISTRATION(LibmonTest);
int main(int argc, char* argv[])
{
	CppUnit::TextUi::TestRunner runner;
	CppUnit::TestFactoryRegistry& registry = CppUnit::TestFactoryRegistry::getRegistry();
	runner.addTest(registry.makeTest());
	bool result = runner.run();

	if (argc > 1)
	{
		if ((argc - 1) % 2 != 0) //first value of argv is the exe itself
		{
			std::cerr << "Incorrect amount of arguments" << std::endl;
			return 0;
		}
		for (int i = 0; i < (argc-1) / 2; ++i)
		{
			S16BIT lower = std::stoi(argv[i*2 + 1]); 
			S16BIT upper = std::stoi(argv[i * 2 + 2]);

			LibmonTest::SetBounds(lower, upper);
			result &= runner.run();
		}
	}
	
	if (!result)
		std::cerr << "Test Failed" << std::endl;
	else
		std::cout << "Test Completed" << std::endl;
	int a = 0;
	return 0;
}
