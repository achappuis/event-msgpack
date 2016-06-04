#include <iostream>
#include <string>
#include <list>
#include <fstream>
#include <cppunit/TestCase.h>
#include <cppunit/TestFixture.h>
#include <cppunit/ui/text/TextTestRunner.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestRunner.h>
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/XmlOutputter.h>

int main( int argc, char **argv)
{
	    CPPUNIT_NS::TestResult testresult;
	    CPPUNIT_NS::TestResultCollector collectedresults;
	    testresult.addListener (&collectedresults);
	    CPPUNIT_NS::BriefTestProgressListener progress;
	    testresult.addListener (&progress);
	    CPPUNIT_NS::TestRunner testrunner;
	    testrunner.addTest (CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest ());
	    testrunner.run(testresult);

	    CPPUNIT_NS::CompilerOutputter compileroutputter(&collectedresults, std::cerr);
	    compileroutputter.write ();

	    std::ofstream xmlFileOut("event-msgpack.xml");
	    CPPUNIT_NS::XmlOutputter xmlOut(&collectedresults, xmlFileOut);
	    xmlOut.write();

	    return collectedresults.wasSuccessful() ? 0 : 1;
}
