/*
 * This file was download from:
 * http://codewrangler.home.comcast.net/~codewrangler/utility/factory.h
 * and refreshed a little bit by ibre5041
 *
 * Copyright (c) 2001 by Cliff Green. All rights reserved. Individual files
 * may be covered by other copyrights (as noted in the file itself).
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that this entire copyright notice is duplicated in all such
 * copies.
 *
 * This software is provided "as is" and without any expressed or implied
 * warranties, including, without limitation, the implied warranties of
 * merchantibility and fitness for any particular purpose.
 */
//----------------------------------------------------------------------
// Source file:   factory.h
// Written by:    Jim Hyslop, Cliff Green, 2001
// Compiler:      Metrowerks CodeWarrior Pro 6
// History:
//   Modified:    8/28/2001
//   By:          Cliff Green
//   Comments:    Modified to use typelist and typetraits facilities from
//                Andrei Alexandrescu's Loki library.
//----------------------------------------------------------------------
// Although significantly modified from the original, this code was
// inspired by Herb Sutter and Jim Hyslop's object factory template,
// and the original copyright notice from Jim appears here:
//----------------------------------------------------------------------
//
// Copyright 2001, Jim Hyslop.
// This file may be freely used, modified and distributed, provided that
// the accompanying copyright notice remains intact.
//
//----------------------------------------------------------------------
// This commentheader supports documentation tools such as Doc++ and
// Doxygen: http://www.doxygen.org/
//----------------------------------------------------------------------
/// Generic object factory template classes.
/**
 *  This generic object factory template is a utility used in creating
 *  objects of related (by inheritance) types. It can be used as a building
 *  block in Abstract Factory patterns, or "as is" without attempting to
 *  relate sets of derived types together.
 *
 *  The Abstract Class Factory pattern is discussed in "Design Patterns:
 *  Elements of Reusable Object-Oriented Software", E. Gamma, R. Helm,
 *  R. Johnson, J. Vlissides, Addison Wesley [1995]
 *
 *  This utility requires the Loki library:
 *     http://www.moderncppdesign.com/
 *
 *  This utility is based on code provided by Herb Sutter and Jim Hyslop
 *  from the June edition of the Experts Forum, C/C++ Users Journal
 *  (http://www.cuj.com/). Quite a bit of the original code from Herb and Jim
 *  is still present, but has been substantially added to and expanded (with
 *  the typelist and typetraits facilities from Andrei Alexandrescu). Note
 *  that Andrei has a similar templatized Object and Abstract Factory in his
 *  Loki library that is more flexible (except for the constructor parameters)
 *  but I wanted to develop my own version based on the foundation from
 *  Herb and Jim (mostly as a learning exercise).
 *
 *  There are two main differences from the original code from Hyslop:
 *    1. Constructor parameter possibilities have been expanded from
 *       no parameters to up to four parameters.
 *    2. The GenericFactory class is expected to be wrapped in a Loki
 *       SingletonHolder (the RegisterInFactory template expects it, or
 *       a similar interface).
 *
 *  The additional constructor parameters are implemented by using Andrei's
 *  typelist facilities, and partially specializing on 0 to 4 parameters
 *  (resulting in five versions of the template classes). All of the
 *  function parameters are enhanced using Loki TypeTraits<T>::ParameterType
 *  facilities (for efficient passing of function arguments).
 *
 *  There are two template class utilities:
 *    1. The RegisterInFactory class, which simplifies registering a creation
 *       function in GenericFactory. In particular, this allows registering
 *       creation functions in appropriate implementation files (e.g. .cpp
 *       files corresponding to each derived class) in an unnamed namespace.
 *       This separates registration of derived classes from a centralized
 *       point (e.g. 'main' function) into a more specific area (where the
 *       derived classes are implemented).
 *    2. A GenericFactory class, which has an implementation class that is
 *       specialized five times (for up to four constructor parameters).
 *
 *  To use the GenericFactory template, you need to provide a base class type,
 *  a list of constructor parameter types (defaults to empty, which means a
 *  constructor taking no parameters) and an optional key type. The key class
 *  must be able to be used as a key in a std::map, i.e. it must
 *    1. implement copy and assignment semantics
 *    2. provide bool operator< () const;
 *  The default for the key class is std::string.
 *
 *  Steps to using the factory:
 *    1. Create the base class and its derivatives.
 *    2. Register each class in the factory by instantiating a
 *       RegisterInFactory<> template class - do this in one file only (the
 *       class implementation file is the perfect place for this). Note that
 *       the RegisterInFactory template takes a derived type as one of the
 *       template parameters, in addition to the GenericFactory (instantiated
 *       as a SingletonHolder).
 *    3. Create the object by calling create() and passing it the same
 *       key value used when you instantiated the RegisterInFactory object.
 *       If constructor parameter types were specified, the constructor
 *       values are passed in to the create() method.
 *
 *  The RegisterInFactory class is used by instantiating it (which registers a static
 *  object creation method in a GenericFactory object). RegisterInFactory has three
 *  template parameters:
 *    1.  Derived type, for object creation (application defined - if the base class
 *        is not an Abstract Base Class and instances of the Base class can be
 *        created, the Derived type may be the same as the Base type for object creation).
 *    2.  Singleton type declaration
 *    3.  Class id key value type (defaults to std::string)
 *  Since all the work is done in the RegisterInFactory constructor, no methods are
 *  available.
 *
 *  It is highly recommended (mandatory for readability) that typedefs are used for
 *  the GenericFactory and SingletonHolder instantiated types.
 *
 *  The GenericFactory class has two public methods available, and is instantiated
 *  with the following template parameters:
 *    1.  Base type
 *    2.  Typelist of constructor parameters
 *    3.  Class id key value type (same as above)
 *  The public methods are:
 *    1.  regCreateFct - called once per program by classes derived from the
 *        Base type (first template parameter) to register the class Id key, and a
 *        pointer to the function that creates the class. This simply associates
 *        the ClassIdKey with the function used to create the class.
 *        This method is typically not used in application code, since it is called
 *        in the RegisterInFactory constructor.
 *    2.  create - called to create a new object specified by the className parameter
 *        value (key value), passing in up to four additional constructor arguments.
 *
 *  The Loki library typelist facilities are used, and are created using the LOKI_TYPELIST_n
 *  macros, where n is from 1 to 4. If no constructor parameters are present,
 *  Loki::NullType is used instead.
 *
 *  The constructor parameters use the Loki TypeTraits<T>::ParameterType facilities,
 *  which resolve the ClassIdKey and typelist types with the following logic:
 *    If T is a fundamental type, use T (pass by value)
 *    Else if T is a reference type, keep T unchanged
 *    Else use const T& (pass by const reference, for efficiency)
 *  This allows simple specification of types for the ClassIdKey and in the constructor
 *  parm typelist, but efficient function argument usage.
 *
 *  Example usages:
 *
 *  Assume class hierarchy, base class Vehicle, derived classes SportsCar and Truck.
 *  Vehicle has no ctor parms, SportsCar has two ctors, one with no parms, one taking
 *  std::string and an unsigned short, and Truck has one ctor, taking std::string.
 *  Vehicle is not an ABC, so an example is shown of creating a Vehicle object. If
 *  Vehicle was an ABC, only derived objects would be created. The Truck class is
 *  registered with an int as the class id key type.
 *
 *  First the typedefs for the GenericFactory:
 *
 *    typedef Util::GenericFactory<Vehicle>                 VehFactNoParm;
 *    typedef Util::GenericFactory<Vehicle,
 *                                 LOKI_TYPELIST_1(std::string),
 *                                 int >                    VehFactOneParm;
 *    typedef Util::GenericFactory<Vehicle,
 *                                 LOKI_TYPELIST_2(std::string, unsigned short)
 *                                 >                        VehFactTwoParms;
 *
 *  Next the typedefs for the SingletonHolder (note that various combinations
 *  of SingletonHolder policies can be specified - these are examples only):
 *
 *    typedef Loki::SingletonHolder<VehFactNoParm,
 *                                  Loki::CreateStatic>     VehFactNoParmSing;
 *
 *    typedef Loki::SingletonHolder<VehFactOneParm>         VehFactOneParmSing;
 *
 *    typedef Loki::SingletonHolder<VehFactTwoParms,
 *                                  Loki::CreateUsingMalloc,
 *                                  Loki::PhoenixSingleton> VehFactTwoParmsSing;
 *
 *  Now the factory registration (should be in appropriate implementation file,
 *  preferably in an unnamed namespace):
 *
 *    Util::RegisterInFactory<Vehicle, VehFactNoParmSing> regVeh("Vehicle");
 *    Util::RegisterInFactory<SportsCar, VehFactNoParmSing> regSpCar1("SportsCar");
 *    Util::RegisterInFactory<SportsCar, VehFactTwoParmsSing> regSpCar2("SportsCar");
 *    Util::RegisterInFactory<Truck, VehFactOneParmSing, int> regTruck(150);
 *
 *  Example code that instantiates the classes (creates objects through the factory):
 *
 *    std::auto_ptr<Vehicle> newVeh = VehFactNoParmSing::Instance().create("Vehicle");
 *    std::auto_ptr<Vehicle> newSpCar1 = VehFactNoParmSing::Instance().create("SportsCar");
 *    std::auto_ptr<Vehicle> newSpCar2 =
 *       VehFactTwoParmsSing::Instance().create("SportsCar", "Red", 220);
 *    std::auto_ptr<Vehicle> newTruck =
 *       VehFactOneParmSing::Instance().create(150, "Big Lights");
 *
 *  Example code using the objects (assume callVirtFunc take Vehicle object by
 *  reference):
 *
 *    callVirtFunc(*newVeh);
 *    callVirtFunc(*newSpCar1);
 *    callVirtFunc(*newSpCar2);
 *    callVirtFunc(*newTruck);
 *
 *  New derivatives can be added without affecting the existing code.
 *
 *  Implementation notes:
 *
 *  GenericFactory is publicly derived from GenericFactoryImpl, which is specialized
 *  five times (for the various flavors of construction possibilities). This results in
 *  some code 'copy and paste' style duplication, but that's ok for a utility component
 *  like this.
 *
 *  The create method simply looks up the class ID, and if it's in the map container, the
 *  statement "(*i).second();" calls the function (with various combinations of function
 *  parameters).
 */


