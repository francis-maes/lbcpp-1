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
| Filename: Type.h                        | The class interface for         |
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

extern void initialize();
extern void deinitialize();

class Type : public NameableObject
{
public:
  Type(const String& className, TypePtr baseType)
    : NameableObject(className), baseType(baseType) {}

  virtual ClassPtr getClass() const;

  static void declare(TypePtr classInstance);
  static TypePtr get(const String& typeName);
  static TypePtr get(const String& typeName, TypePtr argument);
  static TypePtr get(const String& typeName, TypePtr argument1, TypePtr argument2);
  static TypePtr parseAndGet(const String& typeName, ErrorHandler& callback);

  static bool doClassNameExists(const String& className);
  
  /** Creates dynamically an object of type @a typeName.
  **
  ** The type @a typeName must be declared with Type::declare()
  ** and must have a default constructor before being able
  ** to instantiate it dynamically.
  **
  ** @param typeName : type name.
  **
  ** @return an instance of @a typeName object.
  ** @see Type::declare
  */
  static Variable createInstance(const String& typeName);

  /*
  ** Base class
  */
  TypePtr getBaseType() const
    {return baseType;}

  void setBaseType(TypePtr baseType)
    {this->baseType = baseType;}

  bool inheritsFrom(TypePtr baseType) const;
  bool canBeCastedTo(TypePtr targetType) const;

  /*
  ** Template Arguments
  */
  size_t getNumTemplateArguments() const
    {return templateArguments.size();}

  TypePtr getTemplateArgument(size_t index) const
    {jassert(index < templateArguments.size()); return templateArguments[index];}
  
  void setTemplateArgument(size_t index, TypePtr type)
    {jassert(index < templateArguments.size()); templateArguments[index] = type;}

  /*
  ** Operations
  */
  virtual VariableValue getMissingValue() const;
  virtual bool isMissingValue(const VariableValue& value) const;

  virtual VariableValue create() const;
  virtual VariableValue createFromString(const String& value, ErrorHandler& callback) const;
  virtual VariableValue createFromXml(XmlElement* xml, ErrorHandler& callback) const;
  virtual void saveToXml(XmlElement* xml, const VariableValue& value) const;

  virtual void destroy(VariableValue& value) const = 0;
  virtual void copy(VariableValue& dest, const VariableValue& source) const = 0;
  virtual String toString(const VariableValue& value) const;
  virtual String getShortSummary(const VariableValue& value) const
    {return toString(value);}
  virtual int compare(const VariableValue& value1, const VariableValue& value2) const = 0;

  virtual TypePtr multiplyByScalar(VariableValue& value, double scalar)
    {jassert(false); return TypePtr(this);}

  virtual TypePtr addWeighted(VariableValue& target, const Variable& source, double weight)
    {jassert(false); return TypePtr(this);}

  /*
  ** Static Variables
  */
  virtual size_t getObjectNumVariables() const;
  virtual TypePtr getObjectVariableType(size_t index) const;
  virtual String getObjectVariableName(size_t index) const;
  virtual int findObjectVariable(const String& name) const;
  virtual Variable getObjectVariable(const VariableValue& value, size_t index) const;
  virtual void setObjectVariable(const VariableValue& value, size_t index, const Variable& subValue) const;

  /*
  ** Dynamic Variables
  */
  virtual size_t getNumElements(const VariableValue& value) const;
  virtual Variable getElement(const VariableValue& value, size_t index) const;
  virtual String getElementName(const VariableValue& value, size_t index) const;

  /*
  ** Object
  */
  virtual String toString() const
    {return getName();}

  virtual void saveToXml(XmlElement* xml) const
    {xml->addTextElement(getName());}

  virtual void clone(ObjectPtr target) const
  {
    TypePtr targetType = target.staticCast<Type>();
    targetType->baseType = baseType;
    targetType->templateArguments = templateArguments;
  }

  juce_UseDebuggingNewOperator

protected:
  friend class TypeClass;

  TypePtr baseType;
  std::vector<TypePtr> templateArguments; // use Vector ?
};

extern TypePtr topLevelType();

inline TypePtr anyType()
  {return topLevelType();}

extern TypePtr nilType();

class BuiltinType : public Type
{
public:
  BuiltinType(const String& name, TypePtr baseType = topLevelType())
    : Type(name, baseType) {}

  virtual VariableValue createFromXml(XmlElement* xml, ErrorHandler& callback) const
    {return createFromString(xml->getAllSubText(), callback);}

  virtual void saveToXml(XmlElement* xml, const VariableValue& value) const
    {xml->addTextElement(toString(value));}

