/*-----------------------------------------.---------------------------------.
| Filename: Enumeration.cpp                | Enumeration Introspection       |
| Author  : Francis Maes                   |                                 |
| Started : 25/08/2010 02:17               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include <lbcpp/Core/Enumeration.h>
#include <lbcpp/Core/Variable.h>
#include <lbcpp/Core/Vector.h>
#include <lbcpp/Core/XmlSerialisation.h>
#include <map>
using namespace lbcpp;

Enumeration::Enumeration(const String& name)
  : Type(name, enumValueType)
{
}

ClassPtr Enumeration::getClass() const
  {return enumerationClass;}

VariableValue Enumeration::create(ExecutionContext& context) const
  {return getMissingValue();}

VariableValue Enumeration::getMissingValue() const
  {return VariableValue((juce::int64)getNumElements());}

VariableValue Enumeration::createFromXml(XmlImporter& importer) const
  {return createFromString(importer.getContext(), importer.getAllSubText());}
 
VariableValue Enumeration::createFromString(ExecutionContext& context, const String& value) const
{
  String str = value.trim();
  size_t n = getNumElements();
  for (size_t i = 0; i < n; ++i)
    if (str == getElementName(i))
      return VariableValue(i);
  context.errorCallback(T("Enumeration::createFromString"), T("Could not find enumeration value ") + value.quoted());
  return getMissingValue();
}

void Enumeration::saveToXml(XmlExporter& exporter, const VariableValue& value) const
{
  exporter.addTextElement(toString(value));
}

String Enumeration::toString(const VariableValue& value) const
{
  juce::int64 val = value.getInteger();
  return val >= 0 && (size_t)val < getNumElements() ? getElementName((size_t)val) : T("Missing");
}

bool Enumeration::hasOneLetterCodes() const
{
  size_t n = getNumElements(); 
  for (size_t i = 0; i < n; ++i)
    if (getElement(i)->getOneLetterCode().length() != 1)
      return false;
  return true;
}

int Enumeration::findElementByName(const String& name) const
{
  size_t n = getNumElements();
  for (size_t i = 0; i < n; ++i)
    if (getElementName(i) == name)
      return (int)i;
  return -1;
}

int Enumeration::findElementByOneLetterCode(const juce::tchar c) const
{
  size_t n = getNumElements();
  for (size_t i = 0; i < n; ++i)
  {
    String code = getElement(i)->getOneLetterCode();
    if (code.length() == 1 && code[0] == c)
      return (int)i;
  }
  return -1;
}

/*
juce::tchar Enumeration::getOneLetterCode(size_t index) const
{
  jassert(index < elements.size());
  if (oneLetterCodes.length())
  {
    jassert(oneLetterCodes.length() == (int)elements.size());
    return oneLetterCodes[(int)index];
  }
  else
  {
    jassert(elements[index].isNotEmpty());
    return elements[index][0];
  }
}

String Enumeration::getOneLetterCodes() const
{
  if (oneLetterCodes.isEmpty())
  {
    String res;
    for (size_t i = 0; i < elements.size(); ++i)
      res += getOneLetterCode(i);
    return res;
  }
  else
  {
    jassert(oneLetterCodes.length() == (int)elements.size());
    return oneLetterCodes;
  }
}*/

/*
** DefaultEnumeration
*/
DefaultEnumeration::DefaultEnumeration(const String& name)
  : Enumeration(name)
{
}

DefaultEnumeration::DefaultEnumeration(const String& name, const juce::tchar** elements, const String& oneLetterCodes)
  : Enumeration(name)
{
  jassert(!oneLetterCodes.containsChar('_')); // '_' is reserved to denote missing values
  for (size_t index = 0; elements[index]; ++index)
    addElement(defaultExecutionContext(), elements[index], (int)index < oneLetterCodes.length() ? oneLetterCodes.substring(index, index + 1) : String::empty);
}

DefaultEnumeration::DefaultEnumeration(const String& name, const String& oneLetterCodes)
  : Enumeration(name)
{
  jassert(!oneLetterCodes.containsChar('_'));
  for (int i = 0; i < oneLetterCodes.length(); ++i)
  {
    String oneLetterCode = oneLetterCodes.substring(i, i + 1);
    addElement(defaultExecutionContext(), oneLetterCode, oneLetterCode);
  }
}

void DefaultEnumeration::addElement(ExecutionContext& context, const String& name, const String& oneLetterCode, const String& shortName, const String& description)
{
  if (findElementByName(name) >= 0)
  {
    context.errorCallback(T("Enumeration::addElement"), T("Element '") + name + T("' already exists"));
    return;
  }
  elements.push_back(new EnumerationElement(name, oneLetterCode, shortName, description));
}

size_t DefaultEnumeration::findOrAddElement(ExecutionContext& context, const String& name)
{
  int elt = findElementByName(name);
  if (elt >= 0)
    return (size_t)elt;
  size_t res = elements.size();
  elements.push_back(new EnumerationElement(name));
  return res;
}
