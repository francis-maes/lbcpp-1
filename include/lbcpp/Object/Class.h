/*
** This file is part of the LBC++ library - "Learning Based C++"
** Copyright (C) 2009 by Francis Maes, francis.maes@lip6.fr.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*-----------------------------------------.---------------------------------.
| Filename: Class.h                        | The class interface for         |
| Author  : Francis Maes                   |  introspection                  |
| Started : 24/06/2010 11:28               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OBJECT_CLASS_H_
# define LBCPP_OBJECT_CLASS_H_

# include "Object.h"
# include "impl/VariableValue.hpp"

namespace lbcpp
{

class Class : public NameableObject
{
public:
  Class(const String& className, ClassPtr baseClass)
    : NameableObject(className), baseClass(baseClass) {}

  static void declare(ClassPtr classInstance);
  static ClassPtr get(const String& className);
  static bool doClassNameExists(const String& className);
  
  /** Creates dynamically an object of class @a className.
  **
  ** The class @a className must be declared with Class::declare()
  ** and must have a default constructor before being able
  ** to instantiate it dynamically.
  **
  ** @param className : class name.
  **
  ** @return an instance of @a className object.
  ** @see Object::declare
  */
  static ObjectPtr createInstance(const String& className);

  template<class T>
  static ReferenceCountedObjectPtr<T> createInstanceAndCast(const String& className)
    {return checkCast<T>(T("Class::createInstanceAndCast"), create(className));}

  /*
  ** Default Constructor
  */
  typedef ObjectPtr (*DefaultConstructor)();

  DefaultConstructor getDefaultConstructor() const
    {return defaultConstructor;}

  /*
  ** Base class
  */
  ClassPtr getBaseClass() const
    {return baseClass;}

  bool inheritsFrom(ClassPtr baseClass) const;

  /*
  ** Operations
  */
  virtual void destroy(VariableValue& value) const
    {jassert(false);}

  virtual void copy(VariableValue& dest, const VariableValue& source) const
    {jassert(false);}

  virtual String toString(const VariableValue& value) const
    {jassert(false); return String::empty;}

  virtual bool equals(const VariableValue& value1, const VariableValue& value2) const
    {jassert(false); return false;}

  virtual size_t getNumSubVariables(const VariableValue& value) const
    {jassert(false); return 0;}

  virtual Variable getSubVariable(const VariableValue& value, size_t index) const;
  virtual String getSubVariableName(const VariableValue& value, size_t index) const
    {jassert(false); return String::empty;}
  virtual void setSubVariable(const VariableValue& value, size_t index, const VariableValue& subValue) const
    {jassert(false);}
  
  /*
  ** Sub-Variables
  */
  size_t getNumVariables() const;
  ClassPtr getVariableType(size_t index) const;
  const String& getVariableName(size_t index) const;
  int findVariable(const String& name) const;

protected:
  DefaultConstructor defaultConstructor;
  ClassPtr baseClass;
  std::vector< std::pair<ClassPtr, String> > variables;
  CriticalSection variablesLock;

  void addVariable(ClassPtr type, const String& name);
  void addVariable(const String& typeName, const String& name)
    {addVariable(Class::get(typeName), name);}
};

/*
** The top-level base class
*/
class TopLevelClass : public Class
{
public:
  TopLevelClass() : Class(T("Variable"), ClassPtr()) {}
};

extern ClassPtr topLevelClass();

class BuiltinTypeClass : public Class
{
public:
  BuiltinTypeClass(const String& name)
    : Class(name, topLevelClass()) {}
};

extern ClassPtr booleanClass();
extern ClassPtr integerClass();
extern ClassPtr doubleClass();
extern ClassPtr stringClass();

extern ClassPtr pairClass();

/*
** Integer
*/
class IntegerClass : public BuiltinTypeClass
{
public:
  IntegerClass(const String& className)
    : BuiltinTypeClass(className) {}
  IntegerClass() : BuiltinTypeClass(T("Integer")) {}

  virtual void destroy(VariableValue& value) const
    {}

  virtual void copy(VariableValue& dest, const VariableValue& source) const
    {dest.setInteger(source.getInteger());}