#ifndef FACTORY_H
#define FACTORY_H

#include <map>
#include <string>
#include <memory>
#include <vector>

#include "loki/Typelist.h"
#include "loki/Singleton.h"
#include "loki/TypeTraits.h"
#include "loki/TypelistMacros.h"

namespace Util {
  namespace Private {

	  /** Used only in GenericFactory::keys()
	  */
	  struct RetrieveKey
      {
		template <typename T>
		typename T::first_type operator()(T keyValuePair) const
		{
			return keyValuePair.first;
		}
	  };
  }// end namespace

  
  typedef std::string DefaultIdKeyType;

  // RegisterInFactory helper template to make registration painless and simple.
  // Allows creation of a temporary RegisterInFactory object, which performs
  // the Singleton access and calls the registerInFactory method.
  template <class Derived,
    class LokiSingleton,
    class ClassIdKey=DefaultIdKeyType >
    class RegisterInFactory
    {
      public:
      typedef typename Loki::TypeTraits<ClassIdKey>::ParameterType ClassIdKeyParm;
      // typedef ClassIdKey ClassIdKeyParm;
	  RegisterInFactory(ClassIdKeyParm id) {
		  LokiSingleton::Instance().template registerInFactory<Derived>(id);
		  //std::cout << "Registered:" << id << std::endl;
	  }
      // ~RegisterInFactory() { } // implicit
      private:
      RegisterInFactory (const RegisterInFactory& ); // disable copy ctor
      RegisterInFactory& operator= (const RegisterInFactory& ); // disable assign
    };

