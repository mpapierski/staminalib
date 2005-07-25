#include <stdafx.h>
#include <math.h>
#include <cppunit/extensions/HelperMacros.h>
#include <boost/pool/pool.hpp>
#include <boost/pool/singleton_pool.hpp>
#include <Stamina/VersionControl.h>
#include <Stamina/CriticalSection.h>
#include <Stamina/Helpers.h>
#include <Stamina/Lock.h>
#include <Stamina/Mutex.h>

#define TEST_STRING

#include "..\String.h"

#include <ConvertUTF.h>

using namespace Stamina;

__declspec(noinline) int testCall(int a) {
	return max(a, 0);
}


class TestString : public CPPUNIT_NS::TestFixture
{
  
	CPPUNIT_TEST_SUITE( TestString );
  
	//CPPUNIT_TEST( timeFullStaticInit );


	CPPUNIT_TEST_SUITE_END();

protected:

public:

	void setUp() {

	}
	void tearDown() {
	}

protected:




};

CPPUNIT_TEST_SUITE_REGISTRATION( TestString );