  virtual String toString(const VariableValue& value) const
    {return String(value.getInteger());}

  virtual bool equals(const VariableValue& value1, const VariableValue& value2) const
    {return value1.getInteger() == value2.getInteger();}

  virtual size_t getNumSubVariables(const VariableValue& value) const
    {return 0;}
};

typedef ReferenceCountedObjectPtr<IntegerClass> IntegerClassPtr;

/*
** Enumeration
*/
class Enumeration;
typedef ReferenceCountedObjectPtr<Enumeration> EnumerationPtr;

class Enumeration : public IntegerClass
{
public:
  Enumeration(const String& name, const juce::tchar** elements);
  Enumeration(const String& name, const String& elementChars);
  Enumeration(const String& name);

  static EnumerationPtr get(const String& className)
    {return checkCast<Enumeration>(T("Enumeration::get"), Class::get(className));}

  virtual String toString(const VariableValue& value) const
  {
    int val = value.getInteger();
    return val >= 0 && val < (int)getNumElements() ? getElementName(val) : T("N/A");
  }

  size_t getNumElements() const
    {return elements.size();}

  String getElementName(size_t index) const
    {jassert(index < elements.size()); return elements[index];}

protected:
  void addElement(const String& elementName);

private:
  std::vector<String> elements;
};

/*
** ObjectClass
*/
class ObjectClass : public Class
{
public:
  ObjectClass(const String& name, ClassPtr baseClass)
    : Class(name, baseClass) {}
  ObjectClass() : Class(T("Object"), topLevelClass()) {}

  virtual void destroy(VariableValue& value) const
    {value.clearObject();}

  virtual void copy(VariableValue& dest, const VariableValue& source) const
    {dest.clearObject(); dest.setObject(source.getObjectPointer());}

  virtual String toString(const VariableValue& value) const
    {return value.getObject()->toString();}

  virtual bool equals(const VariableValue& value1, const VariableValue& value2) const
    {return value1.getObject() == value2.getObject();}
};

extern ClassPtr objectClass();

/*
** Collection
*/
class Collection;
typedef ReferenceCountedObjectPtr<Collection> CollectionPtr;

class Collection : public ObjectClass
{
public:
  Collection(const String& name)
    : ObjectClass(name, objectClass()) {}

  static CollectionPtr get(const String& className)
    {return checkCast<Collection>(T("Collection::get"), Class::get(className));}

  size_t getNumElements() const
    {return objects.size();}

  ObjectPtr getElement(size_t index) const
    {jassert(index < objects.size()); return objects[index];}

protected:
  void addElement(ObjectPtr object)
    {objects.push_back(object);}

private:
  std::vector<ObjectPtr> objects;
};

/*
** Minimalistic C++ classes Wrapper
*/
template<class Type>
class DefaultAbstractClass_ : public ObjectClass
{
public:
  DefaultAbstractClass_(ClassPtr baseClass)
    : ObjectClass(lbcpp::toString(typeid(Type)), baseClass)
    {}
};

template<class Type>
class DefaultClass_ : public ObjectClass
{
public:
  DefaultClass_(ClassPtr baseClass)
    : ObjectClass(lbcpp::toString(typeid(Type)), baseClass)
    {Class::defaultConstructor = defaultCtor;}

  static ObjectPtr defaultCtor()
    {return ObjectPtr(new Type());}
};

#define LBCPP_DECLARE_ABSTRACT_CLASS(Name, BaseClass) \
  lbcpp::Class::declare(lbcpp::ClassPtr(new lbcpp::DefaultAbstractClass_<Name>(lbcpp::Class::get(#BaseClass))))

#define LBCPP_DECLARE_CLASS(Name, BaseClass) \
  lbcpp::Class::declare(lbcpp::ClassPtr(new lbcpp::DefaultClass_<Name>(lbcpp::Class::get(#BaseClass))))

#define LBCPP_DECLARE_CLASS_LEGACY(Name) \
  lbcpp::Class::declare(lbcpp::ClassPtr(new lbcpp::DefaultClass_<Name>(objectClass())))

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_CLASS_H_