  // GenericFactoryImpl declaration
  // This declaration is partially specialized further below, for the various
  // implementations of constructor parameters.
  template <class Base,
    typename CtorTypeList=Loki::NullType,
    class ClassIdKey=DefaultIdKeyType>
    class GenericFactoryImpl;

  // GenericFactory class template, which uses specializations of the
  // GenericFactoryImpl class.
  template <class Base,
    typename CtorTypeList=Loki::NullType,
    typename ClassIdKey=DefaultIdKeyType>
    class GenericFactory : public GenericFactoryImpl<Base, CtorTypeList, ClassIdKey>
    {
      public:

      typedef GenericFactoryImpl<Base, CtorTypeList, ClassIdKey> Impl;

      typedef typename Impl::CreateFct CreateFct;
      typedef typename Loki::TypeTraits<ClassIdKey>::ParameterType ClassIdKeyParm;
      typedef GenericFactoryImpl<Base, CtorTypeList, ClassIdKey> parent;

	  void regCreateFct(ClassIdKeyParm className, CreateFct func) {
		  //std::cout << "Registering:" << className << std::endl;
		  GenericFactoryImpl<Base, CtorTypeList, ClassIdKey>::mRegistry[className] = func;
	  }

      template <class Derived>
	  void registerInFactory (ClassIdKeyParm id) {
		  regCreateFct( id, Impl::template createInstance<Derived> );
	  }

	  void dump() const {
	      typename parent::FctRegistry::const_iterator i;
		  for(i = parent::mRegistry.begin(); i != parent::mRegistry.end(); ++i)
		  {
			  //std::cout << "Key:" << i->first << std::endl;
		  }
	  }

      std::vector<ClassIdKey> keys() {
          std::vector<ClassIdKey> retval;
		  transform(parent::mRegistry.begin(), parent::mRegistry.end(), back_inserter(retval), Private::RetrieveKey());
	      return retval;
      }

