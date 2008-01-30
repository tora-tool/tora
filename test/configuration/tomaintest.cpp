#include <cppunit/CompilerOutputter.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <iostream>
using std::cerr;

int main( int argc, char** argv )
{
  CppUnit::TextUi::TestRunner runner; // fuer TextUi
  runner.addTest( CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest() );
  runner.setOutputter( new CppUnit::CompilerOutputter( &runner.result(), // fuer TextUi
						       std::cerr ) );        // fuer TextUi 

  bool wasSucessful = runner.run( );

  return wasSucessful ? 0 : 1; 
}