  virtual size_t getNumElements(const VariableValue& value) const
    {return 0;}

  juce_UseDebuggingNewOperator
};

extern TypePtr booleanType();
extern TypePtr integerType();
  extern TypePtr enumValueType();

extern TypePtr doubleType();
  extern TypePtr probabilityType();
  extern TypePtr angstromDistanceType(); // todo: move

extern TypePtr stringType();
  extern TypePtr fileType();

extern TypePtr pairType();
extern TypePtr pairType(TypePtr firstClass, TypePtr secondClass);

extern TypePtr sumType(TypePtr type1, TypePtr type2);
extern TypePtr sumType(TypePtr type1, TypePtr type2, TypePtr type3);
extern TypePtr sumType(TypePtr type1, TypePtr type2, TypePtr type3, TypePtr type4);
extern TypePtr sumType(const std::vector<TypePtr>& types);

/*
** Integer
*/
class IntegerType : public BuiltinType
{
public:
  IntegerType(const String& className, TypePtr baseType)
    : BuiltinType(className, baseType) {}
  IntegerType() : BuiltinType(T("Integer")) {}

  virtual VariableValue create() const
    {return VariableValue(0);}

  virtual VariableValue createFromString(const String& value, ErrorHandler& callback) const
  {
    if (!value.trim().containsOnly(T("-+e0123456789")))
    {
      callback.errorMessage(T("IntegerType::createFromString"), value.quoted() + T(" is not a valid integer"));
      return getMissingValue();
    }
    return VariableValue(value.getIntValue());
  }

  virtual void destroy(VariableValue& value) const
    {value.clearBuiltin();}

  virtual void copy(VariableValue& dest, const VariableValue& source) const
    {dest.setInteger(source.getInteger());}

  virtual String toString(const VariableValue& value) const
    {return String(value.getInteger());}

  virtual int compare(const VariableValue& value1, const VariableValue& value2) const
    {return (int)(value1.getInteger() - value2.getInteger());}

  juce_UseDebuggingNewOperator
};

/*
** Enumeration
*/
class Enumeration;
typedef ReferenceCountedObjectPtr<Enumeration> EnumerationPtr;

class Enumeration : public IntegerType
{
public:
  Enumeration(const String& name, const juce::tchar** elements, const String& oneLetterCodes = String::empty);
  Enumeration(const String& name, const String& oneLetterCodes);
  Enumeration(const String& name);

  virtual ClassPtr getClass() const;

  static EnumerationPtr get(const String& className)
    {return checkCast<Enumeration>(T("Enumeration::get"), Type::get(className));}

  virtual VariableValue getMissingValue() const
    {return VariableValue((juce::int64)getNumElements());}

  virtual VariableValue createFromString(const String& value, ErrorHandler& callback) const;
  virtual String toString(const VariableValue& value) const;

  size_t getNumElements() const
    {return elements.size();}

  String getElementName(size_t index) const
    {jassert(index < elements.size()); return elements[index];}

  int findElement(const String& name) const;

  bool hasOneLetterCodes() const;
  juce::tchar getOneLetterCode(size_t index) const;
  String getOneLetterCodes() const;

  virtual TypePtr multiplyByScalar(VariableValue& value, double scalar);

  juce_UseDebuggingNewOperator

protected:
  void addElement(const String& elementName, const String& oneLetterCode = String::empty, const String& threeLettersCode = String::empty);

private:
  friend class EnumerationClass;

  std::vector<String> elements; // use Vector ?
  String oneLetterCodes;
};

/*
** Class
*/
class Class : public Type
{
public:
  Class(const String& name, TypePtr baseClass)
    : Type(name, baseClass) {}
  Class() : Type(T("Object"), topLevelType()) {}

  virtual String toString() const;

  virtual bool isMissingValue(const VariableValue& value) const
    {return !value.getObject();}
    
  virtual VariableValue getMissingValue() const
    {return VariableValue();}

  virtual VariableValue createFromString(const String& value, ErrorHandler& callback) const;
  virtual VariableValue createFromXml(XmlElement* xml, ErrorHandler& callback) const;
  virtual void saveToXml(XmlElement* xml, const VariableValue& value) const;

  virtual void destroy(VariableValue& value) const
    {value.clearObject();}

  virtual void copy(VariableValue& dest, const VariableValue& source) const
    {dest.clearObject(); dest.setObject(source.getObjectPointer());}

  virtual String toString(const VariableValue& value) const
    {return value.getObject()->toString();}
  virtual String getShortSummary(const VariableValue& value) const
    {return value.getObject()->getShortSummary();}

  virtual int compare(const VariableValue& value1, const VariableValue& value2) const;