      private:

      friend struct Loki::CreateUsingNew<GenericFactory<Base,CtorTypeList,ClassIdKey> >;
      friend struct Loki::CreateUsingMalloc<GenericFactory<Base,CtorTypeList,ClassIdKey> >;
      friend struct Loki::CreateStatic<GenericFactory<Base,CtorTypeList,ClassIdKey> >;

      GenericFactory() : Impl() { } // only allow Singleton to create
      ~GenericFactory() { }         // only allow Singleton to destroy

      GenericFactory (const GenericFactory& ); // disable copy ctor
      GenericFactory& operator= (const GenericFactory& ); // disable assign op

    };


  // GenericFactoryImpl specializations, corresponding to each ctor parm set
  // First specialization - class constructor with no parameters
  template <class Base, typename ClassIdKey>
    struct GenericFactoryImpl<Base, Loki::NullType, ClassIdKey>
    {
      typedef typename Loki::TypeTraits<ClassIdKey>::ParameterType ClassIdKeyParm;
      typedef std::auto_ptr<Base> (*CreateFct)();
      typedef std::map<ClassIdKey, CreateFct> FctRegistry;

      public:

      template <class Derived>
      static std::auto_ptr<Base> createInstance() {
    	  return std::auto_ptr<Base>(new Derived());
      }

      std::auto_ptr<Base> create (ClassIdKeyParm className) const {
    	  typename FctRegistry::const_iterator regEntry(mRegistry.find(className));
    	  return (regEntry == mRegistry.end() ? std::auto_ptr<Base>(0) : (*regEntry).second() );
      }

      Base* createPtr (ClassIdKeyParm className) const {
    	  typename FctRegistry::const_iterator regEntry(mRegistry.find(className));
    	  return (regEntry == mRegistry.end() ? NULL : (*regEntry).second().release() );
      }

      protected:
      FctRegistry mRegistry;
      ~GenericFactoryImpl() { }
    };

  // Second specialization - class constructor with one parameter
    template <class Base,
    typename P1,
    typename ClassIdKey>
    struct GenericFactoryImpl<Base, LOKI_TYPELIST_1(P1), ClassIdKey>
    {
    	typedef typename Loki::TypeTraits<ClassIdKey>::ParameterType ClassIdKeyParm;
	  typedef typename Loki::TypeTraits<P1>::ParameterType P1Parm;
	  typedef std::auto_ptr<Base> (*CreateFct)(P1Parm);
	  typedef std::map<ClassIdKey, CreateFct> FctRegistry;

	  template <class Derived>
	  static std::auto_ptr<Base> createInstance(P1Parm parm1) {
		  return std::auto_ptr<Base>(new Derived(parm1));
	  }

	  std::auto_ptr<Base> create(ClassIdKeyParm className, P1Parm parm1) const {
		  typename FctRegistry::const_iterator regEntry (mRegistry.find(className));
		  return (regEntry == mRegistry.end() ? std::auto_ptr<Base>(0) : (*regEntry).second(parm1) );
	  }

	  Base* createPtr(ClassIdKeyParm className, P1Parm parm1) const {
		  typename FctRegistry::const_iterator regEntry (mRegistry.find(className));
		  return (regEntry == mRegistry.end() ? NULL : (*regEntry).second(parm1).release() );
	  }

  protected:
	  FctRegistry mRegistry;
	  ~GenericFactoryImpl() { }

  };

  // Third specialization - class constructor with two parameters
  template <class Base,
    typename P1,
    typename P2,
    typename ClassIdKey>
    struct GenericFactoryImpl<Base, LOKI_TYPELIST_2(P1, P2), ClassIdKey>
  {
	  typedef typename Loki::TypeTraits<ClassIdKey>::ParameterType ClassIdKeyParm;
	  typedef typename Loki::TypeTraits<P1>::ParameterType P1Parm;
	  typedef typename Loki::TypeTraits<P2>::ParameterType P2Parm;
	  typedef std::auto_ptr<Base> (*CreateFct)(P1Parm, P2Parm);
	  typedef std::map<ClassIdKey, CreateFct> FctRegistry;

	  template <class Derived>
	  static std::auto_ptr<Base> createInstance(P1Parm parm1, P2Parm parm2) {
		  return std::auto_ptr<Base>(new Derived(parm1, parm2));
	  }

	  std::auto_ptr<Base> create(ClassIdKeyParm className, P1Parm parm1, P2Parm parm2) const
	  {
		  typename FctRegistry::const_iterator regEntry (mRegistry.find(className));
		  return (regEntry == mRegistry.end() ? std::auto_ptr<Base>(0) : (*regEntry).second(parm1, parm2) );
	  }

