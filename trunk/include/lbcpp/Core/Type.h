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
| Filename: Type.h                         | The class interface for         |
| Author  : Francis Maes                   |  introspection                  |
| Started : 24/06/2010 11:28               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_TYPE_H_
# define LBCPP_CORE_TYPE_H_

# include "Object.h"
# include "../Execution/ExecutionContext.h"
# include "impl/VariableValue.hpp"

namespace lbcpp
{

class VariableSignature : public NameableObject
{
public:
  VariableSignature(TypePtr type,
                    const String& name,
                    const String& shortName = String::empty,
                    const String& description = String::empty)
    : NameableObject(name), type(type), shortName(shortName), description(description) {}

  VariableSignature() {}

  const TypePtr& getType() const
    {return type;}

  void setType(TypePtr t)
    {type = t;}

  const String& getShortName() const
    {return shortName;}

  void setShortName(const String& shortName)
    {this->shortName = shortName;}

  const String& getDescription() const
    {return description;}

  void setDescription(const String& description)
    {this->description = description;}

protected:
  friend class VariableSignatureClass;

  TypePtr type;
  String shortName;
  String description;
};

class Type : public NameableObject
{
public:
  Type(const String& className, TypePtr baseType);
  Type(TemplateTypePtr templateType, const std::vector<TypePtr>& templateArguments, TypePtr baseType);
  Type() {}
  virtual ~Type();

  /*
  ** Initialization
  */
  bool isInitialized() const
    {return initialized;}

  virtual bool initialize(ExecutionContext& context);
  virtual void deinitialize();

  /*
  ** Name
  */
  virtual bool isUnnamedType() const;

  /*
  ** Type operations
  */
  TypePtr getBaseType() const
    {return baseType;}

  void setBaseType(TypePtr baseType)
    {this->baseType = baseType;}

  bool inheritsFrom(TypePtr baseType) const;
  bool canBeCastedTo(TypePtr targetType) const;

  static TypePtr findCommonBaseType(TypePtr type1, TypePtr type2);

  /*
  ** Template Arguments
  */
  TemplateTypePtr getTemplate() const
    {return templateType;}

  const std::vector<TypePtr>& getTemplateArguments() const
    {return templateArguments;}

  size_t getNumTemplateArguments() const
    {return templateArguments.size();}
  
  TypePtr getTemplateArgument(size_t index) const
    {jassert(index < templateArguments.size()); return templateArguments[index];}
  
  /*
  ** Operations
  */
  virtual VariableValue getMissingValue() const;
  virtual bool isMissingValue(const VariableValue& value) const;

  virtual VariableValue create(ExecutionContext& context) const;
  virtual VariableValue createFromString(ExecutionContext& context, const String& value) const;
  virtual VariableValue createFromXml(XmlImporter& importer) const;
  virtual void saveToXml(XmlExporter& exporter, const VariableValue& value) const;

  virtual void destroy(VariableValue& value) const;
  virtual void copy(VariableValue& dest, const VariableValue& source) const;
  virtual String toString(const VariableValue& value) const;
  virtual String toShortString(const VariableValue& value) const
    {return toString(value);}
  virtual int compare(const VariableValue& value1, const VariableValue& value2) const;

  /*
  ** Member Variables
  */
  virtual size_t getNumMemberVariables() const;
  virtual VariableSignaturePtr getMemberVariable(size_t index) const;

  TypePtr getMemberVariableType(size_t index) const;
  String getMemberVariableName(size_t index) const;
  String getMemberVariableShortName(size_t index) const;
  String getMemberVariableDescription(size_t index) const;

  virtual int findMemberVariable(const String& name) const;
  virtual Variable getMemberVariableValue(const Object* pthis, size_t index) const;
  virtual void setMemberVariableValue(ExecutionContext& context, Object* pthis, size_t index, const Variable& subValue) const;

  /*
  ** Object
  */
  virtual ClassPtr getClass() const;

  virtual void saveToXml(XmlExporter& exporter) const;
  static TypePtr loadUnnamedTypeFromXml(XmlImporter& importer);

  virtual String toString() const
    {return getName();}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class TypeClass;

  bool initialized;

  TypePtr baseType;
  TemplateTypePtr templateType;
  std::vector<TypePtr> templateArguments;
};

extern TypePtr variableType;

// synonims of variableType
extern TypePtr topLevelType;
extern TypePtr anyType;

extern TypePtr nilType;

extern TypePtr booleanType;
extern TypePtr integerType;
  extern TypePtr positiveIntegerType;
    extern TypePtr variableIndexType;
  extern TypePtr enumValueType;

extern TypePtr doubleType;
  extern TypePtr positiveDoubleType;
    extern TypePtr negativeLogProbabilityType;
  extern TypePtr probabilityType;

extern TypePtr stringType;
  extern TypePtr fileType;

extern TypePtr sumType(TypePtr type1, TypePtr type2);
extern TypePtr sumType(TypePtr type1, TypePtr type2, TypePtr type3);
extern TypePtr sumType(TypePtr type1, TypePtr type2, TypePtr type3, TypePtr type4);
extern TypePtr sumType(const std::vector<TypePtr>& types);

/*
** Enumeration
*/
class EnumerationElement : public Object
{
public:
  EnumerationElement(const String& name = String::empty, const String& oneLetterCode = String::empty, const String& shortName = String::empty, const String& description = String::empty)
    : name(name), oneLetterCode(oneLetterCode), shortName(shortName), description(description) {}