  virtual TypePtr multiplyByScalar(VariableValue& value, double scalar);
  virtual TypePtr addWeighted(VariableValue& target, const Variable& source, double weight);

  juce_UseDebuggingNewOperator
};

typedef ReferenceCountedObjectPtr<Class> ClassPtr;

extern ClassPtr objectClass();
extern ClassPtr typeClass();
extern ClassPtr enumerationClass();

class DefaultClass : public Class
{
public:
  DefaultClass(const String& name, TypePtr baseClass = objectClass());
  DefaultClass(const String& name, const String& baseClass);

  virtual size_t getObjectNumVariables() const;
  virtual TypePtr getObjectVariableType(size_t index) const;
  virtual String getObjectVariableName(size_t index) const;
  virtual int findObjectVariable(const String& name) const;

  void addVariable(TypePtr type, const String& name)
    {addVariable(type.get(), name);}

  void addVariable(Type* type, const String& name);
  void addVariable(const String& typeName, const String& name);

  juce_UseDebuggingNewOperator

protected:
  CriticalSection variablesLock;
  std::vector< std::pair<Type*, String> > variables;
};

typedef ReferenceCountedObjectPtr<DefaultClass> DynamicClassPtr;

/*
** Minimalistic C++ classes Wrapper
*/
template<class TT>
class DefaultAbstractClass_ : public Class
{
public:
  DefaultAbstractClass_(TypePtr baseClass)
    : Class(lbcpp::toString(typeid(TT)), baseClass)
    {jassert(baseClass);}

  virtual VariableValue create() const
  {
    Object::error(T("AbstractClass::create"), T("Cannot instantiate abstract classes"));
    return VariableValue(0);
  }

  juce_UseDebuggingNewOperator
};

template<class TT>
class DefaultClass_ : public Class
{
public:
  DefaultClass_(TypePtr baseClass, size_t numTemplateArguments = 0)
    : Class(lbcpp::toString(typeid(TT)), baseClass)
  {
    jassert(baseClass);
    if (numTemplateArguments)
      templateArguments.resize(numTemplateArguments, topLevelType());
  }

  virtual VariableValue create() const
    {return new TT();}

  virtual ObjectPtr clone() const
  {
    ClassPtr res(new DefaultClass_<TT>(baseType));
    Type::clone(res);
    return res;
  }

  juce_UseDebuggingNewOperator
};

#define LBCPP_DECLARE_ABSTRACT_CLASS(Name, BaseClass) \
  lbcpp::Type::declare(lbcpp::TypePtr(new lbcpp::DefaultAbstractClass_<Name>(lbcpp::Type::get(#BaseClass))))

#define LBCPP_DECLARE_CLASS(Name, BaseClass) \
  lbcpp::Type::declare(lbcpp::TypePtr(new lbcpp::DefaultClass_<Name>(lbcpp::Type::get(#BaseClass))))

#define LBCPP_DECLARE_TEMPLATE_CLASS(Name, NumTemplateArguments, BaseClass) \
  lbcpp::Type::declare(lbcpp::TypePtr(new lbcpp::DefaultClass_<Name>(lbcpp::Type::get(#BaseClass), NumTemplateArguments)))

#define LBCPP_DECLARE_CLASS_LEGACY(Name) \
  lbcpp::Type::declare(lbcpp::TypePtr(new lbcpp::DefaultClass_<Name>(objectClass())))

inline bool checkInheritance(TypePtr type, TypePtr baseType, ErrorHandler& callback = ErrorHandler::getInstance())
{
  jassert(baseType);
  if (!type || !type->inheritsFrom(baseType))
  {
    callback.errorMessage(T("checkInheritance"), T("Invalid type, Expected ") + baseType->getName().quoted() + T(" found ") + (type ? type->getName().quoted() : T("Nil")));
    return false;
  }
  return true;
}

/*
** TypeCache
*/
class TypeCache
{
public:
  TypeCache(const String& typeName);

  TypePtr operator ()()
    {return TypePtr(type);}

protected:
  Type* type;
};

class UnaryTemplateTypeCache
{
public:
  UnaryTemplateTypeCache(const String& typeName)
    : typeName(typeName) {}

  TypePtr operator ()(TypePtr argument);

private:
  String typeName;
  std::map<Type*, Type*> m;
};

class BinaryTemplateTypeCache
{
public:
  BinaryTemplateTypeCache(const String& typeName)
    : typeName(typeName) {}

  TypePtr operator ()(TypePtr argument1, TypePtr argument2);

private:
  String typeName;
  std::map<std::pair<Type*, Type*>, Type*> m;
};

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_CLASS_H_
