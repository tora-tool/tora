///////////////////////////////////////////////////////////////////////////////
// Unit Test for Loki
//
// Copyright Terje Slettebø and Pavel Vozenilek 2002.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef FACTORYTEST_H
#define FACTORYTEST_H

// $Id: FactoryTest.h 760 2006-10-17 20:36:13Z syntheticpp $


#include <loki/Factory.h>

///////////////////////////////////////////////////////////////////////////////
// FactoryTest
///////////////////////////////////////////////////////////////////////////////

namespace FactoryTestPrivate
{
  class Shape
  {
  public:
    virtual ~Shape() = 0;
  };

  inline Shape::~Shape() {}

  class Polygon : public Shape
  {
  };

  class Line : public Shape
  {
  };

  class Circle : public Shape
  {
  };

  Polygon *createPolygon() { return new Polygon; }
  Line *createLine() { return new Line; }
  Circle *createCircle() { return new Circle; }

  Polygon *clonePolygon(Polygon *) { return new Polygon; }
  Line *cloneLine(Line *) { return new Line; }
  Circle *cloneCircle(Circle *) { return new Circle; }

  typedef Loki::Factory<Shape, int> FactoryType;

  bool testFactory()
  {
    FactoryType factory;

    factory.Register(1, reinterpret_cast<Shape* (*)()>(createPolygon));
    factory.Register(2, reinterpret_cast<Shape* (*)()>(createCircle));
    factory.Register(3, reinterpret_cast<Shape* (*)()>(createLine));

    Shape *s;

    s = factory.CreateObject(1);
    delete s;
    bool test1=s!=NULL;

    s = factory.CreateObject(2);
    delete s;
    bool test2=s!=NULL;

    s = factory.CreateObject(3);
    delete s;
    bool test3=s!=NULL;

    bool test4=true;

//    try
//    {
//      factory.CreateObject(4);
//
//      test4=false;
//    }
//    catch (std::exception&)
//    {
//    }

    return test1 && test2 && test3 && test4;
  }

  typedef Loki::CloneFactory<Shape> CloneFactoryType;

  bool testCloneFactory()
  {
    CloneFactoryType factory;

    factory.Register(Loki::TypeInfo(typeid(Polygon)), reinterpret_cast<Shape* (*)(const Shape*)>(clonePolygon));
    factory.Register(Loki::TypeInfo(typeid(Circle)), reinterpret_cast<Shape* (*)(const Shape*)>(cloneCircle));
    factory.Register(Loki::TypeInfo(typeid(Line)), reinterpret_cast<Shape* (*)(const Shape*)>(cloneLine));

    Polygon p;
    Circle c;
    Line l;

    Shape *s;

    s = factory.CreateObject(&p);
    delete s;
    bool test1=s!=NULL;

    s = factory.CreateObject(&c);
    delete s;
    bool test2=s!=NULL;

    s = factory.CreateObject(&l);
    delete s;
    bool test3=s!=NULL;

    return test1 && test2 && test3;
  }
}

class FactoryTest : public Test
{
public:
  FactoryTest() : Test("Factory.h") {}

  virtual void execute(TestResult &result)
    {
    printName(result);

    bool test1=FactoryTestPrivate::testFactory();

    bool test2=FactoryTestPrivate::testCloneFactory();

    bool r=test1 && test2;

    testAssert("Factory",r,result);

    std::cout << '\n';
    }
} factoryTest;

#endif
