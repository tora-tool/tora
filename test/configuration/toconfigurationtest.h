#ifndef CPP_TOCONFIGURATIONTEST_H
#define CPP_TOCONFIGURATIONTEST_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class toConfigurationTest : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE( toConfigurationTest );
  CPPUNIT_TEST( testConfiguration );
  CPPUNIT_TEST_SUITE_END();
protected:

public:
  void			setUp();
  void			tearDown();
  toConfigurationTest();
protected:
  void			testConfiguration();
private:
};


#endif