  virtual String getName() const
    {return name;}

  const String& getOneLetterCode() const
    {return oneLetterCode;}

  const String& getShortName() const
    {return shortName;}

  const String& getDescription() const
    {return description;}

private:
  friend class EnumerationElementClass;

  String name;
  String oneLetterCode;
  String shortName;
  String description;
};

typedef ReferenceCountedObjectPtr<EnumerationElement> EnumerationElementPtr;

class Enumeration : public Type
{
public:
  Enumeration(const String& name);
  Enumeration(TemplateTypePtr templateType, const std::vector<TypePtr>& templateArguments, TypePtr baseClass)
    : Type(templateType, templateArguments, baseClass) {}

  virtual ClassPtr getClass() const;

  virtual VariableValue create(ExecutionContext& context) const;
  virtual VariableValue createFromString(ExecutionContext& context, const String& value) const;
  virtual VariableValue createFromXml(XmlImporter& importer) const;
  virtual void saveToXml(XmlExporter& exporter, const VariableValue& value) const;

  virtual VariableValue getMissingValue() const;

  virtual String toString(const VariableValue& value) const;

  // elements
  virtual size_t getNumElements() const = 0;
  virtual EnumerationElementPtr getElement(size_t index) const = 0;

  String getElementName(size_t index) const
    {return getElement(index)->getName();}

  int findElementByName(const String& name) const;
  int findElementByOneLetterCode(const juce::tchar c) const;

  bool hasOneLetterCodes() const;

  lbcpp_UseDebuggingNewOperator
};

extern EnumerationPtr addMissingToEnumerationEnumeration(TypePtr type);
extern EnumerationPtr addEntropyToEnumerationEnumeration(TypePtr type);
extern EnumerationPtr missingOrPresentEnumeration;

class DefaultEnumeration : public Enumeration
{
public:
  DefaultEnumeration(const String& name, const juce::tchar** elements, const String& oneLetterCodes = String::empty);
  DefaultEnumeration(const String& name, const String& oneLetterCodes);
  DefaultEnumeration(const String& name);

  virtual size_t getNumElements() const
    {return elements.size();}

  virtual EnumerationElementPtr getElement(size_t index) const
    {jassert(index < elements.size()); return elements[index];}

  void addElement(ExecutionContext& context, const String& elementName, const String& oneLetterCode = String::empty, const String& shortName = String::empty, const String& description = String::empty);
  size_t findOrAddElement(ExecutionContext& context, const String& name);

private:
  std::vector<EnumerationElementPtr> elements;
};

/*
** Class
*/
class Class : public Type
{
public:
  Class(const String& name, TypePtr baseClass)
    : Type(name, baseClass) {}
  Class(TemplateTypePtr templateType, const std::vector<TypePtr>& templateArguments, TypePtr baseClass)
    : Type(templateType, templateArguments, baseClass) {}
  Class() {}

  virtual String toString() const;
  virtual ClassPtr getClass() const;

  virtual bool isMissingValue(const VariableValue& value) const
    {return !value.getObject();}
    
  virtual VariableValue getMissingValue() const
    {return VariableValue();}

  virtual VariableValue createFromString(ExecutionContext& context, const String& value) const;
  virtual VariableValue createFromXml(XmlImporter& importer) const;
  virtual void saveToXml(XmlExporter& exporter, const VariableValue& value) const;

  virtual void destroy(VariableValue& value) const
    {value.clearObject();}

  virtual void copy(VariableValue& dest, const VariableValue& source) const
    {dest.setObject(source.getObjectPointer());}

  virtual String toString(const VariableValue& value) const
    {return value.getObject()->toString();}
  virtual String toShortString(const VariableValue& value) const
    {return value.getObject()->toShortString();}

  virtual int compare(const VariableValue& value1, const VariableValue& value2) const;

  lbcpp_UseDebuggingNewOperator
};

extern ClassPtr objectClass;
extern ClassPtr pairClass(TypePtr firstClass, TypePtr secondClass);

extern ClassPtr typeClass;
extern ClassPtr enumerationClass;
extern ClassPtr classClass;

class DefaultClass : public Class
{
public:
  DefaultClass(const String& name, TypePtr baseClass = objectClass);
  DefaultClass(const String& name, const String& baseClass);
  DefaultClass(TemplateTypePtr templateType, const std::vector<TypePtr>& templateArguments, TypePtr baseClass);
  DefaultClass() {}

  virtual void deinitialize();

  virtual size_t getNumMemberVariables() const;
  virtual VariableSignaturePtr getMemberVariable(size_t index) const;

  virtual int findMemberVariable(const String& name) const;

  size_t findOrAddMemberVariable(ExecutionContext& context, const String& name, TypePtr type);

  void reserveMemberVariables(size_t count)
    {variables.reserve(count);}

  size_t addMemberVariable(ExecutionContext& context, TypePtr type, const String& name, const String& shortName = String::empty, const String& description = String::empty);
  size_t addMemberVariable(ExecutionContext& context, const String& typeName, const String& name, const String& shortName = String::empty, const String& description = String::empty);
  virtual size_t addMemberVariable(ExecutionContext& context, VariableSignaturePtr signature);

  lbcpp_UseDebuggingNewOperator

protected:
  friend class DefaultClassClass;

  std::vector<VariableSignaturePtr> variables;
  std::map<String, size_t> variablesMap;
};

typedef ReferenceCountedObjectPtr<DefaultClass> DefaultClassPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_TYPE_H_
