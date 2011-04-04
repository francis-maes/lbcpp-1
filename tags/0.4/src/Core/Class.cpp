/*-----------------------------------------.---------------------------------.
| Filename: Class.cpp                      | Class Introspection             |
| Author  : Francis Maes                   |                                 |
| Started : 25/08/2010 02:16               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Core/Class.h>
#include <lbcpp/Core/Variable.h>
#include <lbcpp/Core/XmlSerialisation.h>
#include <lbcpp/Core/Vector.h>
#include <map>
using namespace lbcpp;

/*
** Class
*/
String Class::toString() const
{
  String res = getName();
  res += T(" = {");
  size_t n = getNumMemberVariables();
  for (size_t i = 0; i < n; ++i)
  {
    res += getMemberVariableType(i)->getName() + T(" ") + getMemberVariableName(i);
    if (i < n - 1)
      res += T(", ");
  }
  res += T("}");
  return res;
}

int Class::compare(const VariableValue& value1, const VariableValue& value2) const
{
  ObjectPtr object1 = value1.getObject();
  ObjectPtr object2 = value2.getObject();
  if (!object1)
    return object2 ? -1 : 0;
  if (!object2)
    return 1;
  return object1->compare(object2);
}

VariableValue Class::createFromString(ExecutionContext& context, const String& value) const
{
  VariableValue res = create(context);
  if (isMissingValue(res))
  {
    context.errorCallback(T("Class::createFromString"), T("Could not create instance of ") + getName().quoted());
    return getMissingValue();
  }
  return res.getObject()->loadFromString(context, value) ? res : getMissingValue();
}

VariableValue Class::createFromXml(XmlImporter& importer) const
{
  VariableValue res = create(importer.getContext());
  if (isMissingValue(res))
  {
    importer.errorMessage(T("Class::createFromXml"), T("Could not create instance of ") + getName().quoted());
    return getMissingValue();
  }
  return res.getObject()->loadFromXml(importer) ? res : getMissingValue();
}

void Class::saveToXml(XmlExporter& exporter, const VariableValue& value) const
{
  ObjectPtr object = value.getObject();
  jassert(object);
  object->saveToXml(exporter);
}

ClassPtr Class::getClass() const
  {return classClass;}

/*
** DefaultClass
*/
DefaultClass::DefaultClass(const String& name, TypePtr baseClass)
  : Class(name, baseClass)
{
}

DefaultClass::DefaultClass(const String& name, const String& baseClass)
: Class(name, lbcpp::getType(baseClass))
{
}

DefaultClass::DefaultClass(TemplateTypePtr templateType, const std::vector<TypePtr>& templateArguments, TypePtr baseClass)
  : Class(templateType, templateArguments, baseClass) {}

size_t DefaultClass::addMemberVariable(ExecutionContext& context, const String& typeName, const String& name, const String& shortName, const String& description)
{
  TypePtr type;
  if (templateType)
    type = templateType->instantiateTypeName(context, typeName, templateArguments);
  else
    type = typeManager().getType(context, typeName);
  if (type)
    return addMemberVariable(context, type, name, shortName, description);
  else
    return (size_t)-1;
}

size_t DefaultClass::addMemberVariable(ExecutionContext& context, TypePtr type, const String& name, const String& shortName, const String& description)
{
  if (!type || name.isEmpty())
  {
    context.errorCallback(T("Class::addMemberVariable"), T("Invalid type or name"));
    return (size_t)-1;
  }
  if (findMemberVariable(name) >= 0)
  {
    context.errorCallback(T("Class::addMemberVariable"), T("Another variable with name '") + name + T("' already exists"));
    return (size_t)-1;
  }
  return addMemberVariable(context, new VariableSignature(type, name, shortName, description));
}

size_t DefaultClass::addMemberVariable(ExecutionContext& context, VariableSignaturePtr signature)
{
  size_t res = variables.size();
  variablesMap[signature->getName()] = res;
  variables.push_back(signature);
  return res;
}

size_t DefaultClass::getNumMemberVariables() const
{
  size_t n = baseType->getNumMemberVariables();
  return n + variables.size();
}

VariableSignaturePtr DefaultClass::getMemberVariable(size_t index) const
{
  size_t n = baseType->getNumMemberVariables();
  if (index < n)
    return baseType->getMemberVariable(index);
  index -= n;
  
  jassert(index < variables.size());
  return variables[index];
}

void DefaultClass::deinitialize()
{
  variables.clear();
  Class::deinitialize();
}

int DefaultClass::findMemberVariable(const String& name) const
{
  std::map<String, size_t>::const_iterator it = variablesMap.find(name);
  if (it != variablesMap.end())
    return (int)(baseType->getNumMemberVariables() + it->second);
  return baseType->findMemberVariable(name);
}

size_t DefaultClass::findOrAddMemberVariable(ExecutionContext& context, const String& name, TypePtr type)
{
  int idx = findMemberVariable(name);
  if (idx >= 0)
    return (size_t)idx;
  return addMemberVariable(context, type, name);
}