	  Base* createPtr(ClassIdKeyParm className, P1Parm parm1, P2Parm parm2) const
	  {
		  typename FctRegistry::const_iterator regEntry (mRegistry.find(className));
		  return (regEntry == mRegistry.end() ? NULL : (*regEntry).second(parm1, parm2).release() );
	  }

    protected:
    	FctRegistry mRegistry;
    	~GenericFactoryImpl() { }

  };

  // Fourth specialization - class constructor with three parameters
  template <class Base,
    typename P1,
    typename P2,
    typename P3,
    typename ClassIdKey>
    struct GenericFactoryImpl<Base, LOKI_TYPELIST_3(P1, P2, P3), ClassIdKey> {

    typedef typename Loki::TypeTraits<ClassIdKey>::ParameterType ClassIdKeyParm;
    typedef typename Loki::TypeTraits<P1>::ParameterType P1Parm;
    typedef typename Loki::TypeTraits<P2>::ParameterType P2Parm;
    typedef typename Loki::TypeTraits<P3>::ParameterType P3Parm;
    typedef std::auto_ptr<Base> (*CreateFct)(P1Parm, P2Parm, P3Parm);
    typedef std::map<ClassIdKey, CreateFct> FctRegistry;

    template <class Derived>
      static std::auto_ptr<Base> createInstance(P1Parm parm1, P2Parm parm2, P3Parm parm3) {
      return std::auto_ptr<Base>(new Derived(parm1, parm2, parm3));
    }

    std::auto_ptr<Base> create(ClassIdKeyParm className, P1Parm parm1, P2Parm parm2, P3Parm parm3) const
    {
    	typename FctRegistry::const_iterator regEntry (mRegistry.find(className));
    	return (regEntry == mRegistry.end() ? std::auto_ptr<Base>(0) : (*regEntry).second(parm1, parm2, parm3) );
    }

    Base* createPtr(ClassIdKeyParm className, P1Parm parm1, P2Parm parm2, P3Parm parm3) const
    {
    	typename FctRegistry::const_iterator regEntry (mRegistry.find(className));
    	return (regEntry == mRegistry.end() ? NULL : (*regEntry).second(parm1, parm2, parm3).release() );
    }

    protected:
    FctRegistry mRegistry;
    ~GenericFactoryImpl() { }

  };

  // Fifth specialization - class constructor with four parameters
  template <class Base,
    typename P1,
    typename P2,
    typename P3,
    typename P4,
    typename ClassIdKey>
    struct GenericFactoryImpl<Base, LOKI_TYPELIST_4(P1, P2, P3, P4), ClassIdKey> {

    typedef typename Loki::TypeTraits<ClassIdKey>::ParameterType ClassIdKeyParm;
    typedef typename Loki::TypeTraits<P1>::ParameterType P1Parm;
    typedef typename Loki::TypeTraits<P2>::ParameterType P2Parm;
    typedef typename Loki::TypeTraits<P3>::ParameterType P3Parm;
    typedef typename Loki::TypeTraits<P4>::ParameterType P4Parm;
    typedef std::auto_ptr<Base> (*CreateFct)(P1Parm, P2Parm, P3Parm, P4Parm);
    typedef std::map<ClassIdKey, CreateFct> FctRegistry;

    template <class Derived>
    static std::auto_ptr<Base> createInstance(P1Parm parm1, P2Parm parm2, P3Parm parm3, P4Parm parm4)
    {
    	return std::auto_ptr<Base>(new Derived(parm1, parm2, parm3, parm4));
    }

    std::auto_ptr<Base> create(ClassIdKeyParm className, P1Parm parm1, P2Parm parm2, P3Parm parm3, P4Parm parm4) const
    {
    	typename FctRegistry::const_iterator regEntry (mRegistry.find(className));
    	return (regEntry == mRegistry.end() ? std::auto_ptr<Base>(0) : (*regEntry).second(parm1, parm2, parm3, parm4) );
    }

    Base* createPtr(ClassIdKeyParm className, P1Parm parm1, P2Parm parm2, P3Parm parm3, P4Parm parm4) const
    {
    	typename FctRegistry::const_iterator regEntry (mRegistry.find(className));
    	return (regEntry == mRegistry.end() ? NULL : (*regEntry).second(parm1, parm2, parm3, parm4).release() );
    }

    protected:
    FctRegistry mRegistry;
    ~GenericFactoryImpl() { }

  };


  // ---------- move to implementation file when 'export' is supported


  // ---------- end of template function implementations
} // end namespace

#endif
